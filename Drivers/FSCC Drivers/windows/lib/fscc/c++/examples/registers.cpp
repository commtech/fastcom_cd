#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Registers regs;
    Fscc::Port p(0);

    regs.CCR0 = 0x0011201c;
    regs.BGR = 10;

    p.SetRegisters(regs);
    
    // Re-initialize our registers structure
    regs.Reset();

    // Mark the CCR1 and CCR2 elements to retrieve values
    regs.CCR1 = Fscc::UPDATE_VALUE;
    regs.CCR2 = Fscc::UPDATE_VALUE;

    regs = p.GetRegisters(regs);

    return 0;
}
