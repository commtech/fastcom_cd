using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        // Purge TX
        p.Purge(true, false);

        // Purge RX
        p.Purge(false, true);

        // Purge both TX & RX
        p.Purge(true, true);

        return 0;
    }
}