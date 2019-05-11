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
		/// Constructor
		/// </summary>
		public Controller()
		{
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
			TrayIcon.BalloonTipTitle = "State Light";
			TrayIcon.BalloonTipText = "Hier sollte der Gerätestatus stehen...";
			TrayIcon.BalloonTipIcon = ToolTipIcon.Info;
			TrayIcon.ShowBalloonTip(10000);
		}
	}
}
