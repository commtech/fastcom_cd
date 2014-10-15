#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    unsigned modifiers;

    fscc_connect(0, &h);

    fscc_get_tx_modifiers(h, &modifiers);

    /* Enable transmit repeat & transmit on timer */
    fscc_set_tx_modifiers(h, TXT | XREP);

    /* Revert to normal transmission */
    fscc_set_tx_modifiers(h, XF);

    fscc_disconnect(h);

    return 0;
}
