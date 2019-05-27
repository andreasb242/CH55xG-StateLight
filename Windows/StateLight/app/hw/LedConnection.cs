using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Management;
using System.ComponentModel;
using System.IO.Ports;
using System.Runtime.CompilerServices;
using StateLight.app.hw;

namespace StateLight
{
	class LedConnection : ICmdResult
	{
		/// <summary>
		/// Queue to Queue commands
		/// </summary>
		private SerialCommandQueue queue = new SerialCommandQueue();

		/// <summary>
		/// Connection state
		/// </summary>
		public string ConnectionState = "Noch nicht verbunden";

		/// <summary>
		/// Current connection state
		/// </summary>
		public bool Connected = false;

		/// <summary>
		/// Constructor
		/// </summary>
		public LedConnection()
		{
		}

		/// <summary>
		/// Write all LED Color
		/// </summary>
		/// <param name="color">Color</param>
		public void WriteAllColor(int color)
		{
			WriteCommand("a" + string.Format("{0:X6}", color));
		}

		/// <summary>
		/// Write Color of single LED
		/// </summary>
		/// <param name="ledId">LED ID</param>
		/// <param name="color">Color to write</param>
		public void WriteLedColor(int ledId, int color)
		{
			WriteCommand("s" + ledId + " " + string.Format("{0:X6}", color));
		}

		/// <summary>
		/// Turn on blink for one or all LEDs
		/// </summary>
		/// <param name="ledId">LED ID, or 255 to blink all</param>
		public void WriteBlink(int ledId)
		{
			WriteCommand("b" + ledId);
		}

		/// <summary>
		/// Write Ping and sets timeout, so the device turns of if there is no ping anymore
		/// </summary>
		public void WritePing()
		{
			WriteCommand("w250");
		}

		/// <summary>
		/// Write a command to the LED, excpect OK as return
		/// </summary>
		/// <param name="command">Command to send</param>
		private void WriteCommand(string command)
		{
			queue.SheduleCommand(command, this);
		}

		/// <summary>
		/// Callback with the result data
		/// </summary>
		/// <param name="connected">True if connected / False if the device is not connected, the command failed</param>
		/// <param name="state">User readable message</param>
		public void CommandResult(bool connected, string state)
		{
			this.Connected = connected;
			this.ConnectionState = state;
		}
		/// <summary>
		/// Stop thread
		/// </summary>
		public void Stop()
		{
			queue.Stop();
		}
	}
}
