using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StateLightPluginDef
{
	/// <summary>
	/// Show the State on the LED
	/// </summary>
	public interface IStateProvider
	{
		/// <summary>
		/// Write state to LEDs
		/// </summary>
		/// <param name="state">State to write</param>
		/// <param name="additional">Additional information</param>
		void WriteState(string state, string additional);
	}

	/// <summary>
	/// Needs to be implemented in the plugin
	/// </summary>
	public interface IStateLightPluginDef
	{
		/// <summary>
		/// Checks if this plugin is supported (e.g. if the client is installed, which should be loaded)
		/// </summary>
		/// <returns>true if yes</returns>
		bool IsSupported();

		/// <summary>
		/// Get Plugin details, for Debugging or for about dialog
		/// </summary>
		/// <returns>Author, Version etc.</returns>
		string GetDetails();

		/// <summary>
		/// Name to display in the Menu
		/// </summary>
		/// <returns>Menu String</returns>
		string DisplayName();

		/// <summary>
		/// Start upadting the status
		/// </summary>
		/// <param name="state">State Provider</param>
		void Start(IStateProvider state);

		/// <summary>
		/// Stop plugin for reading the State
		/// </summary>
		void Stop();
	}
}
