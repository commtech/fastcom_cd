#include <stdio.h> /* fprintf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    int e = 0;
    char odata[] = "Hello world!";
    char idata[20] = {0};
    unsigned tmp;

    /* Open port 0 in a blocking IO mode */
    e = fscc_connect(0, &h);
    if (e != 0) {
        fprintf(stderr, "fscc_connect failed with %d\n", e);
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    fscc_write_with_blocking(h, odata, sizeof(odata), &tmp);

    /* Read the data back in (with our loopback connector) */
    fscc_read_with_blocking(h, idata, sizeof(idata), &tmp);

    fprintf(stdout, "%s\n", idata);

    fscc_disconnect(h);

    return EXIT_SUCCESS;
}
