#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    bool status = p.GetAppendStatus();
    
    p.EnableAppendStatus();
    p.DisableAppendStatus();

    return 0;
}
