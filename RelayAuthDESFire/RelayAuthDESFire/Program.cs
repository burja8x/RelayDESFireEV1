using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace RelayAuthDESFire
{
    class Program
    {
        public static SerialPort port;
        public static System.Diagnostics.Process cmd;

        public static string TAG_UID = "";
        public const string PROXMARK_CLIENT_LOCATION = "D:\\ProxSpace-64\\pm3\\proxmark3\\client\\proxmark3.exe COM16"; // D // 16 // C: 4
        public const string SERIAL_PORT = "COM10"; // 10 // 5
        public static bool proxmarkClientOpen = false;

        public static bool lastPreCmd2 = true; // x is 3
        static void Main(string[] args)
        {
            Console.WriteLine("HI");
            try
            {
                SerialInit();
                ProxmarkInit();

                //TestProxClient();
            }
            catch (Exception e)
            {
                Console.WriteLine("______________ERROR_______________");
                if (port != null)
                {
                    CloseSerial();
                }
                if (cmd != null)
                {
                    cmd.Close();
                }
                Console.WriteLine(e.StackTrace);
            }
            Console.ReadLine();
            Console.WriteLine("____________CLOSE______________");
        }

        private static void Cmd_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            Console.WriteLine(e.Data);

            var m = new Regex(@"FOR\sTAG\:(\w+)", RegexOptions.Compiled);
            var oo = m.Match(e.Data);
            if (oo.Success)
            {
                proxmarkClientOpen = false;

                string data = oo.Groups[1].Value;
                Console.WriteLine("______________DATA_____________");

                lastPreCmd2 = data.ElementAt(1) == '2' ? true : false;


                string readerCmd = data.Substring(2, 2).ToUpper(); // second byte.
                string cmdForMole = "";
                if (readerCmd == "AA" || readerCmd == "AF" || readerCmd == "BD")
                {
                    cmdForMole = data.Substring(2, data.Length - 2 - 4);
                    Console.WriteLine($"Sending to Mole: {cmdForMole}");
                    port.WriteLine($"+{cmdForMole}*");
                }
                else
                {
                    Console.WriteLine($"ERROR strange command: {readerCmd}");
                }
            }

            if (e.Data.Contains("-- DONE --"))
            {
                //proxmarkClientOpen = false;
            }
            if (e.Data.Contains("DONE PM3")) {
                proxmarkClientOpen = false;
            }
        }

        private static void Serial_Data(object sender, SerialDataReceivedEventArgs e)
        {
            string line = port.ReadLine();
            if (line.Length != 1)
            {
                Console.WriteLine($"Serial: {line}");
            }

            if (line.StartsWith("UID:"))
            {
                TAG_UID = line.Substring(4, 14);
                Console.WriteLine($"Mole: Tag UID:{TAG_UID}");

            }
            else if (line.Contains("APPLICATION SELECTED"))
            {
                // Send ID to proxmark3.

                SendProxmarkCMD(TAG_UID, false, true);

                //TestPN532();
            }
            else if (line.StartsWith(">"))
            {
                var m = new Regex(@"\>(\w+)\>(\w+)\<", RegexOptions.Compiled);
                var oo = m.Match(line);
                if (oo.Success)
                {
                    string commandX = oo.Groups[1].Value;
                    string response = oo.Groups[2].Value;
                    if (commandX == "AA" && response.Length == 34)
                    {
                        SendProxmarkCMD(response, true, lastPreCmd2);
                    }
                    else if (commandX == "AF" && response.Length == 34)
                    {
                        SendProxmarkCMD(response, true, lastPreCmd2);
                    }
                    else if (commandX == "BD" && response.Length == 66)
                    {
                        SendProxmarkCMD(response, true, lastPreCmd2);
                    }
                    else
                    {
                        Console.WriteLine($"UNKNOWN {commandX}  {response}  {response.Length / 2} bytes");
                    }
                }
                else
                {
                    Console.WriteLine("ERROR regex from tag. !!!");
                }
            }
            else if (line.StartsWith("ERROR WHEN SELECTING APP."))
            {
            }
            else if (line.StartsWith("EXIT"))
            {
            }
            else
            {
            }
        }

        private static void ProxmarkInit()
        {
            cmd = new System.Diagnostics.Process();
            cmd.StartInfo.FileName = "cmd.exe";
            cmd.StartInfo.RedirectStandardInput = true;
            cmd.StartInfo.RedirectStandardOutput = true;
            cmd.StartInfo.RedirectStandardError = true;
            cmd.StartInfo.CreateNoWindow = true;
            cmd.StartInfo.UseShellExecute = false;
            cmd.OutputDataReceived += Cmd_OutputDataReceived;
            cmd.Start();
            cmd.BeginOutputReadLine();

            Thread.Sleep(500);

            cmd.StandardInput.WriteLine(PROXMARK_CLIENT_LOCATION);
            cmd.StandardInput.Flush();

            Thread.Sleep(700);
        }

        private static void SerialInit()
        {
            if (port == null)
            {
                port = new SerialPort(SERIAL_PORT, 115200);
                port.Open();
                port.DataReceived += Serial_Data;
            }
        }

        private static void CloseSerial()
        {
            if (port != null && port.IsOpen)
            {
                port.Close();
            }
        }

        private static void SendProxmarkCMD(string data, bool oneCmd, bool lastCmd2)
        {
            if (!proxmarkClientOpen)
            {
                string ukaz = $"hf 14a simx{(lastCmd2 ? "" : " x")}{(oneCmd ? " e" : "")} u {data}";
                cmd.StandardInput.WriteLine(ukaz);
                cmd.StandardInput.Flush();
                proxmarkClientOpen = true;
            }
            else {
                Console.WriteLine("Already open");
            }
        }

        // When you preform the test tag reader must be on proxmark.
        private static void TestProxClient()
        {
            Thread.Sleep(500);

            SendProxmarkCMD("af2d9d489e652e03405039ecedcf18fe2f", true, true);

            SendProxmarkCMD("009aa5552b72d9eff85686fdf4ffb0dabe", true, false);

            SendProxmarkCMD("00b1b81f574ed0f3d4225533ad54fb91a43b250fbbf44e88b388290c68e3abe99b", true, true);

            SendProxmarkCMD("009aa5552b72d9eff85686fdf4ffb0dabe", true, false);

            SendProxmarkCMD("00b1b81f574ed0f3d4225533ad54fb91a43b250fbbf44e88b388290c68e3a99999", true, true);

        }
        private static void TestPN532()
        {
            port.WriteLine($"+6a*");
            Thread.Sleep(500);
            port.WriteLine($"+5a000000*");
            Thread.Sleep(500);
            port.WriteLine($"+45*");
            Thread.Sleep(500);
            port.WriteLine($"+5a494e55*");
            Thread.Sleep(500);
            port.WriteLine($"+45*");
            Thread.Sleep(500);
            port.WriteLine($"+5a10eeee*");
            Thread.Sleep(500);
            port.WriteLine($"+45*");
        }
    }
}
