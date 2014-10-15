#include <fscc.h> /* fscc_* */

int main(void)
{
	fscc_handle h;
    unsigned status;

	fscc_connect(0, &h);

    fscc_get_append_status(h, &status);

	fscc_enable_append_status(h);
	fscc_disable_append_status(h);

	fscc_disconnect(h);

	return 0;
}
