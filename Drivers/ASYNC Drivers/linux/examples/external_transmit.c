#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <serialfc.h> /* IOCTL_FASTCOM_* */

int main(void)
{
    int fd = 0;
    unsigned status = 0;

    fd = open("/dev/serialfc0", O_RDWR);

    ioctl(fd, IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT, &status);

    ioctl(fd, IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT);

    /* External transmit every 1 character */
    ioctl(fd, IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT, 1);

    close(fd);

    return 0;
}
