using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        var status = p.AppendStatus;
        
        p.AppendStatus = true;
        p.AppendStatus = false;

        return 0;
    }
}