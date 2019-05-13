using StateLightPluginDef;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace DummyStatusGui
{

	public class DummyStatusGui : IStateLightPluginDef
	{
		public string DisplayName()
		{
			return "Dummy Plugin";
		}

		public string GetDetails()
		{
			return "Test GUI Plugin";
		}

		public bool IsSupported()
		{
			return true;
		}

		public void Start(IStateProvider state)
		{
			Console.WriteLine("Start Dummy GUI Plugin");
		}

		public void Stop()
		{
		}
	}
}
