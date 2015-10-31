using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace DECA
{
    public class DriveScanner
    {
        #region DECA Disk Scanner.dll function prototypes
        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr createScanner();

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void buildScanner(IntPtr driveScanner, UInt32 chunkSize, UInt32 sectorSize, string diskPath, UInt32 offset, UInt32 maxSignatureSize);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int mountVolume(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void addSignature(IntPtr driveScanner, Model.ScanRequest.SignatureData signature);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void lockSignatureList(IntPtr diskScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr scanChunkBySector(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void unmountVolume();

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void disposeScanner(IntPtr driveScanner);
        #endregion

        #region Global variables
        public Model.SignatureLibrary.Signatures SignatureLibrary;
        private IntPtr DriveScannerPointer = IntPtr.Zero;
        #endregion

        #region Private functions
        /// <summary>
        /// Converts a hexademical string into a text string.
        /// </summary>
        /// <param name="hexValue">hexadecimal string</param>
        /// <returns>text string</returns>
        private static string ConvertHexToString(string hexValue)
        {
            string StrValue = "";
            while (hexValue.Length > 0)
            {
                StrValue += System.Convert.ToChar(System.Convert.ToUInt32(hexValue.Substring(0, 2), 16)).ToString();
                hexValue = hexValue.Substring(2, hexValue.Length - 2);
            }
            return StrValue;
        }

        /// <summary>
        /// Gets the length of the greatest signature in the database
        /// </summary>
        /// <returns>An integer represing the length of the greatest signature in the database</returns>
        private int GetMaximumSignatureLength()
        {
            //Get the maximum signature size
            int currentMaxSignatureLength = 0;
            for (int i = 0; i <= SignatureLibrary.Signature.Length - 1; i++)
            {
                if (SignatureLibrary.Signature[i].HeaderSignature.Length > currentMaxSignatureLength)
                {
                    currentMaxSignatureLength = SignatureLibrary.Signature[i].HeaderSignature.Length;
                }
            }

            return currentMaxSignatureLength;
        }

        /// <summary>
        /// Creates the Drive Scanner in the dll
        /// </summary>
        private void CreateScanner()
        {
            //Check if the scanne has been created and if not, create it
            if (DriveScannerPointer == IntPtr.Zero)
                DriveScannerPointer = createScanner();
        }
        #endregion

        #region Public functions
        /// <summary>
        /// Loads the signature library from the specified location into our objects
        /// </summary>
        /// <param name="signatureLibraryPath">The path to the signature database file</param>
        /// <returns>A response code stating whether the function completed successfully or failed</returns>
        public int LoadSignatureLibrary(string signatureLibraryPath)
        {
            //Check if the signature file exists
            if (!File.Exists(signatureLibraryPath))
            {
                //File doesn't exist
                return 1;
            }
            else
            {
                //File does exist, try to load the contents of the file
                try
                {
                    //Read in the signature library and convert into our class
                    StreamReader libraryReader = new StreamReader(signatureLibraryPath);
                    XmlSerializer serializer = new XmlSerializer(typeof(Model.SignatureLibrary.Signatures));
                    SignatureLibrary = (Model.SignatureLibrary.Signatures)serializer.Deserialize(libraryReader);
                    libraryReader.Close();
                }
                catch (Exception e)
                {
                    //Conversion failed
                    Console.WriteLine(e.ToString());
                    return 2;
                }

                try
                {
                    //Create the scanner object
                    CreateScanner();

                    //Add the list of signature into the signature library located within the dll
                    for (int i = 0; i <= SignatureLibrary.Signature.Length - 1; i++)
                    {
                        Model.ScanRequest.SignatureData signature = new Model.ScanRequest.SignatureData();
                        signature.sigId = (UInt32)i;
                        signature.sigHeader = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].HeaderSignature);
                        addSignature(DriveScannerPointer, signature);
                    }
                }
                catch (Exception e)
                {
                    //Error while processing the signature list
                    Console.WriteLine(e.ToString());
                    return 3;
                }

                return 0;
            }
        }

        /// <summary>
        /// Initializes the drive scanner on the dll
        /// </summary>
        /// <param name="chunkSize">The amount of sectors you want to scan per chunk</param>
        /// <param name="sectorSize">The amount of bytes in each sector to scan</param>
        /// <param name="drivePath">The windows formatted path of the drive to scan</param>
        /// <param name="offset">The offset from the start of the disk from which you want to begin scanning</param>
        /// <returns>A response code stating whether the initialization completed successfully or failed</returns>
        public int Initialize(int chunkSize, int sectorSize, string drivePath, int offset = 0)
        {
            try
            {
                //Create the scanner object
                CreateScanner();

                //Initialize the Drive Scanner
                buildScanner(DriveScannerPointer, (UInt32)chunkSize, (UInt32)sectorSize, drivePath, (UInt32)offset, (UInt32)GetMaximumSignatureLength());

                //Lock the signature list
                lockSignatureList(DriveScannerPointer);
            }
            catch (Exception e)
            {
                //Error while initializing Drive Scanner
                Console.Write(e.ToString());
                return 1;
            }

            //Attempt to mount the volume
            if (mountVolume(DriveScannerPointer) != 0)
            {
                Console.WriteLine("Unable to mount volume.");
                return 2;
            }

            return 0;
        }

        /// <summary>
        /// Scans a single sector and returns the result
        /// </summary>
        /// <returns>A 1d int array containing the results of the scan or -1 if the scan fails</returns>
        public int[] ScanSector()
        {
            try
            {
                //Scan the chunk and store the result an int pointer
                IntPtr result = scanChunkBySector(DriveScannerPointer);

                //Copy the contents of the pointer into an int array
                int[] resultArray = new int[SignatureLibrary.Signature.Length - 1];
                Marshal.Copy(result, resultArray, 0, SignatureLibrary.Signature.Length);

                return resultArray;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                return new int[] { -1 };
            }
        }

        /// <summary>
        /// Disposes of the previously initialized Drive Scanner
        /// </summary>
        /// <returns>A response code stating whether the Drive Scanner was successfully disposed or not</returns>
        public int Dispose()
        {
            try
            {
                //Unmount the volume
                unmountVolume();

                //Dispose of the Drive Scanner Instance
                disposeScanner(DriveScannerPointer);
            }
            catch (Exception e)
            {
                //Failure while disposing
                Console.WriteLine(e.ToString());
                return 1;
            }

            return 0;
        }


        #endregion
    }
}