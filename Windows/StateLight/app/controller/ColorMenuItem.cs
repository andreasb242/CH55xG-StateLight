using StateLight.app;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StateLight.src
{
	/// <summary>
	/// A single Popup Menu Entry
	/// </summary>
	class ColorMenuItem : MenuItem
	{
		/// <summary>
		/// Main Controller
		/// </summary>
		private Controller controller;

		/// <summary>
		/// Menu Item Name
		/// </summary>
		private String name;

		/// <summary>
		/// Color
		/// </summary>
		private ColorList colorList;

		/// <summary>
		/// LED Connection to the hardware
		/// </summary>
		private LedConnection led;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="controller">Main Controller</param>
		/// <param name="name">Menu Item Name</param>
		/// <param name="colorList">Color</param>
		public ColorMenuItem(Controller controller, string name, ColorList colorList) : base(name)
		{
			this.controller = controller;
			this.name = name;
			this.colorList = colorList;

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

			colorList.DrawIcon(g, foreground, bounds.Left, bounds.Top);
			g.DrawString(name, e.Font, foreground, new PointF(bounds.Left + 20, bounds.Top));
		}

		/// <summary>
		/// Menu Entry selected
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="ea">EventArgs</param>
		void MenuOnClick(object obj, EventArgs ea)
		{
			controller.SetManualColor(this.colorList);
		}
	}
}
