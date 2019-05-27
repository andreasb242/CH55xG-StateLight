using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;

namespace StateLight.app.hw
{
	/// <summary>
	/// Callback to get current device state
	/// </summary>
	interface ICmdResult
	{
		/// <summary>
		/// Callback with the result data
		/// </summary>
		/// <param name="connected">True if connected / False if the device is not connected, the command failed</param>
		/// <param name="state">User readable message</param>
		void CommandResult(bool connected, string state);
	}

	/// <summary>
	/// A single Queue Entry
	/// </summary>
	class QueueEntry
	{
		/// <summary>
		/// Command
		/// </summary>
		public string Command;

		/// <summary>
		/// Listener
		/// </summary>
		public ICmdResult ResultListener;
	}

	/// <summary>
	/// Queue Commands to send to Serial Port
	/// </summary>
	class SerialCommandQueue
	{
		/// <summary>
		/// Serial Port to access the USB Device
		/// </summary>
		private SerialPort port = new SerialPort();

		/// <summary>
		/// Internal queue
		/// </summary>
		private Queue<QueueEntry> queue = new Queue<QueueEntry>();

		/// <summary>
		/// Communication Thread
		/// </summary>
		private Thread thread;

		/// <summary>
		/// Running flag
		/// </summary>
		private bool running = true;

		/// <summary>
		/// Constructor
		/// </summary>
		public SerialCommandQueue()
		{
			thread = new Thread(() =>
			{
				Thread.CurrentThread.IsBackground = true;
				BackgroundRunner();
			});

			thread.Start();
		}

		/// <summary>
		/// Executed in the Background Thread
		/// </summary>
		private void BackgroundRunner()
		{
			while (running)
			{
				ExecuteTask();
			}
		}

		/// <summary>
		/// Execute a single queued task
		/// </summary>
		private void ExecuteTask()
		{
			QueueEntry cmd = null;
			lock (queue)
			{
				while (queue.Count == 0)
				{
					if (!running)
					{
						return;
					}

					System.Threading.Monitor.Wait(queue);
				}

				cmd = queue.Dequeue();
			}

			if (cmd == null)
			{
				return;
			}

			WriteCommand(cmd);
		}

		/// <summary>
		/// Write a command to the LED, excpect OK as return
		/// </summary>
		/// <param name="cmd">Command to send</param>
		private void WriteCommand(QueueEntry cmd)
		{
			try
			{
				if (!port.IsOpen)
				{
					if (!OpenSerialPort())
					{
						cmd.ResultListener.CommandResult(false, "Kein Gerät gefunden");
						return;
					}
				}

				port.Open();

				string command = cmd.Command + "\n";
				port.Write(command);
				string result = port.ReadLine();
				result = result.Trim();
				Console.WriteLine("Device Respond: \"" + result + "\"");

				if (result.Length >= 2 && result.Substring(0, 2).Equals("OK"))
				{
					cmd.ResultListener.CommandResult(true, "Verbunden");
				}
				else
				{
					cmd.ResultListener.CommandResult(false, "Gerätefehler: " + result);
				}

				port.Close();
			}
			catch (Exception ex)
			{
				port.Close();
				Console.WriteLine("Kommunikationsfehler");
				Console.WriteLine(ex.ToString());

				cmd.ResultListener.CommandResult(false, "Kommunikationsfehler: " + ex.ToString());
			}
		}

		/// <summary>
		/// Shedule a command to send
		/// </summary>
		/// <param name="command">Serial Command, without newline</param>
		/// <param name="callback">Callback to get result</param>
		public void SheduleCommand(string command, ICmdResult callback)
		{
			Console.WriteLine("Shedule send command: \"" + command + "\"");
			lock (queue)
			{
				QueueEntry cmd = new QueueEntry();
				cmd.Command = command;
				cmd.ResultListener = callback;

				queue.Enqueue(cmd);
				System.Threading.Monitor.Pulse(queue);
			}
		}

		/// <summary>
		/// Try to open the serial port
		/// </summary>
		/// <returns>true if opened</returns>
		private bool OpenSerialPort()
		{
			ComPortFinder finder = new ComPortFinder();
			string portName = finder.FindLedComPort();
			if (portName == null)
			{
				return false;
			}

			Console.WriteLine("Open COM Port " + portName);

			port.PortName = portName;

			// Baud rate doesen't matter, it's an USB Device
			port.BaudRate = 9600;

			// Set the read/write timeouts
			port.ReadTimeout = Properties.Settings.Default.serialTimeoutRead;
			port.WriteTimeout = Properties.Settings.Default.serialTimeoutWrite;
			return true;
		}

		/// <summary>
		/// Stop thread
		/// </summary>
		public void Stop()
		{
			running = false;
			lock (queue)
			{
				System.Threading.Monitor.Pulse(queue);
			}
		}
	}
}
