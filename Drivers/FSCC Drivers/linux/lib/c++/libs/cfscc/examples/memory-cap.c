#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    struct fscc_memory_cap memcap;

    fscc_connect(0, &h);

    memcap.input = 1000000; /* 1 MB */
    memcap.output = 2000000; /* 2 MB */

    fscc_set_memory_cap(h, &memcap);

    fscc_get_memory_cap(h, &memcap);

    fscc_disconnect(h);

    return 0;
}
