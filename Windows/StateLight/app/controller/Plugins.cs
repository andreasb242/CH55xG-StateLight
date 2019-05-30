using StateLight.app.controller;
using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace StateLight.app
{
	/// <summary>
	/// Plugin loader
	/// </summary>
	class Plugins
	{
		/// <summary>
		/// Array with all loaded Plugins
		/// </summary>
		private List<PluginWrapper> pluginList = new List<PluginWrapper>();

		/// <summary>
		/// Array with all loaded Plugins
		/// </summary>
		public List<PluginWrapper> PluginList { get { return pluginList; } }

		/// <summary>
		/// Constructor
		/// </summary>
		public Plugins()
		{
		}

		/// <summary>
		/// Load plugin list
		/// </summary>
		public void LoadPluginList()
		{
			String path = Application.StartupPath + "\\Plugin";
			if (!Directory.Exists(path))
			{
				Console.WriteLine("Plugin folder \"" + path + "\" does not exist");
				return;
			}

			foreach (string p in Directory.GetDirectories(path))
			{
				string pluginXml = p + "/plugin.xml";
				if (!File.Exists(pluginXml))
				{
					continue;
				}

				try
				{
					PluginWrapper plugin = new PluginWrapper(pluginXml);
					if (!plugin.IsSupported())
					{
						Console.WriteLine("Plugin \"" + plugin.GetDetails() + "\" is not supported");
						continue;
					}

					Console.WriteLine("Plugin \"" + plugin.GetDetails() + "\" loaded");
					this.pluginList.Add(plugin);
				}
				catch (Exception e)
				{
					Console.WriteLine("Exception loading plugin: " + e.ToString());
				}
			}
		}
	}
}
