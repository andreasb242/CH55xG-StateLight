using StateLight.app.controller;
using StateLight.src;
using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StateLight.app
{
	class PluginMenuItem : MenuItem
	{
		/// <summary>
		/// Main Controller
		/// </summary>
		private Controller controller;

		/// <summary>
		/// Plugin
		/// </summary>
		private PluginWrapper plugin;

		/// <summary>
		/// LED Connection to the hardware
		/// </summary>
		private LedConnection led;

		/// <summary>
		/// Plugin enabled / disabled
		/// </summary>
		private bool enabled = false;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="controller">Main Controller</param>
		/// <param name="plugin">Plugin</param>
		public PluginMenuItem(Controller controller, PluginWrapper plugin) : base(plugin.DisplayName())
		{
			this.controller = controller;
			this.plugin = plugin;

			OwnerDraw = true;
			Click += new EventHandler(MenuOnClick);
			DrawItem += new DrawItemEventHandler(MenuOnDrawItem);
			MeasureItem += new MeasureItemEventHandler(MenuOnMeasureItem);
		}

		/// <summary>
		/// Define size of Menu entry, which is painted by this class
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="miea">MeasureItemEventArgs</param>
		void MenuOnMeasureItem(object obj, MeasureItemEventArgs miea)
		{
			miea.ItemWidth = Properties.Settings.Default.MenuWidth;
			miea.ItemHeight = Properties.Settings.Default.MenuHeight;
		}

		/// <summary>
		/// Draw Menu
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="e">DrawItemEventArgs</param>
		void MenuOnDrawItem(object obj, DrawItemEventArgs e)
		{
			Graphics g = e.Graphics;
			e.DrawBackground();
			Rectangle bounds = e.Bounds;

			Brush foreground = new SolidBrush(e.ForeColor);

			if (enabled)
			{
				Pen pen = new Pen(foreground, 2);
				g.DrawLine(pen, bounds.Left + 0, bounds.Top + 10, bounds.Left + 6, bounds.Top + 16);
				g.DrawLine(pen, bounds.Left + 6, bounds.Top + 16, bounds.Left + 16, bounds.Top + 0);
			}

			g.DrawString(plugin.DisplayName(), e.Font, foreground, new PointF(bounds.Left + 20, bounds.Top));
		}

		/// <summary>
		/// Enable the menu and the plugin
		/// </summary>
		/// <param name="enabled"></param>
		public void SetEnabled(bool enabled)
		{
			if (this.enabled == enabled)
			{
				return;
			}

			this.enabled = enabled;

			if (enabled)
			{
				plugin.Start(controller);
			}
			else
			{
				plugin.Stop();
			}
		}

		/// <summary>
		/// Menu Entry selected
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="ea">EventArgs</param>
		void MenuOnClick(object obj, EventArgs ea)
		{
			SetEnabled(true);
		}
	}
}
