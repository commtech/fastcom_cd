using Fscc;

public class Tutorial
{
    public static int Main(string[] args)
    {
        Fscc.Port p = new Fscc.Port(0);
    
        p.MemoryCap.Input = 1000000; // 1 MB
        p.MemoryCap.Output = 1000000; // 1 MB
        
        var input = p.MemoryCap.Input;
        var output = p.MemoryCap.Output;

        return 0;
    }
}