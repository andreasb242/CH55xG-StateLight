using StateLight.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.IO;
using System.Globalization;

namespace StateLight
{
	static class Program
	{
		/// <summary>
		/// Logfile / Stream
		/// </summary>
		static FileStream ostrm;

		/// <summary>
		/// Logfile / Stream
		/// </summary>
		static StreamWriter writer;

		/// <summary>
		/// Der Haupteinstiegspunkt für die Anwendung.
		/// </summary>
		/// <param name="args">Command line Arguments</param>
		[STAThread]
		static void Main(string[] args)
		{
			if (args.Length > 0 && args[0].Equals("d"))
			{
				RedirectLog();
			}

			Console.WriteLine("** Startup **");
			Console.WriteLine(DateTime.Now.ToString(new CultureInfo("de-DE")));

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			Application.Run(new StateLightSystemTray());

			if (writer != null)
			{
				writer.Close();
			}
			if (ostrm != null)
			{
				ostrm.Close();
			}
		}

		/// <summary>
		/// Redirect standard output to logfile
		/// </summary>
		private static void RedirectLog()
		{
			try
			{
				ostrm = new FileStream("Error.log", FileMode.OpenOrCreate, FileAccess.Write);
				writer = new StreamWriter(ostrm);
			}
			catch (Exception e)
			{
				Console.WriteLine("Cannot open Error.log for writing");
				Console.WriteLine(e.ToString());
				return;
			}
			Console.SetOut(writer);
		}
	}
}
