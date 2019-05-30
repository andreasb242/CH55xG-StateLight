using System;
using System.Threading;
using Microsoft.Lync.Model;
using StateLightPluginDef;


// TODO Handle incomming calls: https://stackoverflow.com/questions/9207549/detecting-an-incoming-call-in-lync
namespace Lync2010Plugin
{
	public class Lync2010PluginImpl : IStateLightPluginDef
	{
		/// <summary>
		/// Timeout
		/// </summary>
		private const int CONNECT_TIMEOUT = 5000;

		/// <summary>
		/// Client, set in constructor if found
		/// </summary>
		private LyncClient lyncClient = null;

		/// <summary>
		/// Flag to enable event forwarding
		/// </summary>
		private bool sendEvent = false;

		/// <summary>
		/// Callback handler
		/// </summary>
		private IStateProvider callback;

		/// <summary>
		/// Timer to connect lync, if lync is not running on startup
		/// </summary>
		private Timer connectTimer;

		/// <summary>
		/// Constructor
		/// </summary>
		public Lync2010PluginImpl()
		{
			ConnectLyncClient();

			if (lyncClient == null)
			{
				connectTimer = new Timer(OnConnectTimer, null, CONNECT_TIMEOUT, Timeout.Infinite);
			}
		}

		/// <summary>
		/// Try to connect lync
		/// </summary>
		public void OnConnectTimer(Object state)
		{
			Console.WriteLine("Lync connect Timer");

			ConnectLyncClient();

			if (lyncClient == null)
			{
				connectTimer.Change(CONNECT_TIMEOUT, Timeout.Infinite);
			}
			else
			{
				Console.WriteLine("Lync is started up - connected!");

				connectTimer.Dispose();

				// If enabled: Send state
				SendState();
			}
		}

		/// <summary>
		/// Connect to the lync client, if possible
		/// </summary>
		private void ConnectLyncClient()
		{
			Console.WriteLine("Connecting Lync...");

			try
			{
				lyncClient = LyncClient.GetClient();
				lyncClient.StateChanged += CallbackStateChanged;

				if (lyncClient.State == ClientState.SignedIn)
				{
					lyncClient.Self.Contact.ContactInformationChanged += CallbackContactInformationChanged;
				}
			}
			catch (ClientNotFoundException)
			{
				Console.WriteLine("Lync not running");
			}
			catch (Exception e)
			{
				Console.WriteLine("Exception connection lync: " + e.ToString());
			}
		}

		public void Start(IStateProvider state)
		{
			callback = state;
			sendEvent = true;

			SendState();
		}

		public void Stop()
		{
			sendEvent = false;
		}

		/// <summary>
		/// Callback, called by lync
		/// </summary>
		/// <param name="sender">Sender</param>
		/// <param name="e">ContactInformationChangedEventArgs</param>
		void CallbackContactInformationChanged(object sender, ContactInformationChangedEventArgs e)
		{
			if (!sendEvent)
			{
				return;
			}

			if (!e.ChangedContactInformation.Contains(ContactInformationType.Availability))
			{
				return;
			}

			SendState();
		}


		/// <summary>
		/// Send state to callback
		/// </summary>
		private void SendState()
		{
			if (callback == null)
			{
				return;
			}

			if (!sendEvent)
			{
				return;
			}


			if (lyncClient == null)
			{
				callback.WriteState("not-running", "");
				return;
			}

			if (lyncClient.State != ClientState.SignedIn)
			{
				callback.WriteState("signed-out", "");
				return;
			}

			ContactAvailability currentAvailability = (ContactAvailability)lyncClient.Self.Contact.GetContactInformation(ContactInformationType.Availability);

			// Busy, BusyIdle, Free, FreeIdle, Away, TemporarilyAway, DoNotDisturb, Offline
			callback.WriteState(currentAvailability.ToString(), "");
		}

		void CallbackStateChanged(object sender, ClientStateChangedEventArgs e)
		{
			switch (e.NewState)
			{
				case ClientState.Initializing:
					break;

				case ClientState.Invalid:
					break;

				case ClientState.ShuttingDown:
					break;

				case ClientState.SignedIn:
					lyncClient.Self.Contact.ContactInformationChanged += CallbackContactInformationChanged;
					if (sendEvent)
					{
						SendState();
					}
					break;

				case ClientState.SignedOut:
					if (sendEvent)
					{
						callback.WriteState("signed-out", "");
					}
					lyncClient.StateChanged -= CallbackStateChanged;
					break;

				case ClientState.SigningIn:
					break;

				case ClientState.SigningOut:
					break;

				case ClientState.Uninitialized:
					break;

				default:
					break;
			}
		}
	}
}
