using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        p.Registers.CCR0 = 0x0011201c;
        p.Registers.BGR = 0;
        
        var ccr1 = p.Registers.CCR1;
        var ccr2 = p.Registers.CCR2;

        return 0;
    }
}