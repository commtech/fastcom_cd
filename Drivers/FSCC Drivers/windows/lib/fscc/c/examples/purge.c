#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;

    fscc_connect(0, &h);

    /* TX */
    fscc_purge(h, 1, 0);

    /* RX */
    fscc_purge(h, 0, 1);

    /* TX & RX */
    fscc_purge(h, 1, 1);

    fscc_disconnect(h);

    return 0;
}
