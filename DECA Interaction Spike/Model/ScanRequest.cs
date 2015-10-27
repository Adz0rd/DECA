using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DECA_Interaction_Spike.Model
{
    class ScanRequest
    {
        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct SignatureData
        {
            public UInt32 sigId;
            [MarshalAs(UnmanagedType.LPStr)]
            public string sigHeader;
            [MarshalAs(UnmanagedType.LPStr)]
            public string sigFooter;
            public IntPtr next;
        }

        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct Request
        {
            public UInt32 maxSignatureSize;
            public UInt32 numSigPairs;
            public IntPtr sigArr;
        }
    }
}
