#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    bool status = p.GetRxMultiple();
    
    p.EnableRxMultiple();
    p.DisableRxMultiple();

    return 0;
}
