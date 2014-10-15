#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    unsigned status;

    fscc_connect(0, &h);

    fscc_get_rx_multiple(h, &status);

    fscc_enable_rx_multiple(h);
    fscc_disable_rx_multiple(h);

    fscc_disconnect(h);

    return 0;
}
