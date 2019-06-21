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
