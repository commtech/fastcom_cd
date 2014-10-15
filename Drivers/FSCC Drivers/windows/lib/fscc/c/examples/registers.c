#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    struct fscc_registers regs;

    fscc_connect(0, &h);

    /* Initialize our registers structure */
    FSCC_REGISTERS_INIT(regs);

    /* Change the CCR0 and BGR elements to our desired values */
    regs.CCR0 = 0x0011201c;
    regs.BGR = 10;

    /* Set the CCR0 and BGR register values */
    fscc_set_registers(h, &regs);

    /* Re-initialize our registers structure */
    FSCC_REGISTERS_INIT(regs);

    /* Mark the CCR0 and CCR1 elements to retrieve values */
    regs.CCR1 = FSCC_UPDATE_VALUE;
    regs.CCR2 = FSCC_UPDATE_VALUE;

    /* Get the CCR1 and CCR2 register values */
    fscc_get_registers(h, &regs);

    fscc_disconnect(h);

    return 0;
}
