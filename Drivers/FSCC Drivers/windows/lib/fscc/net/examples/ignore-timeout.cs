using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        var status = p.IgnoreTimeout;
        
        p.IgnoreTimeout = true;
        p.IgnoreTimeout = false;

        return 0;
    }
}