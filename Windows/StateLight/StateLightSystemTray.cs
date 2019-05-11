using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
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

		public StateLightSystemTray()
		{
			// Initialize Tray Icon
			trayIcon = new NotifyIcon()
			{
				Icon = new Icon("icon.ico"),
				ContextMenu = menu,
				Visible = true
			};

			GenerateContextMenu();

			menu.MenuItems.Add("-");
			menu.MenuItems.Add(new MenuItem("Exit", menuExit));
		}

		private void GenerateContextMenu()
		{
			using (StreamReader f = new StreamReader("States.txt"))
			{
				string line;
				while ((line = f.ReadLine()) != null)
				{
					ParseLine(line);
				}
			}
		}

		private void ParseLine(string line)
		{
			int pos = line.IndexOf(':');
			string name = line.Substring(0, pos).Trim();
			string color = line.Substring(pos + 1).Trim();

			ColorMenuItem cm = new ColorMenuItem(name);
			menu.MenuItems.Add(cm);
		}

		void menuExit(object sender, EventArgs e)
		{
			// Hide tray icon, otherwise it will remain shown until user mouses over it
			trayIcon.Visible = false;

			Application.Exit();
		}
	}


	class ColorMenuItem : MenuItem
	{
		public ColorMenuItem(string name) : base(name)
		{
			Bitmap image = new Bitmap(16, 16);
			using (Graphics g = Graphics.FromImage(image))
			{
				g.DrawRectangle(Pens.Red, 4, 4, 8, 8);
			}

//			Icon = Icon.FromHandle(image.GetHicon());

			// DrawItem += delegate (object sender, DrawItemEventArgs e) {
			//	double factor = (double)e.Bounds.Height / zeroIconBmp.Height;
			//	var rect = new Rectangle(e.Bounds.X, e.Bounds.Y,
			//								(int)(zeroIconBmp.Width * factor),
			//								(int)(zeroIconBmp.Height * factor));
			//	e.Graphics.DrawImage(zeroIconBmp, rect);
			//};
		}
	}
}
