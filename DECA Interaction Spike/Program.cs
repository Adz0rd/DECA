using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Xml;
using System.IO;
using System.Xml.Serialization;

namespace DECA_Interaction_Spike
{
    class Program
    {
        #region DECA Disk Scanner.dll function prototypes
        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr createScanner();

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void buildScanner(IntPtr driveScanner, UInt32 scanChunkSize, UInt32 sectorSize, string pathToDisk, UInt32 offset, UInt32 maxSignatureSize);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int mountVolume(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void addSignature(IntPtr driveScanner, Model.ScanRequest.SignatureData signature);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void lockSignatureList(IntPtr diskScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr scanChunk(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr scanChunk_BST(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr scanChunkBySector(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr scanChunkBySector_BST(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int unmountVolume(IntPtr driveScanner);
        #endregion

        public static string ConvertHexToString(string HexValue)
        {
            string StrValue = "";
            while (HexValue.Length > 0)
            {
                StrValue += System.Convert.ToChar(System.Convert.ToUInt32(HexValue.Substring(0, 2), 16)).ToString();
                HexValue = HexValue.Substring(2, HexValue.Length - 2);
            }
            return StrValue;
        }

        static void Main(string[] args)
        {
            //Load signature library
            if (File.Exists(Environment.CurrentDirectory + "\\signatures.xml"))
            {
                StreamReader libraryReader = new StreamReader(Environment.CurrentDirectory + "\\signatures.xml");

                XmlSerializer serializer = new XmlSerializer(typeof(Model.SignatureLibrary.Signatures));

                Model.SignatureLibrary.Signatures SignatureLibrary = (Model.SignatureLibrary.Signatures)serializer.Deserialize(libraryReader);
                libraryReader.Close();

                int currentMaxSignatureSize = 0;
                for (int i = 0; i <= SignatureLibrary.Signature.Length - 1; i++)
                {
                    if (SignatureLibrary.Signature[i].HeaderSignature.Length > currentMaxSignatureSize)
                    {
                        currentMaxSignatureSize = SignatureLibrary.Signature[i].HeaderSignature.Length;
                    }
                }

                //Initialize the drive scanner
                IntPtr DriveScanner = createScanner();

                //Configure the scanner
                //buildScanner(DriveScanner, Convert.ToUInt32(args[0]), 1024, args[1], 0);
                buildScanner(DriveScanner, 1, 512, @"\\.\E:", 129280, (UInt32)currentMaxSignatureSize);

                //Mount volume
                if (mountVolume(DriveScanner) == 0)
                {
                    for (int i = 0; i <= SignatureLibrary.Signature.Length - 1; i++)
                    {
                        Model.ScanRequest.SignatureData signature = new Model.ScanRequest.SignatureData();
                        signature.sigId = (UInt32)i;
                        signature.sigHeader = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].HeaderSignature);
                        addSignature(DriveScanner, signature);
                    }

                    lockSignatureList(DriveScanner);

                    IntPtr result = scanChunkBySector(DriveScanner);
             

                    unmountVolume(DriveScanner);
                }


            } else {
                System.Environment.Exit(1);
            }
            //Print results
            Console.ReadKey();
        }
    }
}
