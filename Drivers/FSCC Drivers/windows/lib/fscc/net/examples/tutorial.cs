using Fscc;
using System;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        // Send "Hello world!" text
        p.Write("Hello world!");

        // Read the data back in (with our loopback connector)
        Console.WriteLine(p.Read(100));

        return 0;
    }
}