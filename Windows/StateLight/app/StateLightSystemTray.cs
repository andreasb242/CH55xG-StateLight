using StateLight.app;
using StateLight.src;
using StateLightPluginDef;
using System;
using System.Collections;
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

		/// <summary>
		/// Plugin menu items
		/// </summary>
		private List<PluginMenuItem> pluginMenuItems = new List<PluginMenuItem>();

		/// <summary>
		/// Constructor
		/// </summary>
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
			controller.SystemTray = this;

			trayIcon.MouseClick += (object sender, MouseEventArgs e) =>
			{
				if (e.Button != MouseButtons.Left)
				{
					return;
				}

				controller.IconClicked();
			};

			GenerateContextMenu();

			bool separatorAdded = false;

			foreach (IStateLightPluginDef p in controller.Plugins.PluginList)
			{
				if (!separatorAdded)
				{
					menu.MenuItems.Add("-");
					separatorAdded = true;
				}

				PluginMenuItem pm = new PluginMenuItem(controller, p);
				menu.MenuItems.Add(pm);
				pluginMenuItems.Add(pm);
			}


			menu.MenuItems.Add("-");
			menu.MenuItems.Add(new MenuItem("Exit", (object sender, EventArgs e) => { controller.ShutdownApplication(); }));
		}

		/// <summary>
		/// Disable all Plugin menus
		/// </summary>
		public void DisableAllPluginMenus()
		{
			foreach (PluginMenuItem it in pluginMenuItems)
			{
				it.SetEnabled(false);
			}
		}

		/// <summary>
		/// Load context menu from config
		/// </summary>
		private void GenerateContextMenu()
		{
			ConfigParser cfg = new ConfigParser(Properties.Settings.Default.States);
			foreach (KeyValuePair<string, ColorList> e in cfg.Values)
			{
				ColorMenuItem cm = new ColorMenuItem(controller, e.Key, e.Value);
				menu.MenuItems.Add(cm);
			}
		}

	}
}
