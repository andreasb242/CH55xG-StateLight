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
		/// Main Controller
		/// </summary>
		private Controller controller = new Controller();
		public Controller Controller { get { return controller; } }

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
				Icon = Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location),
				ContextMenu = menu,
				Visible = true
			};

			controller.TrayIcon = trayIcon;

			trayIcon.MouseClick += (object sender, MouseEventArgs e) =>
			{
				if (e.Button != MouseButtons.Left)
				{
					return;
				}

				controller.IconClicked();
			};

			GenerateContextMenu();

			menu.MenuItems.Add("-");
			menu.MenuItems.Add(new MenuItem("Exit", menuExit));
		}

		/// <summary>
		/// Load context menu from config
		/// </summary>
		private void GenerateContextMenu()
		{
			foreach (string line in Properties.Settings.Default.States.Split('\n'))
			{
				string line2 = line.Trim();
				if (line2.Equals("") || line.StartsWith("#"))
				{
					// Ignore empty lines or comments
					continue;
				}

				ParseLine(line2);
			}
		}

		/// <summary>
		/// Parse a single line from config
		/// </summary>
		/// <param name="line"></param>
		private void ParseLine(string line)
		{
			int pos = line.IndexOf(':');
			string name = line.Substring(0, pos).Trim();
			string color = line.Substring(pos + 1).Trim();

			ColorMenuItem cm = new ColorMenuItem(controller, name, System.Drawing.ColorTranslator.FromHtml(color));
			menu.MenuItems.Add(cm);
		}

		/// <summary>
		/// Menu Exit was pressed
		/// </summary>
		/// <param name="sender">Object</param>
		/// <param name="e">EventArgs</param>
		void menuExit(object sender, EventArgs e)
		{
			// Hide tray icon, otherwise it will remain shown until user mouses over it
			trayIcon.Visible = false;

			Application.Exit();
		}
	}
}
