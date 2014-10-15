/*
    Copyright (C) 2014 Commtech, Inc.

    This file is part of cfscc.

    cfscc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cfscc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cfscc.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <fcntl.h> /* open, O_RDWR */

#ifndef _WIN32
#include <unistd.h> /* write, close */
#include <poll.h>
#endif

#include "fscc.h"
#include "errno.h"
#include "calculate-clock-bits.h"

#define UNUSED(x) (void)(x)

#define MAX_NAME_LENGTH 25

int translate_error(int e)
{
#ifdef _WIN32
    switch (e) {
        case ERROR_ACCESS_DENIED:
            return FSCC_INVALID_ACCESS;
        case ERROR_FILE_NOT_FOUND:
            return FSCC_PORT_NOT_FOUND;
        case ERROR_SEM_TIMEOUT:
            return FSCC_TIMEOUT;
        case ERROR_INSUFFICIENT_BUFFER:
            return FSCC_BUFFER_TOO_SMALL;
    default:
        return e;
    }
#else
    switch (e) {
        case ENOENT:
            return FSCC_PORT_NOT_FOUND;
        case EACCES:
            return FSCC_INVALID_ACCESS;
        case EOPNOTSUPP:
            return FSCC_INCORRECT_MODE;
        case ETIMEDOUT:
            return FSCC_TIMEOUT;
        case ENOBUFS:
            return FSCC_BUFFER_TOO_SMALL;
    default:
        return e;
    }
#endif
}

int ioctl_action(fscc_handle h, int ioctl_name)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             NULL, 0,
                             NULL, 0,
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    result = ioctl(h, ioctl_name);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int ioctl_get_boolean(fscc_handle h, int ioctl_name, unsigned *status)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             NULL, 0,
                             status, sizeof(*status),
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    result = ioctl(h, ioctl_name, status);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int ioctl_set_integer(fscc_handle h, int ioctl_name, int value)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             &value, sizeof(value),
                             NULL, 0,
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    result = ioctl(h, ioctl_name, value);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int ioctl_get_integer(fscc_handle h, int ioctl_name, int *value)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             NULL, 0,
                             value, sizeof(*value),
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    result = ioctl(h, ioctl_name, value);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int ioctl_set_pointer(fscc_handle h, int ioctl_name, const void *value,
                      int size)
{
    int result;

    #ifdef _WIN32
        DWORD temp;

        result = DeviceIoControl(h, (DWORD)ioctl_name,
                                 (void *)value, size,
                                 NULL, 0,
                                 &temp, (LPOVERLAPPED)NULL);

        return (result == TRUE) ? 0 : translate_error(GetLastError());
    #else
        UNUSED(size);

        result = ioctl(h, ioctl_name, value);

        return (result != -1) ? 0 : translate_error(errno);
    #endif
}

int ioctl_get_pointer(fscc_handle h, int ioctl_name, void *value,
                      int size)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             NULL, 0,
                             value, size,
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(size);

    result = ioctl(h, ioctl_name, value);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int ioctl_getset_pointer(fscc_handle h, int ioctl_name, void *value,
                         int size)
{
    int result;

#ifdef _WIN32
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)ioctl_name,
                             value, size,
                             value, size,
                             &temp, (LPOVERLAPPED)NULL);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(size);

    result = ioctl(h, ioctl_name, value);

    return (result != -1) ? 0 : translate_error(errno);
#endif
}

int fscc_connect(unsigned port_num, fscc_handle *h)
{
    char name[MAX_NAME_LENGTH];

#ifdef _WIN32
    sprintf_s(name, MAX_NAME_LENGTH, "\\\\.\\FSCC%u", port_num);

    *h = CreateFile(name,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL
    );

    return (*h != INVALID_HANDLE_VALUE) ? 0 : translate_error(GetLastError());
#else
    sprintf(name, "/dev/fscc%u", port_num);

    *h = open(name, O_RDWR);

    return (*h != -1) ? 0 : translate_error(errno);
#endif
}

int fscc_set_tx_modifiers(fscc_handle h, unsigned modifiers)
{
    return ioctl_set_integer(h, FSCC_SET_TX_MODIFIERS, modifiers);
}

int fscc_get_tx_modifiers(fscc_handle h, unsigned *modifiers)
{
    return ioctl_get_integer(h, FSCC_GET_TX_MODIFIERS, (int *)modifiers);
}

int fscc_set_memory_cap(fscc_handle h, const struct fscc_memory_cap *memcap)
{
    return ioctl_set_pointer(h, FSCC_SET_MEMORY_CAP, memcap, sizeof(*memcap));
}

int fscc_get_memory_cap(fscc_handle h, struct fscc_memory_cap *memcap)
{
    return ioctl_get_pointer(h, FSCC_GET_MEMORY_CAP, memcap, sizeof(*memcap));
}

int fscc_set_registers(fscc_handle h, const struct fscc_registers *regs)
{
    return ioctl_set_pointer(h, FSCC_SET_REGISTERS, regs, sizeof(*regs));
}

