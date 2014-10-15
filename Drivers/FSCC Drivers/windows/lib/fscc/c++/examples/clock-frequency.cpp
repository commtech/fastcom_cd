#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);
    
    // 18.432 MHz
    p.SetClockFrequency(18432000);

    return 0;
}
