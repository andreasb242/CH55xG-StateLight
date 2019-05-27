using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;

namespace StateLight.app
{
	/// <summary>
	/// Class to store 1 or 6 colors and draw it to an icon
	/// </summary>
	class ColorList
	{
		/// <summary>
		/// List of all colors
		/// </summary>
		public Color[] Colors { get; }

		/// <summary>
		/// Blink flag for all Colors
		/// </summary>
		public bool[] Blink { get; }

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="color">Color string to parse</param>
		public ColorList(string color)
		{
			string[] tmpColors = color.Split('-');

			int count = tmpColors.Length;

			if (count >= 6)
			{
				count = 6;
			}
			else if (count > 0)
			{
				count = 1;
			}
			else
			{
				Colors = new Color[1] { Color.Black };
				Blink = new bool[1] { false };
				return;
			}

			Colors = new Color[count];
			Blink = new bool[count];

			for (int i = 0; i < count; i++)
			{
				Blink[i] = false;

				string c = tmpColors[i];

				if (c.Length == 8)
				{
					if (c.EndsWith("b"))
					{
						Blink[i] = true;
					}
					c = c.Substring(0, 7);
				}

				Colors[i] = System.Drawing.ColorTranslator.FromHtml(c);
			}
		}

		public void DrawIcon(Graphics g, Brush foreground, int left, int top)
		{
			if (Colors.Length == 1)
			{
				Rectangle r = new Rectangle(left + 4, top + 4, 8, 8);
				g.FillRectangle(new SolidBrush(Colors[0]), r);
				g.DrawRectangle(new Pen(foreground), r);
			}
			else
			{
				// 5 3 1
				// 6 4 2

				int i = 0;
				for (int px = 0; px < 3; px++)
				{
					for (int py = 0; py < 2; py++)
					{
						int x = 16 - (px + 1) * 5;
						int y = py * 8;

						Rectangle r = new Rectangle(left + x, top + y, 5, 8);
						g.FillRectangle(new SolidBrush(Colors[i]), r);
						g.DrawRectangle(new Pen(foreground), r);

						i++;
					}
				}
			}
		}
	}
}
