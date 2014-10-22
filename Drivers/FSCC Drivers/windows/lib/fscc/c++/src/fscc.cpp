/*
    Copyright (C) 2014 Commtech, Inc.

    This file is part of cppfscc.

    cppfscc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cppfscc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cppfscc.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <cstdio>
#include <sstream>
#include <memory>

#include <fscc.h>

#include "fscc.hpp"

template <class T>
inline std::string to_string(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

namespace Fscc {

/// <param name="port_num">Used to indicate status.</param>
Port::Port(unsigned port_num) throw(SystemException)
{
    init(port_num);
}

Port::~Port(void)
{
    cleanup();
}

void Port::init(unsigned port_num, fscc_handle h) throw()
{
    _port_num = port_num;
    _h = h;
} 

void Port::init(unsigned port_num) throw(SystemException)
{
    _port_num = port_num;

    int e = fscc_connect(port_num, &_h);

    switch (e) {
    case 0:
        break;

    case FSCC_PORT_NOT_FOUND:
        throw PortNotFoundException(port_num);

    case FSCC_INVALID_ACCESS:
        throw InvalidAccessException();

    default:
        throw SystemException(to_string(e));
    }
} 

void Port::cleanup(void) throw()
{
    fscc_disconnect(_h);
} 

Port::Port(const Port &other) throw(SystemException)
{
    init(other._port_num);
}

Port& Port::operator=(const Port &other) throw(SystemException)
{
    if (this != &other) {
        fscc_handle h2;
        int e = fscc_connect(other._port_num, &h2);

        if (e) {
            throw SystemException(to_string(e));
        }
        else {
            cleanup();
            init(other._port_num, h2);
        }
    }

    return *this;
}

int Port::Write(const char *buf, unsigned size, OVERLAPPED *o) throw(SystemException)
{
    unsigned bytes_written = 0;

    int e = fscc_write(_h, (char *)buf, size, &bytes_written, o);

    switch (e) {
    case 0:
    case 997: // ERROR_IO_PENDING
        break;

    case FSCC_BUFFER_TOO_SMALL:
        throw BufferTooSmallException();

    case FSCC_TIMEOUT:
        throw TimeoutException();

    case FSCC_INCORRECT_MODE:
        throw IncorrectModeException();

    default:
        throw SystemException(to_string(e));
    }

    return e;
}

unsigned Port::Write(const char *buf, unsigned size) throw(SystemException)
{
    unsigned bytes_written = 0;

    int e = fscc_write_with_blocking(_h, (char *)buf, size, &bytes_written);

    switch (e) {
    case 0:
        break;

    case FSCC_BUFFER_TOO_SMALL:
        throw BufferTooSmallException();

    case FSCC_TIMEOUT:
        throw TimeoutException();

    case FSCC_INCORRECT_MODE:
        throw IncorrectModeException();

    default:
        throw SystemException(to_string(e));
    }

    return bytes_written;
}

unsigned Port::Write(const std::string &str) throw(SystemException)
{
    return Write(str.c_str(), str.length());
}

int Port::Read(char *buf, unsigned size, OVERLAPPED *o) throw(SystemException)
{
    unsigned bytes_read;

    int e = fscc_read(_h, buf, size, &bytes_read, o);

    switch (e) {
    case 0:
    case 997: // ERROR_IO_PENDING
        break;

    case FSCC_BUFFER_TOO_SMALL:
        throw BufferTooSmallException();

    case FSCC_INCORRECT_MODE:
        throw IncorrectModeException();

    default:
        throw SystemException(to_string(e));
    }

    return e;
}

unsigned Port::Read(char *buf, unsigned size) throw(SystemException)
{
    unsigned bytes_read;

    int e = fscc_read_with_blocking(_h, buf, size, &bytes_read);

    switch (e) {
    case 0:
        break;

    case FSCC_BUFFER_TOO_SMALL:
        throw BufferTooSmallException();

    case FSCC_INCORRECT_MODE:
        throw IncorrectModeException();

    default:
        throw SystemException(to_string(e));
    }

    return bytes_read;
}

unsigned Port::Read(char *buf, unsigned size, unsigned timeout) throw(SystemException)
{
    unsigned bytes_read;

    int e = fscc_read_with_timeout(_h, buf, size, &bytes_read, timeout);

    switch (e) {
    case 0:
        break;

    case FSCC_BUFFER_TOO_SMALL:
        throw BufferTooSmallException();

    case FSCC_INCORRECT_MODE:
        throw IncorrectModeException();

    default:
        throw SystemException(to_string(e));
    }

    return bytes_read;
}

std::string Port::Read(unsigned size) throw(SystemException)
{
    std::unique_ptr<char []> buf(new char[size]);
    std::string str;

    try {
        Read(buf.get(), size);
    }
    catch (SystemException &e) {
        throw e;
    }

    str.assign(buf.get());
    return str;
}

std::string Port::Read(unsigned size, unsigned timeout) throw(SystemException)
{
    std::unique_ptr<char[]> buf(new char[size]);
    std::string str;

    try {
        Read(buf.get(), size, timeout);
    }
    catch (SystemException &e) {
        throw e;
    }

    str.assign(buf.get());
    return str;
}

unsigned Port::GetTxModifiers(void) throw()
{
    unsigned modifiers;

    fscc_get_tx_modifiers(_h, &modifiers);

    return modifiers;
}

void Port::SetTxModifiers(unsigned modifiers) throw()
{
    fscc_set_tx_modifiers(_h, modifiers);
}

void Port::SetMemoryCap(const MemoryCap &memcap) throw()
{
    struct fscc_memory_cap m;

    memcpy(&m, &memcap, sizeof(m));

    fscc_set_memory_cap(_h, &m);
}

MemoryCap Port::GetMemoryCap(void) throw()
{
    struct MemoryCap memcap;

    fscc_get_memory_cap(_h, (struct fscc_memory_cap *)&memcap);

    return memcap;
}

void Port::SetRegisters(const Registers &regs) throw(SystemException)
{
    struct fscc_registers r;

    memcpy(&r, &regs, sizeof(r));

    int e = fscc_set_registers(_h, &r);

    switch (e) {
    case 0:
        break;

    case FSCC_TIMEOUT:
        throw TimeoutException();

    default:
        throw SystemException(to_string(e));
    }
}

Registers Port::GetRegisters(const Registers &regs) throw()
{
    fscc_get_registers(_h, (struct fscc_registers *)&regs);

    return regs;
}

void Port::SetClockFrequency(unsigned frequency) throw(SystemException)
{
    int e = fscc_set_clock_frequency(_h, frequency);

    switch (e) {
    case 0:
        break;

    case FSCC_INVALID_PARAMETER:
        throw InvalidParameterException();

    default:
        throw SystemException(to_string(e));
    }
}

bool Port::GetAppendStatus(void) throw()
{
    unsigned status;

    fscc_get_append_status(_h, &status);

    return status != 0;
}

void Port::EnableAppendStatus(void) throw()
{
    fscc_enable_append_status(_h);
}

void Port::DisableAppendStatus(void) throw()
{
    fscc_disable_append_status(_h);
}

bool Port::GetAppendTimestamp(void) throw()
{
    unsigned status;

    fscc_get_append_timestamp(_h, &status);

    return status != 0;
}

void Port::EnableAppendTimestamp(void) throw()
{
    fscc_enable_append_timestamp(_h);
}

void Port::DisableAppendTimestamp(void) throw()
{
    fscc_disable_append_timestamp(_h);
}

bool Port::GetIgnoreTimeout(void) throw()
{
    unsigned status;

    fscc_get_ignore_timeout(_h, &status);

    return status != 0;
}

void Port::EnableIgnoreTimeout(void) throw()
{
    fscc_enable_ignore_timeout(_h);
}

void Port::DisableIgnoreTimeout(void) throw()
{
    fscc_disable_ignore_timeout(_h);
}

bool Port::GetRxMultiple(void) throw()
{
    unsigned status;

    fscc_get_rx_multiple(_h, &status);

    return status != 0;
}

void Port::EnableRxMultiple(void) throw()
{
    fscc_enable_rx_multiple(_h);
}

void Port::DisableRxMultiple(void) throw()
{
    fscc_disable_rx_multiple(_h);
}

void Port::Purge(bool tx, bool rx) throw(SystemException)
{
    int e = fscc_purge(_h, tx, rx);

    switch (e) {
    case 0:
        break;

    case FSCC_TIMEOUT:
        throw TimeoutException();

    default:
        throw SystemException(to_string(e));
    }
}

int Port::TrackInterrupts(unsigned interrupts, unsigned *matches, OVERLAPPED *o) throw(SystemException)
{
    int e = fscc_track_interrupts(_h, interrupts, matches, o);

    switch (e) {
    case 0:
    case 997: // ERROR_IO_PENDING
        break;

    default:
        throw SystemException(to_string(e));
    }
    
    return e;
}

unsigned Port::TrackInterrupts(unsigned interrupts) throw(SystemException)
{
    unsigned matches = 0;

    int e = fscc_track_interrupts_with_blocking(_h, interrupts, &matches);

    if (e)
        throw SystemException(to_string(e));

    return matches;
}

unsigned Port::TrackInterrupts(unsigned interrupts, unsigned timeout) throw(SystemException)
{
    unsigned matches = 0;

    int e = fscc_track_interrupts_with_timeout(_h, interrupts, &matches, timeout);

    if (e)
        throw SystemException(to_string(e));

    return matches;
}

Registers::Registers()
{
    Reset();
}

void Registers::Reset(void)
{
    FIFOT = -1;

    CMDR = -1;
    STAR = -1; /* Read-only */
    CCR0 = -1;
    CCR1 = -1;
    CCR2 = -1;
    BGR = -1;
    SSR = -1;
    SMR = -1;
    TSR = -1;
    TMR = -1;
    RAR = -1;
    RAMR = -1;
    PPR = -1;
    TCR = -1;
    VSTR = -1;

    IMR = -1;
    DPLLR = -1;

    /* BAR 2 */
    FCR = -1;
}

MemoryCap::MemoryCap()
{
    Reset();
}

void MemoryCap::Reset(void)
{
    input = -1;
    output = -1;
}

} /* namespace FSCC */
