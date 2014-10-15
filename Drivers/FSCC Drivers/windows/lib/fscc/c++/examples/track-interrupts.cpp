#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    /* TIN interrupt */
    unsigned matches = p.TrackInterrupts(0x00000400);

    return 0;
}
