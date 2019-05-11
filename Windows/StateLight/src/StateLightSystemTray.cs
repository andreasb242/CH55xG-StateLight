using StateLight.src;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;

namespace StateLight
{
	class StateLightSystemTray : ApplicationContext
	{
		/// <summary>
		/// Tray Icon
		/// </summary>
		private NotifyIcon trayIcon;

		/// <summary>
		/// Menu
		/// </summary>
		private ContextMenu menu = new ContextMenu();

		/// <summary>
		/// LED Connection to the hardware
		/// </summary>
		private LedConnection led = new LedConnection();

		public StateLightSystemTray()
		{
			// Initialize Tray Icon
			trayIcon = new NotifyIcon()
			{
				Icon = Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location),
				ContextMenu = menu,
				Visible = true
			};
			trayIcon.MouseClick += (object sender, MouseEventArgs e) =>
			{
				if (e.Button != MouseButtons.Left)
				{
					return;
				}

				trayIcon.BalloonTipTitle = "State Light";
				trayIcon.BalloonTipText = "Hier sollte der Gerätestatus stehen...";
				trayIcon.BalloonTipIcon = ToolTipIcon.Info;
				trayIcon.ShowBalloonTip(10000);
			};

			GenerateContextMenu();

			menu.MenuItems.Add("-");
			menu.MenuItems.Add(new MenuItem("Exit", menuExit));
		}

		private void GenerateContextMenu()
		{
			foreach (string line in Properties.Settings.Default.States.Split('\n'))
			{
				string line2 = line.Trim();
				if (line2.Equals(""))
				{
					continue;
				}

				ParseLine(line2);
			}
		}

		private void ParseLine(string line)
		{
			int pos = line.IndexOf(':');
			string name = line.Substring(0, pos).Trim();
			string color = line.Substring(pos + 1).Trim();

			ColorMenuItem cm = new ColorMenuItem(led, name, System.Drawing.ColorTranslator.FromHtml(color));
			menu.MenuItems.Add(cm);
		}

		void menuExit(object sender, EventArgs e)
		{
			// Hide tray icon, otherwise it will remain shown until user mouses over it
			trayIcon.Visible = false;

			Application.Exit();
		}
	}
}
