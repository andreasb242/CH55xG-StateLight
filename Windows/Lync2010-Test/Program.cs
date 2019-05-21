using Lync2010Plugin;
using Microsoft.Lync.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Lync2010_Test
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Test started...");

            LyncClient.GetClient();

            Lync2010PluginImpl p = new Lync2010PluginImpl();

            Console.WriteLine("Supported: " + p.IsSupported());
        }
    }
}
