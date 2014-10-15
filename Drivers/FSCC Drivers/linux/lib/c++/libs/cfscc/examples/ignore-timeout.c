#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    unsigned status;

    fscc_connect(0, &h);

    fscc_get_ignore_timeout(h, &status);

    fscc_enable_ignore_timeout(h);
    fscc_disable_ignore_timeout(h);

    fscc_disconnect(h);

    return 0;
}
