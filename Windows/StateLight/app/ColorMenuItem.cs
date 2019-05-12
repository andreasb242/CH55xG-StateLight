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
		/// Menu size in Pixel
		/// </summary>
		private const int MENU_HEIGHT = 22;

		/// <summary>
		/// Menu size in Pixel
		/// </summary>
		private const int MENU_WIDTH = 150;

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
			Click += new EventHandler(MenuHelpOnClick);
			DrawItem += new DrawItemEventHandler(MenuHelpOnDrawItem);
			MeasureItem += new MeasureItemEventHandler(MenuHelpOnMeasureItem);
		}

		/// <summary>
		/// Define size of Menu entry, which is painted by this class
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="miea">MeasureItemEventArgs</param>
		void MenuHelpOnMeasureItem(object obj, MeasureItemEventArgs miea)
		{
			miea.ItemWidth = MENU_WIDTH;
			miea.ItemHeight = MENU_HEIGHT;
		}

		/// <summary>
		/// Draw Menu
		/// </summary>
		/// <param name="obj">Object</param>
		/// <param name="e">DrawItemEventArgs</param>
		void MenuHelpOnDrawItem(object obj, DrawItemEventArgs e)
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
		void MenuHelpOnClick(object obj, EventArgs ea)
		{
			controller.SetColor(this.colorList);
		}
	}
}
