using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StateLight.src
{
	class Controller
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
			pingTimer.Enabled = true;
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

		/// <summary>
		/// Write the Color to the device
		/// </summary>
		/// <param name="color">Color</param>
		public void SetColor(Color color)
		{
			int factor = Properties.Settings.Default.LedBrightness;
			int colorHex = (color.R / factor) << 16 | (color.G / factor) << 8 | color.B / factor;
			led.WriteColor(colorHex);
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
