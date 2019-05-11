using StateLight.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace StateLight
{
	static class Program
	{
		/// <summary>
		/// Der Haupteinstiegspunkt für die Anwendung.
		/// </summary>
		[STAThread]
		static void Main()
		{

			Console.WriteLine("Startup");

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
//			Application.Run(new Form1());
			Application.Run(new StateLightSystemTray());
		}
	}
}
