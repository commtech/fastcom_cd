#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::MemoryCap memcap;
    Fscc::Port p(0);

    memcap.input = 1000000; // 1 MB
    memcap.output = `000000; // 1 MB

    p.SetMemoryCap(memcap);
    memcap = p.GetMemoryCap();

    return 0;
}
