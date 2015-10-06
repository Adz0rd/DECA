using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DECA_Interaction_Spike.Model
{
    class Scan
    {
        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct ScanResult
        {
            public UInt32 sigId;
            public UInt32 headerCount;
            public UInt32 footerCount;
        }

        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct Response
        {
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.Struct)]
            public ScanResult[] scanResultsArr;
        }
    }
}
