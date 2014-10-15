using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        var matches = p.TrackInterrupts(0x00000400);

        return 0;
    }
}