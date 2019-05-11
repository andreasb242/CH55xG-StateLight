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


	class ColorMenuItem : MenuItem
	{
		/// <summary>
		/// Menu Item Name
		/// </summary>
		private String name;

		/// <summary>
		/// Color
		/// </summary>
		private Color color;

		/// <summary>
		/// LED Connection to the hardware
		/// </summary>
		private LedConnection led;

		private const int MENU_HEIGHT = 22;
		private const int MENU_WIDTH = 150;

		public ColorMenuItem(LedConnection led, string name, Color color) : base(name)
		{
			this.led = led;
			this.name = name;
			this.color = color;

			OwnerDraw = true;
			Click += new EventHandler(MenuHelpOnClick);
			DrawItem += new DrawItemEventHandler(MenuHelpOnDrawItem);
			MeasureItem += new MeasureItemEventHandler(MenuHelpOnMeasureItem);
		}

		void MenuHelpOnMeasureItem(object obj, MeasureItemEventArgs miea)
		{
			miea.ItemWidth = MENU_WIDTH;
			miea.ItemHeight = MENU_HEIGHT;
		}

		void MenuHelpOnDrawItem(object obj, DrawItemEventArgs e)
		{
			Graphics g = e.Graphics;
			e.DrawBackground();
			Rectangle bounds = e.Bounds;

			Brush foreground = new SolidBrush(e.ForeColor);
			Rectangle r = new Rectangle(bounds.Left + 4, bounds.Top + 4, 8, 8);
			g.FillRectangle(new SolidBrush(color), r);
			g.DrawRectangle(new Pen(foreground), r);
			g.DrawString(name, e.Font, foreground, new PointF(bounds.Left + 20, bounds.Top));
		}

		void MenuHelpOnClick(object obj, EventArgs ea)
		{
			int factor = Properties.Settings.Default.LedBrightness;
			int color = (this.color.R / factor) << 16 | (this.color.G / factor) << 8 | this.color.B / factor;
			led.WriteColor(color);
		}
	}
}
