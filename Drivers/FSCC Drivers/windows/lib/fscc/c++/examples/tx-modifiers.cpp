#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    unsigned modifiers = p.GetTxModifiers();

    // Enable transmit repeat & transmit on timer
    p.SetTxModifiers(Fscc::TxModifiers::TXT | Fscc::TxModifiers::XREP);

    // Revert to normal transmission 
    p.SetTxModifiers(Fscc::TxModifiers::XF);

    return 0;
}