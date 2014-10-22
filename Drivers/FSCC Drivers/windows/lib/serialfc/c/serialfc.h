/*
    Copyright (C) 2013 Commtech, Inc.

    This file is part of cserialfc.

    cserialfc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cserialfc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cserialfc.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SERIALFC_H
#define SERIALFC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#endif

enum error_type { SERIALFC_NOT_SUPPORTED=17000, SERIALFC_INVALID_PARAMETER, SERIALFC_INVALID_ACCESS, SERIALFC_PORT_NOT_FOUND };

#ifdef _WIN32
    #define SERIALFC_IOCTL_MAGIC 0x8019

    #define IOCTL_FASTCOM_ENABLE_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_SET_SAMPLE_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_SAMPLE_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_SET_TX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_TX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_SET_RX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80D, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_RX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80E, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_SET_CLOCK_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80F, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_SET_FRAME_LENGTH CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x816, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_FRAME_LENGTH CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x817, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_GET_CARD_TYPE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x818, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_9BIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x819, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_9BIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x81A, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_9BIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x81B, METHOD_BUFFERED, FILE_ANY_ACCESS)

    #define IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x81C, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x81D, METHOD_BUFFERED, FILE_ANY_ACCESS)
    #define IOCTL_FASTCOM_GET_FIXED_BAUD_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x81E, METHOD_BUFFERED, FILE_ANY_ACCESS)
#else
    #define SERIALFC_IOCTL_MAGIC 0x19

    #define IOCTL_FASTCOM_ENABLE_RS485 _IO(SERIALFC_IOCTL_MAGIC, 0)
    #define IOCTL_FASTCOM_DISABLE_RS485 _IO(SERIALFC_IOCTL_MAGIC, 1)
    #define IOCTL_FASTCOM_GET_RS485 _IOR(SERIALFC_IOCTL_MAGIC, 2, unsigned *)

    #define IOCTL_FASTCOM_ENABLE_ECHO_CANCEL _IO(SERIALFC_IOCTL_MAGIC, 3)
    #define IOCTL_FASTCOM_DISABLE_ECHO_CANCEL _IO(SERIALFC_IOCTL_MAGIC, 4)
    #define IOCTL_FASTCOM_GET_ECHO_CANCEL _IOR(SERIALFC_IOCTL_MAGIC, 5, unsigned *)

    #define IOCTL_FASTCOM_ENABLE_TERMINATION _IO(SERIALFC_IOCTL_MAGIC, 6)
    #define IOCTL_FASTCOM_DISABLE_TERMINATION _IO(SERIALFC_IOCTL_MAGIC, 7)
    #define IOCTL_FASTCOM_GET_TERMINATION _IOR(SERIALFC_IOCTL_MAGIC, 8, unsigned *)

    #define IOCTL_FASTCOM_SET_SAMPLE_RATE _IOW(SERIALFC_IOCTL_MAGIC, 9, const unsigned)
    #define IOCTL_FASTCOM_GET_SAMPLE_RATE _IOR(SERIALFC_IOCTL_MAGIC, 10, unsigned *)

    #define IOCTL_FASTCOM_SET_TX_TRIGGER _IOW(SERIALFC_IOCTL_MAGIC, 11, const unsigned)
    #define IOCTL_FASTCOM_GET_TX_TRIGGER _IOR(SERIALFC_IOCTL_MAGIC, 12, unsigned *)

    #define IOCTL_FASTCOM_SET_RX_TRIGGER _IOW(SERIALFC_IOCTL_MAGIC, 13, const unsigned)
    #define IOCTL_FASTCOM_GET_RX_TRIGGER _IOR(SERIALFC_IOCTL_MAGIC, 14, unsigned *)

    #define IOCTL_FASTCOM_SET_CLOCK_RATE _IOW(SERIALFC_IOCTL_MAGIC, 15, const unsigned)

    #define IOCTL_FASTCOM_ENABLE_ISOCHRONOUS _IOW(SERIALFC_IOCTL_MAGIC, 16, const unsigned)
    #define IOCTL_FASTCOM_DISABLE_ISOCHRONOUS _IO(SERIALFC_IOCTL_MAGIC, 17)
    #define IOCTL_FASTCOM_GET_ISOCHRONOUS _IOR(SERIALFC_IOCTL_MAGIC, 18, unsigned *)

    #define IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT _IOW(SERIALFC_IOCTL_MAGIC, 19, const unsigned)
    #define IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT _IO(SERIALFC_IOCTL_MAGIC, 20)
    #define IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT _IOR(SERIALFC_IOCTL_MAGIC, 21, unsigned *)

    #define IOCTL_FASTCOM_SET_FRAME_LENGTH _IOW(SERIALFC_IOCTL_MAGIC, 22, const unsigned)
    #define IOCTL_FASTCOM_GET_FRAME_LENGTH _IOR(SERIALFC_IOCTL_MAGIC, 23, int *)

    #define IOCTL_FASTCOM_GET_CARD_TYPE _IOR(SERIALFC_IOCTL_MAGIC, 24, unsigned *)

    #define IOCTL_FASTCOM_ENABLE_9BIT _IO(SERIALFC_IOCTL_MAGIC, 25)
    #define IOCTL_FASTCOM_DISABLE_9BIT _IO(SERIALFC_IOCTL_MAGIC, 26)
    #define IOCTL_FASTCOM_GET_9BIT _IOR(SERIALFC_IOCTL_MAGIC, 27, unsigned *)

    #define IOCTL_FASTCOM_SET_CLOCK_BITS _IOW(SERIALFC_IOCTL_MAGIC, 28, const unsigned char[20])

    #define IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE _IOW(SERIALFC_IOCTL_MAGIC, 29, const unsigned)
    #define IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE _IO(SERIALFC_IOCTL_MAGIC, 30)
    #define IOCTL_FASTCOM_GET_FIXED_BAUD_RATE _IOR(SERIALFC_IOCTL_MAGIC, 31, unsigned *)
#endif


#ifndef _WIN32
typedef void OVERLAPPED;
#endif

#ifdef _WIN32
typedef HANDLE serialfc_handle;
#else
typedef int serialfc_handle;
#endif

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

DLL_EXPORT int serialfc_connect(unsigned port_num, serialfc_handle *h);
DLL_EXPORT int serialfc_enable_rs485(serialfc_handle h);
DLL_EXPORT int serialfc_disable_rs485(serialfc_handle h);
DLL_EXPORT int serialfc_get_rs485(serialfc_handle h, unsigned *status);
DLL_EXPORT int serialfc_enable_echo_cancel(serialfc_handle h);
DLL_EXPORT int serialfc_disable_echo_cancel(serialfc_handle h);
DLL_EXPORT int serialfc_get_echo_cancel(serialfc_handle h, unsigned *status);
DLL_EXPORT int serialfc_enable_termination(serialfc_handle h);
DLL_EXPORT int serialfc_disable_termination(serialfc_handle h);
DLL_EXPORT int serialfc_get_termination(serialfc_handle h, unsigned *status);
DLL_EXPORT int serialfc_set_sample_rate(serialfc_handle h, unsigned rate);
DLL_EXPORT int serialfc_get_sample_rate(serialfc_handle h, unsigned *rate);
DLL_EXPORT int serialfc_set_tx_trigger(serialfc_handle h, unsigned level);
DLL_EXPORT int serialfc_get_tx_trigger(serialfc_handle h, unsigned *level);
DLL_EXPORT int serialfc_set_rx_trigger(serialfc_handle h, unsigned level);
DLL_EXPORT int serialfc_get_rx_trigger(serialfc_handle h, unsigned *level);
DLL_EXPORT int serialfc_set_clock_rate(serialfc_handle h, unsigned rate);
DLL_EXPORT int serialfc_enable_isochronous(serialfc_handle h, unsigned mode);
DLL_EXPORT int serialfc_disable_isochronous(serialfc_handle h);
DLL_EXPORT int serialfc_get_isochronous(serialfc_handle h, int *mode);
DLL_EXPORT int serialfc_enable_external_transmit(serialfc_handle h, unsigned num_frames);
DLL_EXPORT int serialfc_disable_external_transmit(serialfc_handle h);
DLL_EXPORT int serialfc_get_external_transmit(serialfc_handle h, unsigned *num_frames);
DLL_EXPORT int serialfc_set_frame_length(serialfc_handle h, unsigned num_chars);
DLL_EXPORT int serialfc_get_frame_length(serialfc_handle h, unsigned *num_chars);
DLL_EXPORT int serialfc_enable_9bit(serialfc_handle h);
DLL_EXPORT int serialfc_disable_9bit(serialfc_handle h);
DLL_EXPORT int serialfc_get_9bit(serialfc_handle h, unsigned *status);
DLL_EXPORT int serialfc_enable_fixed_baud_rate(serialfc_handle h, unsigned rate);
DLL_EXPORT int serialfc_disable_fixed_baud_rate(serialfc_handle h);
DLL_EXPORT int serialfc_get_fixed_baud_rate(serialfc_handle h, int *rate);
DLL_EXPORT int serialfc_get_card_type(serialfc_handle h, unsigned *type);
DLL_EXPORT int serialfc_write(serialfc_handle h, char *buf, unsigned size, unsigned *bytes_written, OVERLAPPED *o);
DLL_EXPORT int serialfc_write_with_blocking(serialfc_handle h, char *buf, unsigned size, unsigned *bytes_written);
DLL_EXPORT int serialfc_read(serialfc_handle h, char *buf, unsigned size, unsigned *bytes_read, OVERLAPPED *o);
DLL_EXPORT int serialfc_read_with_blocking(serialfc_handle h, char *buf, unsigned size, unsigned *bytes_read);
DLL_EXPORT int serialfc_disconnect(serialfc_handle h);

#ifdef __cplusplus
}
#endif

#endif
