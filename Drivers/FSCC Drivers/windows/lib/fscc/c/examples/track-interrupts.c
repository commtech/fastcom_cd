#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    unsigned matches;

    fscc_connect(0, &h);

    /* TIN interrupt */
    fscc_track_interrupts_with_blocking(h, 0x00000100, &matches);

    fscc_disconnect(h);

    return 0;
}
