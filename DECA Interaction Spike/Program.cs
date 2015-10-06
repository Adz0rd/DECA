using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;


namespace DECA_Interaction_Spike
{
    class Program
    {
        #region DiskScanner.dll function prototypes
        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr createScanner();

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void buildScanner(IntPtr driveScanner, UInt32 scanChunkSize, UInt32 sectorSize, string pathToDisk, UInt32 offset);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void disposeScanner(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int scanChunk(IntPtr driveScanner, Model.Signature.Library signatureLibrary, IntPtr scanResponse);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int mountVolume(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void readSigData(IntPtr driveScanner, Model.Signature.Library signatureLibrary);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int scanChunkTest(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void unmountVolume();
        #endregion

        static void Main(string[] args)
        {
            //Initialize the drive scanner
            IntPtr DriveScanner = createScanner();

            //Configure the scanner
            //buildScanner(DriveScanner, Convert.ToUInt32(args[0]), 1024, args[1], 0);
            buildScanner(DriveScanner, 10000, 1024, @"\\.\D:", 0);
            int ScanResult = -1;
            

            //Construct the signature library
            Model.Signature.Library SignatureLibrary = new Model.Signature.Library()
            {
                numSigPairs = 1,
                maxSignatureSize = 3,
                sigArr = new Model.Signature.SignatureData[1]
                {
                    new Model.Signature.SignatureData()
                    {
                        sigId = 1,
                        sigHeader = "FFD8FF",
                        sigFooter = null
                    }
                }
            };

            IntPtr ResponsePointer = Marshal.AllocHGlobal(Marshal.SizeOf(new Model.Scan.Response()));
            
            //Mount volume
            if (mountVolume(DriveScanner) == 0)
            {
                //Scan the device
                ScanResult = scanChunk(DriveScanner, SignatureLibrary, ResponsePointer);
            }

            //readSigData(DriveScanner, SignatureLibrary);

            //Unmount the volume
            //unmountVolume();

            //Dispose of the scanner
            //disposeScanner(DriveScanner);

            //Print results
            Console.WriteLine("Scan result: " + ScanResult);
            Console.ReadKey();
        }
    }
}
