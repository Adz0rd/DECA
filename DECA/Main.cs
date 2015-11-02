using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DECA
{
    public partial class Main : Form
    {
        

        #region Global variables
        DriveInfo[] allDrives = DriveInfo.GetDrives();
        DriveScanner DriveScanner;
        #endregion

        public Main()
        {
            InitializeComponent();
        }

        private void Main_Load(object sender, EventArgs e)
        {
            //Initialize the GUI
            driveSelectionBox.Items.AddRange(allDrives);
            quickSearchCheckBox.Checked = true;

            //Load the signature library
            DriveScanner = new DriveScanner();
            int loadResult = DriveScanner.LoadSignatureLibrary(Application.StartupPath + "\\signatures.xml");

            if(loadResult != 0)
            {
                switch (loadResult)
                {
                    case 1:
                        MessageBox.Show("Signature library file not found. Application will now quit.");
                        break;
                    case 2:
                        MessageBox.Show("Invalid signature library file detected. Application will now quit.");
                        break;
                    case 3:
                        MessageBox.Show("Error while processing the signature library. Application will now quit.");
                        break;
                }
                Application.Exit();
            }
        }

        private void analysisBeginButton_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(driveSelectionBox.Text))
            {
                MessageBox.Show("You have not selected a drive to analyse.");
            } else
            {
                //Disable user controls while scanning
                analysisBeginButton.Enabled = false;
                driveSelectionBox.Enabled = false;
                quickSearchCheckBox.Enabled = false;

                //Initialization
                statusLabel.Text = "initializing...";
                analysisProgressBar.Value = 0;
                string drivePath = @"\\.\" + driveSelectionBox.Text.Substring(0, driveSelectionBox.Text.Length - 1);
                long totalAmountOfSectors = allDrives[driveSelectionBox.SelectedIndex].TotalSize / 512;
                int sectorsPerCluster = (int)totalAmountOfSectors / 10000;
                int initializationResult = DriveScanner.Initialize(sectorsPerCluster, 512, drivePath);

                //Error check
                switch (initializationResult)
                {
                    case 1:
                        MessageBox.Show("Error while initializing the volume. Please restart the program and try again.");
                        statusLabel.Text = "failed to initialize. Please restart the application.";
                        break;
                    case 2:
                        MessageBox.Show("Error while mounting the volume. Please make sure the volume isn't being utilized by another program and try again.");
                        statusLabel.Text = "failed to initialize. Please restart the application.";
                        break;
                }

                //Scanning
                statusLabel.Text = "analysing...";
                if (scanSectorBackgroundWorker.IsBusy != true)
                {
                    // Start the asynchronous operation.
                    scanSectorBackgroundWorker.RunWorkerAsync(quickSearchCheckBox.Checked);
                }
            }

        }

        private void scanSectorBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            // Get the BackgroundWorker that raised this event.
            BackgroundWorker worker = sender as BackgroundWorker;

            //Store the result
            e.Result = ScanSector((bool)e.Argument, worker, e);
        }

        private void scanSectorBackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            string ResultMessage = "";

            if (e.Error != null)
            {
                Console.WriteLine(e.Error.Message.ToString());
            }
            else
            {
                int[] result = (int[])e.Result;

                //Match up the result to signatures
                Dictionary<string, int> resultList = new Dictionary<string, int>();
                for (int i = 0; i < result.Length - 1; i++)
                {
                    if (result[i] > 0)
                    {
                        if (resultList.ContainsKey(DriveScanner.SortedSignatureLibrary.Signature[i].Category))
                        {
                            //The result list contains the category of signature already, just update the amount we found
                            resultList[DriveScanner.SortedSignatureLibrary.Signature[i].Category] += result[i];
                        } else
                        {
                            //The result list doesnt contain the category of signature, add it to the list with teh current value
                            resultList.Add(DriveScanner.SortedSignatureLibrary.Signature[i].Category, result[i]);
                        }
                    }
                }

                //Create a message to return to the user
                foreach (KeyValuePair<string, int> pair in resultList)
                {
                    ResultMessage += pair.Value + " " + pair.Key + " files found.\n";
                }

                MessageBox.Show(ResultMessage == "" ? "No files found." : ResultMessage);
            }

            //Re-enable user controls - Disabled due to errors (out-of-scope)
            //analysisBeginButton.Enabled = true;
            //driveSelectionBox.Enabled = true;

            //Reset graphical controls
            statusLabel.Text = "analysis complete.";

            //Dispose of finished drive scanner
            DriveScanner.Dispose();
        }

        private void scanSectorBackgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            this.analysisProgressBar.Value = e.ProgressPercentage;
        }

        private int[] ScanSector(bool IsSearchQuick, BackgroundWorker worker, DoWorkEventArgs e)
        {
            int[][] resultArrays = new int[10000][];
            int[] results = new int[DriveScanner.SortedSignatureLibrary.Signature.Length];

            int currentProgressBarValue = 0;

            for (int i = 0; i < 10000; i++)
            {
                resultArrays[i] = DriveScanner.ScanSector(IsSearchQuick);

                //Sum all of the results into a single array
                if (i > 0)
                {
                    results = resultArrays[0].Zip(resultArrays[i], (x, y) => x + y).ToArray();
                } else
                {
                    results = resultArrays[0];
                }
                
                worker.ReportProgress(currentProgressBarValue++);
            }

            return results;
        }
    }
}
