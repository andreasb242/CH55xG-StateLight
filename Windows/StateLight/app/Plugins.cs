﻿using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;

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
		private IStateLightPluginDef[] pluginList;

		/// <summary>
		/// Constructor
		/// </summary>
		public Plugins()
		{
		}

		/// <summary>
		/// Load plugin list
		/// </summary>
		public void loadPluginList()
		{
			String path = Application.StartupPath + "\\plugin";
			string[] pluginFiles = Directory.GetFiles(path, "*.dll");

			pluginList = (
				// From each file in the files.
				from file in pluginFiles
				// Load the assembly.
				let asm = Assembly.LoadFile(file)
				// For every type in the assembly that is visible outside of
				// the assembly.
				from type in asm.GetExportedTypes()
				// Where the type implements the interface.
				where typeof(IStateLightPluginDef).IsAssignableFrom(type)
				// Create the instance.
				select (IStateLightPluginDef)Activator.CreateInstance(type)
				// Materialize to an array.
			).ToArray();

			foreach (IStateLightPluginDef p in pluginList)
			{
				Console.WriteLine("Load Plugin: \"" + p.GetDetails() + "\", supported: " + p.IsSupported());
			}
		}
	}
}
