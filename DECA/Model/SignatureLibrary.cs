using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;

namespace DECA.Model
{
    public class SignatureLibrary
    {
        [XmlRoot]
        public class Signatures
        {
            [XmlElement]
            public Signature[] Signature { get; set; }
        }

        public class Signature
        {
            [XmlElement]
            public string Name { get; set; }

            [XmlElement]
            public string HeaderSignature { get; set; }

            [XmlElement]
            public string FooterSignature { get; set; }

            [XmlElement]
            public string Offset { get; set; }

            [XmlElement]
            public string FileExtension { get; set; }

            [XmlElement]
            public string Category { get; set; }
        }
    }
}
