using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Management;
using System.ComponentModel;
using System.IO.Ports;
using System.Runtime.CompilerServices;

namespace StateLight
{
	class LedConnection
	{
		/// <summary>
		/// Serial Port to access the USB Device
		/// </summary>
		private SerialPort port = new SerialPort();

		/// <summary>
		/// Current connection state
		/// </summary>
		public bool Connected = false;

		/// <summary>
		/// Connection state
		/// </summary>
		public string ConnectionState = "Noch nicht verbunden";

		/// <summary>
		/// Try to open the serial port
		/// </summary>
		/// <returns>true if opened</returns>
		private bool OpenSerialPort()
		{
			string portName = FindLedComPort();
			if (portName == null)
			{
				Connected = false;
				ConnectionState = "Kein Gerät gefunden";

				Console.WriteLine("No device found");
				return false;
			}

			Console.WriteLine("Open COM Port " + portName);

			port.PortName = portName;

			// Baud rate doesen't matter, it's an USB Device
			port.BaudRate = 9600;

			// Set the read/write timeouts
			port.ReadTimeout = 500;
			port.WriteTimeout = 10;
			return true;
		}

		/// <summary>
		/// Write all LED Color
		/// </summary>
		/// <param name="color">Color</param>
		public void WriteAllColor(int color)
		{
			WriteCommand("a" + string.Format("{0:X6}", color));
		}

		/// <summary>
		/// Write Color of single LED
		/// </summary>
		/// <param name="ledId">LED ID</param>
		/// <param name="color">Color to write</param>
		public void WriteLedColor(int ledId, int color)
		{
			WriteCommand("s" + ledId + " " + string.Format("{0:X6}", color));
		}

		/// <summary>
		/// Turn on blink for one or all LEDs
		/// </summary>
		/// <param name="ledId">LED ID, or 255 to blink all</param>
		public void WriteBlink(int ledId)
		{
			WriteCommand("b" + ledId);
		}

		/// <summary>
		/// Write Ping and sets timeout, so the device turns of if there is no ping anymore
		/// </summary>
		public void WritePing()
		{
			WriteCommand("w250");
		}

		/// <summary>
		/// Write a command to the LED, excpect OK as return
		/// </summary>
		/// <param name="command">Command to send</param>
		[MethodImpl(MethodImplOptions.Synchronized)]
		private void WriteCommand(string command)
		{
			Console.WriteLine("Send Command to device: \"" + command + "\"");

			try
			{
				if (!port.IsOpen)
				{
					if (!OpenSerialPort())
					{
						return;
					}
				}

				port.Open();

				command += "\n";
				port.Write(command);
				string result = port.ReadLine();
				result = result.Trim();
				Console.WriteLine("Device Respond: \"" + result + "\"");

				if (result.Length >= 2 && result.Substring(0, 2).Equals("OK"))
				{
					Connected = true;
					ConnectionState = "Verbunden";
				}
				else
				{
					Connected = false;
					ConnectionState = "Gerätefehler: " + result;
				}

				port.Close();
			}
			catch (Exception ex)
			{
				port.Close();
				Console.WriteLine("Kommunikationsfehler");
				Console.WriteLine(ex.ToString());

				Connected = false;
				ConnectionState = "Kommunikationsfehler: " + ex.ToString();
			}
		}

		private static ConnectionOptions ProcessConnectionOptions()
		{
			ConnectionOptions options = new ConnectionOptions();
			options.Impersonation = ImpersonationLevel.Impersonate;
			options.Authentication = AuthenticationLevel.Default;
			options.EnablePrivileges = true;
			return options;
		}

		private static ManagementScope ConnectionScope(string machineName, ConnectionOptions options, string path)
		{
			ManagementScope connectScope = new ManagementScope();
			connectScope.Path = new ManagementPath(@"\\" + machineName + path);
			connectScope.Options = options;
			connectScope.Connect();
			return connectScope;
		}

		/// <summary>
		/// Find the correct COM Port for the Device
		/// </summary>
		/// <returns>ComPort name, or null, if no device was found</returns>
		public string FindLedComPort()
		{
			ConnectionOptions options = ProcessConnectionOptions();
			ManagementScope connectionScope = ConnectionScope(Environment.MachineName, options, @"\root\CIMV2");
			ObjectQuery objectQuery = new ObjectQuery("SELECT * FROM Win32_PnPEntity WHERE ConfigManagerErrorCode = 0");

			using (ManagementObjectSearcher comPortSearcher = new ManagementObjectSearcher(connectionScope, objectQuery))
			{
				foreach (ManagementObject obj in comPortSearcher.Get())
				{
					if (obj == null)
					{
						continue;
					}

					object captionObj = obj["Caption"];
					if (captionObj == null)
					{
						continue;
					}

					string caption = captionObj.ToString();
					if (!caption.Contains("(COM"))
					{
						continue;
					}

					// Debug output attributes
					// Console.WriteLine("==================");
					string path = "";
					foreach (PropertyDescriptor descriptor in TypeDescriptor.GetProperties(obj))
					{
						string name = descriptor.Name;
						object value = descriptor.GetValue(obj);

						if (name.Equals("Path") && value != null)
						{
							path = value.ToString();
							break;
						}

						// Debug output attributes
						// Console.WriteLine("{0}={1}", name, value);
					}

					if (!path.Contains(Properties.Settings.Default.UsbIdSearch))
					{
						continue;
					}

					string comPort = caption.Substring(caption.LastIndexOf("(COM") + 1);
					return comPort.Substring(0, comPort.LastIndexOf(')'));
				}
			}

			return null;
		}
	}
}
