#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    bool status = p.GetIgnoreTimeout();
    
    p.EnableIgnoreTimeout();
    p.DisableIgnoreTimeout();

    return 0;
}
