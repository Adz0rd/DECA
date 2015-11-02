using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace DECA_Initial_Library_Parser
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                List <string[]> libraryLines = new List<string[]>();
                int lineCounter = 0;

                // Open the text file using a stream reader.
                using (StreamReader sr = new StreamReader(args[0]))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        //Split the string by comma and add them to a list for processing
                        libraryLines.Add(line.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries));
                        lineCounter++;
                    }
                }

                //Write out the signatures to an xml file which will be treated as the DECA signature database
                using (XmlWriter writer = XmlWriter.Create(Environment.CurrentDirectory + "\\signatures.xml"))
                {
                    writer.WriteStartDocument();
                    writer.WriteStartElement("Signatures");

                    foreach (var libraryLine in libraryLines)
                    {
                        writer.WriteStartElement("Signature");
                        writer.WriteElementString("Name", libraryLine[0]);
                        writer.WriteElementString("HeaderSignature", libraryLine[1].Replace(" ", string.Empty));
                        writer.WriteElementString("FooterSignature", null);
                        writer.WriteElementString("Offset", "0");
                        writer.WriteElementString("FileExtension", libraryLine[4]);
                        writer.WriteElementString("Category", libraryLine[5]);
                        writer.WriteEndElement();
                    }

                    writer.WriteEndElement();
                    writer.WriteEndDocument();
                }

                Console.WriteLine("Finished converting " + lineCounter + " records.");
            }
            catch (Exception e)
            {
                Console.WriteLine("Error while reading the signature library: " + e.Message.ToString());
            }

            Console.ReadKey();
        }
    }
}
