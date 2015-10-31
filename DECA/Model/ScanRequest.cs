using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DECA.Model
{
    public class ScanRequest
    {
        [StructLayout(LayoutKind.Sequential), Serializable]
        public struct SignatureData
        {
            public UInt32 sigId;
            [MarshalAs(UnmanagedType.LPStr)]
            public string sigHeader;
        }
    }
}
