using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);

        var status = p.RxMultiple;
        
        p.RxMultiple = true;
        p.RxMultiple = false;

        return 0;
    }
}