using Fscc;
using System;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        var status = p.AppendTimestamp;
        
        p.AppendTimestamp = true;
        p.AppendTimestamp = false;

        return 0;
    }
}