int fscc_get_registers(fscc_handle h, struct fscc_registers *regs)
{
    return ioctl_getset_pointer(h, FSCC_GET_REGISTERS, regs, sizeof(*regs));
}

int fscc_get_append_status(fscc_handle h, unsigned *status)
{
    return ioctl_get_boolean(h, FSCC_GET_APPEND_STATUS, status);
}

int fscc_enable_append_status(fscc_handle h)
{
    return ioctl_action(h, FSCC_ENABLE_APPEND_STATUS);
}

int fscc_disable_append_status(fscc_handle h)
{
    return ioctl_action(h, FSCC_DISABLE_APPEND_STATUS);
}

int fscc_get_append_timestamp(fscc_handle h, unsigned *status)
{
    return ioctl_get_boolean(h, FSCC_GET_APPEND_TIMESTAMP, status);
}

int fscc_enable_append_timestamp(fscc_handle h)
{
    return ioctl_action(h, FSCC_ENABLE_APPEND_TIMESTAMP);
}

int fscc_disable_append_timestamp(fscc_handle h)
{
    return ioctl_action(h, FSCC_DISABLE_APPEND_TIMESTAMP);
}

int fscc_get_ignore_timeout(fscc_handle h, unsigned *status)
{
    return ioctl_get_boolean(h, FSCC_GET_IGNORE_TIMEOUT, status);
}

int fscc_enable_ignore_timeout(fscc_handle h)
{
    return ioctl_action(h, FSCC_ENABLE_IGNORE_TIMEOUT);
}

int fscc_disable_ignore_timeout(fscc_handle h)
{
    return ioctl_action(h, FSCC_DISABLE_IGNORE_TIMEOUT);
}

int fscc_get_rx_multiple(fscc_handle h, unsigned *status)
{
    return ioctl_get_boolean(h, FSCC_GET_RX_MULTIPLE, status);
}

int fscc_enable_rx_multiple(fscc_handle h)
{
    return ioctl_action(h, FSCC_ENABLE_RX_MULTIPLE);
}

int fscc_disable_rx_multiple(fscc_handle h)
{
    return ioctl_action(h, FSCC_DISABLE_RX_MULTIPLE);
}

int fscc_track_interrupts(fscc_handle h, unsigned interrupts, unsigned *matches, OVERLAPPED *o)
{
#ifdef _WIN32
    int result;
    DWORD temp;

    result = DeviceIoControl(h, (DWORD)FSCC_TRACK_INTERRUPTS,
                             &interrupts, sizeof(interrupts),
                             matches, sizeof(*matches),
                             &temp, o);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(h);
    UNUSED(interrupts);
    UNUSED(matches);
    UNUSED(o);

    return 0;
#endif
}

int fscc_track_interrupts_with_blocking(fscc_handle h, unsigned interrupts, unsigned *matches)
{
#ifdef _WIN32
    int result;
    OVERLAPPED ol;
    unsigned bytes_transferred = 0;

    memset(&ol, 0, sizeof(ol));

    result = fscc_track_interrupts(h, interrupts, matches, &ol);

    if (result == 997)
        result = GetOverlappedResult(h, &ol, (DWORD *)&bytes_transferred, 1);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    return fscc_track_interrupts(h, interrupts, matches, NULL);
#endif
}

int fscc_track_interrupts_with_timeout(fscc_handle h, unsigned interrupts, unsigned *matches, unsigned timeout)
{
#ifdef _WIN32
    OVERLAPPED o;
    BOOL result;
    unsigned bytes_transferred = 0;
    DWORD temp;

    memset(&o, 0, sizeof(o));

    o.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (o.hEvent == NULL)
        return GetLastError();

    result = DeviceIoControl(h, (DWORD)FSCC_TRACK_INTERRUPTS,
                             &interrupts, sizeof(interrupts),
                             matches, sizeof(*matches),
                             &temp, &o);

    if (result == FALSE) {
        DWORD status;
        int e;

        /* There was an actual error instead of a pending read */
        if ((e = GetLastError()) != ERROR_IO_PENDING) {
            CloseHandle(o.hEvent);
            return e;
        }

        do {
            status = WaitForSingleObject(o.hEvent, timeout);

            switch (status) {
            case WAIT_TIMEOUT:
                bytes_transferred = 0;
                /* Switch to CancelIoEx if using Vista or higher and prefer the
                   way CancelIoEx operates. */
                /* CancelIoEx(h, &o); */
                CancelIo(h);
                CloseHandle(o.hEvent);
                return ERROR_SUCCESS;

            case WAIT_FAILED:
                e = GetLastError();
                CloseHandle(o.hEvent);
                return e;
            }
        }
        while (status != WAIT_OBJECT_0);

        result = GetOverlappedResult(h, &o, (DWORD *)&bytes_transferred, TRUE);
    }

    CloseHandle(o.hEvent);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(timeout);

    return fscc_track_interrupts(h, interrupts, matches, NULL);
#endif
}

