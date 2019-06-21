using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StateLight.app
{
	/// <summary>
	/// Parse Configuration
	/// </summary>
	class ConfigParser
	{ 
		/// <summary>
		/// Parsed values, Key => Color
		/// </summary>
		public Dictionary<string, ColorList> Values = new Dictionary<string, ColorList>();

		/// <summary>
		/// Parse Configuration string with Color values
		/// </summary>
		/// <param name="config">String to parse</param>
		public ConfigParser(string config)
		{
			foreach (string line in config.Split('\n'))
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

			Values[name] = new ColorList(color);
		}
	}
}
