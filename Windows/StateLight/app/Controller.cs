using StateLight.app;
using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StateLight.src
{
	class Controller : IStateProvider
	{
		/// <summary>
		/// LED Connection to the hardware
		/// </summary>
		private LedConnection led = new LedConnection();

		/// <summary>
		/// Tray Icon
		/// </summary>
		public NotifyIcon TrayIcon;

		/// <summary>
		/// Plugin handler list
		/// </summary>
		public Plugins plugins = new Plugins();

		/// <summary>
		/// Ping Timer, to make sure the device is connected and the device does not auto turn off
		/// </summary>
		private Timer pingTimer = new Timer
		{
			Interval = 1000
		};

		/// <summary>
		/// Constructor
		/// </summary>
		public Controller()
		{
			plugins.loadPluginList();

			pingTimer.Enabled = Properties.Settings.Default.WatchdogActive;
			pingTimer.Tick += new System.EventHandler(OnPingTimer);
		}

		/// <summary>
		/// Ping timer event
		/// </summary>
		private void OnPingTimer(object sender, EventArgs e)
		{
			// Communicate with the Controller, to make sure it don't get turn off
			// Set the timeout to 2.5 second, and repeat this every second
			led.WriteCommand("w250\n");
		}

		/// <summary>
		/// Shutdown the application
		/// </summary>
		public void ShutdownApplication()
		{
			pingTimer.Stop();

			// Hide tray icon, otherwise it will remain shown until user mouses over it
			TrayIcon.Visible = false;

			Application.Exit();
		}

		public void WriteState(string state, string additional)
		{
			// TODO Implement
			Console.WriteLine("=>" + state + " | " + additional);
		}

		/// <summary>
		/// Write the Color to the device
		/// </summary>
		/// <param name="colorList">Color</param>
		public void SetColor(ColorList colorList)
		{
			Bitmap bmp = new Bitmap(16, 16);
			using (Graphics g = Graphics.FromImage(bmp))
			{
				colorList.DrawIcon(g, new SolidBrush(Color.White), 0, 0);
			}
			TrayIcon.Icon = Icon.FromHandle(bmp.GetHicon());

			if (colorList.Colors.Length == 1)
			{
				Color color = colorList.Colors[0];
				int factor = Properties.Settings.Default.LedBrightness;
				int colorHex = (color.R / factor) << 16 | (color.G / factor) << 8 | color.B / factor;
				led.WriteAllColor(colorHex);

				if (colorList.Blink[0])
				{
					led.WriteBlink(255);
				}
			}
			else
			{
				for (int i = 0; i < colorList.Colors.Length; i++)
				{
					Color color = colorList.Colors[i];
					int factor = Properties.Settings.Default.LedBrightness;
					int colorHex = (color.R / factor) << 16 | (color.G / factor) << 8 | color.B / factor;
					led.WriteLedColor(i, colorHex);

					if (colorList.Blink[i])
					{
						led.WriteBlink(i);
					}
				}
			}
		}

		/// <summary>
		/// Application icon was clicked
		/// </summary>
		public void IconClicked()
		{
			if (led.Connected)
			{
				TrayIcon.BalloonTipIcon = ToolTipIcon.Info;
				TrayIcon.BalloonTipTitle = "Verbunden";
				TrayIcon.BalloonTipText = "State LED verbunden";
			}
			else
			{
				TrayIcon.BalloonTipIcon = ToolTipIcon.Error;
				TrayIcon.BalloonTipTitle = "Fehler";
				TrayIcon.BalloonTipText = led.ConnectionState;
			}

			TrayIcon.ShowBalloonTip(10000);
		}
	}
}
