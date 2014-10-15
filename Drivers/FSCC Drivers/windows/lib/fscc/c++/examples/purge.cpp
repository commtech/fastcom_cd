#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    // TX
    p.Purge(true, false);

    // RX
    p.Purge(false, true);

    // TX & RX
    p.Purge(true, true);

    return 0;
}
