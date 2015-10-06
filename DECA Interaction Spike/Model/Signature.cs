using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DECA_Interaction_Spike.Model
{
    class Signature
    {
        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct SignatureData
        {
            public UInt32 sigId;
            [MarshalAs(UnmanagedType.LPStr)]
            public string sigHeader;
            [MarshalAs(UnmanagedType.LPStr)]
            public string sigFooter;
        }

        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct Library
        {
            public UInt32 maxSignatureSize;
            public UInt32 numSigPairs;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.Struct)]
            public SignatureData[] sigArr;
        }
    }
}
