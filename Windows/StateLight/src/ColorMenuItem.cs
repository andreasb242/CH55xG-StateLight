using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StateLight.src
{
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
