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
            driveSelectionBox.Items.AddRange(allDrives);

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

                //Initialization
                statusLabel.Text = "initializing...";
                string drivePath = @"\\.\" + driveSelectionBox.Text.Substring(0, driveSelectionBox.Text.Length - 1);
                long totalAmountOfSectors = allDrives[driveSelectionBox.SelectedIndex].TotalSize / 512;
                //int sectorsPerCluster = (int)totalAmountOfSectors / 10000;
                int sectorsPerCluster = 1;
                DriveScanner.Initialize(sectorsPerCluster, 512, drivePath);

                //Scanning
                statusLabel.Text = "scanning...";
                if (scanSectorBackgroundWorker.IsBusy != true)
                {
                    // Start the asynchronous operation.
                    scanSectorBackgroundWorker.RunWorkerAsync(analysisProgressBar.Value);
                }
            }

        }

        private void scanSectorBackgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            // Get the BackgroundWorker that raised this event.
            BackgroundWorker worker = sender as BackgroundWorker;

            //Store the result
            e.Result = ScanSector((int)e.Argument, worker, e);
        }

        private void scanSectorBackgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                Console.WriteLine(e.Error.Message.ToString());

            }
            else
            {
                // Finally, handle the case where the operation 
                // succeeded.
                Console.WriteLine(e.Result.ToString());
            }
            //Re-enable user controls
            analysisBeginButton.Enabled = true;
            driveSelectionBox.Enabled = true;

            //Dispose of finished drive scanner
            DriveScanner.Dispose();
        }

        private void scanSectorBackgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            this.analysisProgressBar.Value = e.ProgressPercentage;
        }

        private int[] ScanSector(int currentProgressBarValue, BackgroundWorker worker, DoWorkEventArgs e)
        {
            //Get the scan results for all sectors
            int[][] resultArrays = new int[10000][];
            for (int i = 0; i < 10000; i++)
            {
                resultArrays[i] = DriveScanner.ScanSector();
                worker.ReportProgress(currentProgressBarValue++);
            }

            //Sum all of the results into a single array
            foreach (int[] resultArray in resultArrays)
            {
                resultArrays[0].Zip(resultArray, (x, y) => x + y);
            }

            return resultArrays[0];
        }
    }
}
