using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);
        
        // 18.432 MHz
        p.ClockFrequency = 18432000;

        return 0;
    }
}