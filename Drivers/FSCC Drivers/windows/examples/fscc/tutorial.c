#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    char odata[] = "Hello world!";
    char idata[20];

    /* Open port 0 in a blocking IO mode */
    h = CreateFile("\\\\.\\FSCC0", GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   OPEN_EXISTING, 0, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile failed with %d\n", GetLastError());
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    WriteFile(h, odata, sizeof(odata), &tmp, NULL);

    /* Read the data back in (with our loopback connector) */
    ReadFile(h, idata, sizeof(idata), &tmp, NULL);

    fprintf(stdout, "%s\n", idata);

    CloseHandle(h);

    return EXIT_SUCCESS;
}
