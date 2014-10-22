using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
	{
        Fscc.Port p = new Fscc.Port(0);

        var modifiers = p.TxModifiers;

        // Enable transmit repeat & transmit on timer
        p.TxModifiers = Fscc.TxModifiers.TXT | Fscc.TxModifiers.XREP;

        // Revert to normal transmission
        p.TxModifiers = Fscc.TxModifiers.XF;

        return 0;
    }
}