int fscc_purge(fscc_handle h, unsigned tx, unsigned rx)
{
    int error;

    if (tx) {
        error = ioctl_action(h, FSCC_PURGE_TX);

        if (error)
            return error;
    }

    if (rx) {
        error = ioctl_action(h, FSCC_PURGE_RX);

        if (error)
            return error;
    }

    return 0;
}

int fscc_set_clock_frequency(fscc_handle h, unsigned frequency)
{
    unsigned char clock_bits[20];
    int result = 0;

    result = calculate_clock_bits(frequency, 10, clock_bits);

    if (result != 0)
        return FSCC_INVALID_PARAMETER;

    return ioctl_set_pointer(h, FSCC_SET_CLOCK_BITS, clock_bits, sizeof(clock_bits));
}

int fscc_write(fscc_handle h, char *buf, unsigned size,
               unsigned *bytes_written, OVERLAPPED *o)
{
    int result;

#ifdef _WIN32
    result = WriteFile(h, buf, size, (DWORD*)bytes_written, o);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(o);

    result = write(h, buf, size);

    if (result == -1)
        return translate_error(errno);

    *bytes_written = result;

    return 0;
#endif
}

int fscc_write_with_blocking(fscc_handle h, char *buf, unsigned size,
                             unsigned *bytes_written)
{
#ifdef _WIN32
    int result;
    OVERLAPPED ol;

    memset(&ol, 0, sizeof(ol));

    result = fscc_write(h, buf, size, bytes_written, &ol);

    if (result == 997)
        result = GetOverlappedResult(h, &ol, (DWORD *)bytes_written, 1);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    return fscc_write(h, buf, size, bytes_written, 0);
#endif
}

int fscc_read(fscc_handle h, char *buf, unsigned size, unsigned *bytes_read,
              OVERLAPPED *o)
{
    int result;
#ifdef _WIN32

    result = ReadFile(h, buf, size, (DWORD*)bytes_read, o);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    UNUSED(o);

    result = read(h, buf, size);

    if (result == -1)
        return translate_error(errno);

    *bytes_read = result;

    return 0;
#endif
}

int fscc_read_with_blocking(fscc_handle h, char *buf, unsigned size,
                            unsigned *bytes_read)
{
#ifdef _WIN32
    int result;
    OVERLAPPED ol;

    memset(&ol, 0, sizeof(ol));

    result = fscc_read(h, buf, size, bytes_read, &ol);

    if (result == 997)
        result = GetOverlappedResult(h, &ol, (DWORD *)bytes_read, 1);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    return fscc_read(h, buf, size, bytes_read, 0);
#endif
}


/******************************************************************************/
/*!

  \note
    Due to supporting Windows XP we have to use CancelIo() instead of
    CancelIoEx(). As a biproduct if there is a WAIT_TIMEOUT both pending
    transmit and receive IO will be cancelled instead of just receiving. If you
    are using Vista or newer you can change this to use CancelIoEx and you will
    only cancel the receiving IO.

*/
/******************************************************************************/
int fscc_read_with_timeout(fscc_handle h, char *buf, unsigned size,
                           unsigned *bytes_read, unsigned timeout)
{
#ifdef _WIN32
    OVERLAPPED o;
    BOOL result;

    memset(&o, 0, sizeof(o));

    o.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (o.hEvent == NULL)
        return GetLastError();

    result = ReadFile(h, buf, size, (DWORD*)bytes_read, &o);

    if (result == FALSE) {
        DWORD status;
        int e;

        /* There was an actual error instead of a pending read */
        if ((e = GetLastError()) != ERROR_IO_PENDING) {
            CloseHandle(o.hEvent);
            return e;
        }

        do {
            status = WaitForSingleObject(o.hEvent, timeout);

            switch (status) {
            case WAIT_TIMEOUT:
                *bytes_read = 0;
                /* Switch to CancelIoEx if using Vista or higher and prefer the
                   way CancelIoEx operates. */
                /* CancelIoEx(h, &o); */
                CancelIo(h);
                CloseHandle(o.hEvent);
                return ERROR_SUCCESS;

            case WAIT_FAILED:
                e = GetLastError();
                CloseHandle(o.hEvent);
                return e;
            }
        }
        while (status != WAIT_OBJECT_0);

        result = GetOverlappedResult(h, &o, (DWORD *)bytes_read, TRUE);
    }

    CloseHandle(o.hEvent);

    return (result == TRUE) ? 0 : translate_error(GetLastError());
#else
    struct pollfd fds[1];
    int e = 0;

    fds[0].fd = h;
    fds[0].events = POLLIN;

    switch (poll(fds, 1, timeout)) {
    case -1:
        e = errno;
        break;

    case 0:
        break;

    default:
        e = fscc_read_with_blocking(h, buf, size, bytes_read);
        break;
    }

    return e;
#endif
}

int fscc_disconnect(fscc_handle h)
{
    int result;

#ifdef _WIN32
    result = CloseHandle(h);

    return (result == TRUE) ? 0 : GetLastError();
#else

    result = close(h);

    return (result != -1) ? 0 : errno;
#endif
}
