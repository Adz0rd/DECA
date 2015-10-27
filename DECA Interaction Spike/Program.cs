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
        public static extern void buildScanner(IntPtr driveScanner, UInt32 scanChunkSize, UInt32 sectorSize, string pathToDisk, UInt32 offset);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void disposeScanner(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int scanChunk(IntPtr driveScanner, Model.ScanRequest.Request signatureLibrary, IntPtr scanResponse);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int mountVolume(IntPtr driveScanner);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void readSigData(IntPtr driveScanner, Model.ScanRequest.Request signatureLibrary);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr scanChunkList(IntPtr driveScanner, Model.ScanRequest.Request signatureLibrary);

        [DllImport("DECA Disk Scanner.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void unmountVolume();
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

        public static Model.ScanRequest.Request ConvertSignatureLibraryToScanRequest(Model.SignatureLibrary.Signatures signatureLibrary)
        {
            UInt32 i = 460;

            //Initialize the first signature manually
            var head = new Model.ScanRequest.SignatureData();
            head.sigId = i;
            head.sigHeader = ConvertHexToString(signatureLibrary.Signature.ToArray()[i].HeaderSignature);
            head.sigFooter = ConvertHexToString(signatureLibrary.Signature.ToArray()[i].FooterSignature);

            Model.ScanRequest.SignatureData current = head;

            //for (i = 1; i < signatureLibrary.Signature.Length; i++)
            //{
            //    var subSigArrayElement = new Model.ScanRequest.SignatureData();
            //    subSigArrayElement.sigId = i;
            //    subSigArrayElement.sigHeader = signatureLibrary.Signature.ToArray()[i].HeaderSignature;
            //    subSigArrayElement.sigFooter = signatureLibrary.Signature.ToArray()[i].FooterSignature;

            //    int subSigSize = Marshal.SizeOf(subSigArrayElement);
            //    current.next = Marshal.AllocCoTaskMem(subSigSize);
            //    Marshal.StructureToPtr(subSigArrayElement, current.next, false);

            //    current = subSigArrayElement;
            //}

            int sigSize = Marshal.SizeOf(head);
            Model.ScanRequest.Request request = new Model.ScanRequest.Request();
            request.numSigPairs = 1;//(UInt32)signatureLibrary.Signature.Length;
            request.maxSignatureSize = 3;
            request.sigArr = Marshal.AllocCoTaskMem(sigSize);
            Marshal.StructureToPtr(head, request.sigArr, false);

            return request;
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

                //Initialize the drive scanner
                IntPtr DriveScanner = createScanner();

                //Configure the scanner
                //buildScanner(DriveScanner, Convert.ToUInt32(args[0]), 1024, args[1], 0);
                buildScanner(DriveScanner, 16410, 512, @"\\.\F:", 0);

                //Mount volume
                if (mountVolume(DriveScanner) == 0)
                {
                    //Model.ScanRequest.SignatureData previous = new Model.ScanRequest.SignatureData();
                    IntPtr childPtr = IntPtr.Zero;

                    for(int i = SignatureLibrary.Signature.Length - 1; i >= 0; i--)
                    {
                        if (childPtr == IntPtr.Zero)
                        {
                            //No struct has been built yet, create new struct
                            Model.ScanRequest.SignatureData child = new Model.ScanRequest.SignatureData();
                            child.sigId = (UInt32)i;
                            child.sigHeader = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].HeaderSignature);
                            child.sigFooter = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].FooterSignature);

                            //Allocate memory for the current and move the pointer to it
                            int sigSize = Marshal.SizeOf(child);
                            childPtr = Marshal.AllocCoTaskMem(sigSize);
                            Marshal.StructureToPtr(child, childPtr, false);

                        } else {
                            //A child exists, build its parent
                            Model.ScanRequest.SignatureData parent = new Model.ScanRequest.SignatureData();
                            parent.sigId = (UInt32)i;
                            parent.sigHeader = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].HeaderSignature);
                            parent.sigFooter = ConvertHexToString(SignatureLibrary.Signature.ToArray()[i].FooterSignature);
                            
                            //Link the child to its parent
                            parent.next = Marshal.AllocCoTaskMem(Marshal.SizeOf(childPtr));
                            parent.next = childPtr;

                            //Make this parent into a child for the next parent
                            childPtr = Marshal.ReAllocCoTaskMem(childPtr, Marshal.SizeOf(parent));
                            Marshal.StructureToPtr(parent, childPtr, true);
                        }
                    }

                    //Construct the signature library
                    Model.ScanRequest.Request request = new Model.ScanRequest.Request()
                    {
                        numSigPairs = (UInt32)SignatureLibrary.Signature.Length,
                        maxSignatureSize = 3,
                        sigArr = childPtr
                    };

                    //int sigSize2 = Marshal.SizeOf(sigArr2);
                    //sigArr.next = Marshal.AllocCoTaskMem(sigSize2);

                    //try
                    //{
                    //    Marshal.StructureToPtr(sigArr2, sigArr.next, false);
                    //    Marshal.StructureToPtr(sigArr, SignatureLibrary.sigArr, false);

                    IntPtr response = scanChunkList(DriveScanner, request);

                    //Model.Scan.ScanResult scanResults = new Model.Scan.ScanResult();
                    //scanResults = (Model.Scan.ScanResult)Marshal.PtrToStructure(response, typeof(Model.Scan.ScanResult));

                    //Console.WriteLine("The value of the first sigId:" + scanResults.sigId);

                    //Model.Scan.ScanResult scanResults2 = new Model.Scan.ScanResult();
                    //scanResults2 = (Model.Scan.ScanResult)Marshal.PtrToStructure(scanResults.next, typeof(Model.Scan.ScanResult));
                    //Console.WriteLine("The value of the first sigId:" + scanResults2.sigId);
                    //}
                    //finally
                    //{
                    //    Marshal.FreeCoTaskMem(sigArr.next);
                    //    Marshal.FreeCoTaskMem(SignatureLibrary.sigArr);
                    //}
                }


            } else {
                System.Environment.Exit(1);
            }



            

            

            






            //IntPtr ResponsePointer = Marshal.AllocHGlobal(Marshal.SizeOf(new Model.Scan.Response()));





            //Unmount the volume
            //unmountVolume();

            //Dispose of the scanner
            //disposeScanner(DriveScanner);

            //Print results
            Console.ReadKey();
        }
    }
}
