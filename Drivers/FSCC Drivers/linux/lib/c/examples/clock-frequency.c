#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;

    fscc_connect(0, &h);

    /* 18.432 Mhz */
    fscc_set_clock_frequency(h, 18432000);

    fscc_disconnect(h);

    return 0;
}
