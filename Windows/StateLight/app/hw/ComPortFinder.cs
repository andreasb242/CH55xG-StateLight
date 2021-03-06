﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Management;
using System.ComponentModel;
using System.IO.Ports;
using System.Runtime.CompilerServices;


namespace StateLight.app.hw
{
	/// <summary>
	/// Find the correct COM Port
	/// </summary>
	class ComPortFinder
	{

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
