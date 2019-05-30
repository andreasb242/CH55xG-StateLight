using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Xml;

namespace StateLight.app.controller
{
	class PluginWrapper : IStateLightPluginDef
	{
		/// <summary>
		/// True if the plugin is loaded successfully
		/// </summary>
		private bool pluginLoaded = false;

		/// <summary>
		/// Plugin implementation
		/// </summary>
		private IStateLightPluginDef pluginImpl;

		/// <summary>
		/// Plugin Details
		/// </summary>
		private string details;

		/// <summary>
		/// Plugin Name
		/// </summary>
		private string name;

		/// <summary>
		/// DLL Loading Path
		/// </summary>
		private string dllLoadingPath;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="xmlPath">Plugin.xml Path</param>
		public PluginWrapper(string xmlPath)
		{
			LoadPlugin(xmlPath);
		}

		/// <summary>
		/// Checks if a plugin should be loaded and load it
		/// </summary>
		/// <param name="xmlPath">plugin.xml Path</param>
		private void LoadPlugin(string xmlPath)
		{
			XmlDocument doc = new XmlDocument();
			doc.Load(xmlPath);

			bool enabled = false;
			XmlNode pluginNode = doc.SelectSingleNode("plugin");

			this.name = pluginNode.SelectSingleNode("name").InnerText;
			this.details = pluginNode.SelectSingleNode("info").InnerText;

			foreach (XmlNode check in pluginNode.SelectNodes("check"))
			{
				XmlNode fileNode = check.SelectSingleNode("file");
				string filePath = fileNode.InnerText;

				foreach (XmlNode metadata in check.SelectNodes("metadata"))
				{
					if (metadata.Attributes["name"].InnerText == "description")
					{
						FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(filePath);
						if (versionInfo.FileDescription == metadata.InnerText)
						{
							enabled = true;
						}
					}
				}
			}

			if (!enabled)
			{
				return;
			}

			string path = pluginNode.SelectSingleNode("path").InnerText;
			this.dllLoadingPath = new FileInfo(path).Directory.FullName;
			string dllPath = new FileInfo(dllLoadingPath + "\\" + path).FullName;

			AppDomain.CurrentDomain.ReflectionOnlyAssemblyResolve += ResolveDpendency;
			AppDomain.CurrentDomain.AssemblyResolve += ResolveDpendency;

			Console.WriteLine("Load Plugin DLL: " + dllPath);

			LoadPluginDll(dllPath);

			AppDomain.CurrentDomain.ReflectionOnlyAssemblyResolve -= ResolveDpendency;
			AppDomain.CurrentDomain.AssemblyResolve -= ResolveDpendency;
		}

		/// <summary>
		/// DLL Resolver
		/// </summary>
		/// <param name="sender">Sender</param>
		/// <param name="e"></param>
		/// <returns>ResolveEventArgs</returns>
		private Assembly ResolveDpendency(Object sender, ResolveEventArgs e)
		{
			string[] tokens = e.Name.Split(",".ToCharArray());
			System.Diagnostics.Debug.WriteLine("Resolving: " + e.Name);
			return Assembly.LoadFile(Path.Combine(new string[] { dllLoadingPath, tokens[0] + ".dll" }));
		}

		/// <summary>
		/// Load Plugin DLL
		/// </summary>
		/// <param name="dllPath">DLL Path</param>
		private void LoadPluginDll(string dllPath)
		{
			IEnumerable<IStateLightPluginDef> pluginList = null;
			try
			{
				// Load the assembly.
				Assembly asm = Assembly.LoadFile(dllPath);

				// For every type in the assembly that is visible outside of
				// the assembly.
				pluginList = from type in asm.GetExportedTypes()

								 // Where the type implements the interface.
							 where typeof(IStateLightPluginDef).IsAssignableFrom(type)

							 // Create the instance.
							 select (IStateLightPluginDef)Activator.CreateInstance(type);
			}
			catch (Exception e)
			{
				Console.WriteLine("Exception loading plugin: " + e.ToString());
				return;
			}

			foreach (IStateLightPluginDef p in pluginList)
			{
				pluginImpl = p;
				pluginLoaded = true;
				break;
			}
		}


		/// <summary>
		/// Checks if this plugin is supported (e.g. if the client is installed, which should be loaded)
		/// </summary>
		/// <returns>true if yes</returns>
		public bool IsSupported()
		{
			return pluginLoaded;
		}

		/// <summary>
		/// Get Plugin details, for Debugging or for about dialog
		/// </summary>
		/// <returns>Author, Version etc.</returns>
		public string GetDetails()
		{
			return this.details;
		}

		/// <summary>
		/// Name to display in the Menu
		/// </summary>
		/// <returns>Menu String</returns>
		public string DisplayName()
		{
			return this.name;
		}

		/// <summary>
		/// Start upadting the status
		/// </summary>
		/// <param name="state">State Provider</param>
		public void Start(IStateProvider state)
		{
			pluginImpl.Start(state);
		}

		/// <summary>
		/// Stop plugin for reading the State
		/// </summary>
		public void Stop()
		{
			pluginImpl.Stop();
		}
	}
}
