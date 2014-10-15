"""
    Copyright (C) 2014 Commtech, Inc.

    This file is part of pyfscc.

    pyfscc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pyfscc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pyfscc.  If not, see <http://www.gnu.org/licenses/>.

"""

import struct
import os
import ctypes
import sys

if os.name == 'nt':
    DLL_NAME = 'cfscc.dll'
else:
    if ctypes.sizeof(ctypes.c_voidp) == 4:
        DLL_NAME = 'libcfscc.so'
    else:
        DLL_NAME = 'libcfscc.so.6'

try:
    lib = ctypes.cdll.LoadLibrary(DLL_NAME)
except:
    if os.name == 'nt':
        lib = ctypes.cdll.LoadLibrary(os.path.join(sys.prefix, 'DLLS',
                                                   DLL_NAME))
    else:
        lib = ctypes.cdll.LoadLibrary(os.path.join(sys.prefix, 'local', 'DLLs',
                                                   DLL_NAME))


FSCC_UPDATE_VALUE = -2

XF, XREP, TXT, TXEXT = 0, 1, 2, 4
FSCC_TIMEOUT, FSCC_INCORRECT_MODE, \
    FSCC_BUFFER_TOO_SMALL, \
    FSCC_PORT_NOT_FOUND, \
    FSCC_INVALID_ACCESS, \
    FSCC_INVALID_PARAMETER = 16000, 16001, 16002, 16003, 16004, 16005

NOT_SUPPORTED_TEXT = 'This feature isn\'t supported on this port.'


class PortNotFoundError(OSError):
    def __init__(self, port_num=None):
        if port_num:
            error_msg = 'Port {} not found'.format(port_num)
        else:
            error_msg = 'Port not found'

        super(PortNotFoundError, self).__init__(error_msg)


class InvalidAccessError(OSError):
    def __init__(self):
        super(InvalidAccessError, self).__init__('Invalid access')


class TimeoutError(OSError):
    def __str__(self):
        return 'Command timed out (missing clock)'


class BufferTooSmallError(OSError):
    def __str__(self):
        return 'Buffer too small'


class IncorrectModeError(OSError):
    def __str__(self):
        return 'Incorrect mode'


class InvalidParameterError(ValueError):
    def __str__(self):
        return 'Invalid parameter'


class Port(object):
    """Commtech FSCC port."""
    class Registers(object):
        """Registers on the FSCC port."""
        register_names = ['FIFOT', 'CMDR', 'STAR', 'CCR0', 'CCR1', 'CCR2',
                          'BGR', 'SSR', 'SMR', 'TSR', 'TMR', 'RAR', 'RAMR',
                          'PPR', 'TCR', 'VSTR', 'IMR', 'DPLLR', 'FCR']

        readonly_register_names = ['STAR', 'VSTR']
        writeonly_register_names = ['CMDR']

        editable_register_names = [r for r in register_names if r not in
                                   ['STAR', 'VSTR']]

        def __init__(self, port):
            self.port = port
            self._clear_registers()

            for register in self.register_names:
                self._add_register(register)

        def _internal_registers(self):
            return [-1, -1, self._FIFOT, -1, -1, self._CMDR, self._STAR,
                    self._CCR0, self._CCR1, self._CCR2, self._BGR,
                    self._SSR, self._SMR, self._TSR, self._TMR, self._RAR,
                    self._RAMR, self._PPR, self._TCR, self._VSTR, -1,
                    self._IMR, self._DPLLR, self._FCR]

        def _add_register(self, register):
            """Dynamically add a way to edit a register to the port."""
            if register not in self.writeonly_register_names:
                fget = lambda self: self._get_register(register)
            else:
                fget = None

            if register not in self.readonly_register_names:
                fset = lambda self, value: self._set_register(register, value)
            else:
                fset = None

            setattr(self.__class__, register, property(fget, fset, None, ""))

        def _get_register(self, register):
            """Gets the value of a register."""
            self._clear_registers()
            setattr(self, '_%s' % register, FSCC_UPDATE_VALUE)
            self._get_registers()

            return getattr(self, '_%s' % register)

        def _set_register(self, register, value):
            """Sets the value of a register."""
            self._clear_registers()
            setattr(self, '_%s' % register, int(value))
            self._set_registers()

        def _clear_registers(self):
            """Clears the stored register values."""
            for register in self.register_names:
                setattr(self, "_%s" % register, -1)

        def _get_registers(self):
            """Gets all of the register values."""
            registers = self._internal_registers()

            fmt = 'q' * len(registers)
            regs = self.port._ctypes_get_struct(lib.fscc_get_registers, fmt,
                                                registers)

            for i, register in enumerate(registers):
                if register != -1:
                    self._set_register_by_index(i, regs[i])

        def _set_registers(self):
            """Sets all of the register values."""
            registers = self._internal_registers()

            fmt = 'q' * len(registers)
            buf = struct.pack(fmt, *registers)
            e = lib.fscc_set_registers(self.port._handle, buf)

            if e == 0:
                pass
            elif e == FSCC_TIMEOUT:
                raise TimeoutError()
            else:
                raise OSError(e)

        def _set_register_by_index(self, index, value):
            """Sets the value of a register by it's index."""
            data = [('FIFOT', 2), ('CMDR', 5), ('STAR', 6), ('CCR0', 7),
                    ('CCR1', 8), ('CCR2', 9), ('BGR', 10), ('SSR', 11),
                    ('SMR', 12), ('TSR', 13), ('TMR', 14), ('RAR', 15),
                    ('RAMR', 16), ('PPR', 17), ('TCR', 18), ('VSTR', 19),
                    ('IMR', 21), ('DPLLR', 22), ('FCR', 23)]

            for r, i in data:
                if i == index:
                    setattr(self, '_%s' % r, value)

        # Note: clears registers
        def import_from_file(self, import_file):
            """Reads and stores the register values from a file."""
            import_file.seek(0, os.SEEK_SET)

            for line in import_file:
                try:
                    line = str(line, encoding='utf8')
                except:
                    pass

                if line[0] != '#':
                    d = line.split('=')
                    reg_name, reg_val = d[0].strip().upper(), d[1].strip()

                    if reg_name in self.editable_register_names:
                        if reg_val[0] == '0' and reg_val[1] in ['x', 'X']:
                            reg_val = int(reg_val, 16)
                        else:
                            reg_val = int(reg_val)

                        setattr(self, reg_name, reg_val)

        def export_to_file(self, export_file):
            """Writes the current register values to a file."""
            for reg_name in self.editable_register_names:
                if reg_name in self.writeonly_register_names:
                    continue

                value = getattr(self, reg_name)

                if value >= 0:
                    export_file.write('%s = 0x%08x\n' % (reg_name, value))

        def __len__(self):
            return len(self.register_names)

        def __getitem__(self, key):
            return self._get_register(key)

        def __setitem__(self, key, value):
            self._set_register(key, value)

        def __iter__(self):
            for reg_name in self.register_names:
                if reg_name not in self.writeonly_register_names:
                    yield (reg_name, self[reg_name])
                else:
                    yield (reg_name, None)

        def __str__(self):
            reg_values = []
            for reg_name, reg_value in self:
                if reg_value is not None:
                    reg_values.append((reg_name, '0x{:08x}'.format(reg_value)))
                else:
                    reg_values.append((reg_name, None))
            return str(reg_values)

    class MemoryCap(object):

        def __init__(self, port=None):
            self.port = port

        def _set_memcap(self, input_memcap, output_memcap):
            """Sets the value of the memory cap setting."""
            fmt = 'i' * 2
            buf = struct.pack(fmt, input_memcap, output_memcap)
            lib.fscc_set_memory_cap(self.port._handle, buf)

        def _get_memcap(self):
            """Gets the value of the memory cap setting."""
            return self.port._ctypes_get_struct(lib.fscc_get_memory_cap,
                                                'i' * 2, (-1, -1))

        def _set_imemcap(self, memcap):
            """Sets the value of the input memory cap setting."""
            self._set_memcap(int(memcap), -1)

        def _get_imemcap(self):
            """Gets the value of the output memory cap setting."""
            return self._get_memcap()[0]

        input = property(fset=_set_imemcap, fget=_get_imemcap)

        def _set_omemcap(self, memcap):
            """Sets the value of the output memory cap setting."""
            self._set_memcap(-1, int(memcap))

        def _get_omemcap(self):
            """Gets the value of the output memory cap setting."""
            return self._get_memcap()[1]

        output = property(fset=_set_omemcap, fget=_get_omemcap)

    def __init__(self, port_num, append_status=True, append_timestamp=True):
        self._handle = ctypes.c_void_p()

        e = lib.fscc_connect(port_num, ctypes.byref(self._handle))

        Port._check_error(e)

        self._handle = self._handle.value
        self._port_num = port_num

        self.registers = Port.Registers(self)
        self.memory_cap = Port.MemoryCap(self)

        if append_status is not None:
            self.append_status = append_status

        if append_timestamp is not None:
            self.append_timestamp = append_timestamp

    def purge(self, tx=True, rx=True):
        """Removes unsent and/or unread data from the card."""
        self._ctypes_set(lib.fscc_purge, bool(tx), bool(rx))

    def _set_append_status(self, status):
        """Sets the value of the append status setting."""
        self._ctypes_set_bool(lib.fscc_enable_append_status,
                              lib.fscc_disable_append_status,
                              status)

    def _get_append_status(self):
        """Gets the value of the append status setting."""
        return self._ctypes_get_bool(lib.fscc_get_append_status)

    append_status = property(fset=_set_append_status, fget=_get_append_status)

    def _set_append_timestamp(self, status):
        """Sets the value of the append timestamp setting."""
        self._ctypes_set_bool(lib.fscc_enable_append_timestamp,
                              lib.fscc_disable_append_timestamp,
                              status)

    def _get_append_timestamp(self):
        """Gets the value of the append timestamp setting."""
        return self._ctypes_get_bool(lib.fscc_get_append_timestamp)

    append_timestamp = property(fset=_set_append_timestamp,
                                fget=_get_append_timestamp)

    def _set_ignore_timeout(self, status):
        """Sets the value of the ignore timeout setting."""
        self._ctypes_set_bool(lib.fscc_enable_ignore_timeout,
                              lib.fscc_disable_ignore_timeout,
                              status)

    def _get_ignore_timeout(self):
        """Gets the value of the ignore timeout setting."""
        return self._ctypes_get_bool(lib.fscc_get_ignore_timeout)

    ignore_timeout = property(fset=_set_ignore_timeout,
                              fget=_get_ignore_timeout)

    def _set_tx_modifiers(self, value):
        """Sets the value of the transmit modifiers setting."""
        self._ctypes_set(lib.fscc_set_tx_modifiers, int(value))

    def _get_tx_modifiers(self):
        """Gets the value of the transmit modifiers setting."""
        return self._ctypes_get_uint(lib.fscc_get_tx_modifiers)

    tx_modifiers = property(fset=_set_tx_modifiers, fget=_get_tx_modifiers)

    def _set_rx_multiple(self, status):
        """Sets the value of the rx multiple setting."""
        self._ctypes_set_bool(lib.fscc_enable_rx_multiple,
                              lib.fscc_disable_rx_multiple,
                              status)

    def _get_rx_multiple(self):
        """Gets the value of the ignore timeout setting."""
        return self._ctypes_get_bool(lib.fscc_get_rx_multiple)

    rx_multiple = property(fset=_set_rx_multiple, fget=_get_rx_multiple)

    def _set_clock_frequency(self, frequency):
        """Sets the value of the clock frequency setting."""
        self._ctypes_set(lib.fscc_set_clock_frequency, int(frequency))

    clock_frequency = property(fset=_set_clock_frequency)

    @staticmethod
    def _check_error(e):
        if e == 0:
            pass
        elif e == FSCC_TIMEOUT:
            raise TimeoutError()
        elif e == FSCC_INCORRECT_MODE:
            raise IncorrectModeError()
        elif e == FSCC_BUFFER_TOO_SMALL:
            raise BufferTooSmallError()
        elif e == FSCC_PORT_NOT_FOUND:
            raise PortNotFoundError()
        elif e == FSCC_INVALID_ACCESS:
            raise InvalidAccessError()
        elif e == FSCC_INVALID_PARAMETER:
            raise InvalidParameterError()
        else:
            raise OSError(e)

    def _ctypes_set_bool(self, enable_func, disable_func, status):
        func = enable_func if status else disable_func
        e = func(self._handle)
        Port._check_error(e)

    def _ctypes_get_bool(self, func):
        return bool(self._ctypes_get_uint(func))

    def _ctypes_set(self, func, *args):
        e = func(self._handle, *args)
        Port._check_error(e)

    def _ctypes_get_uint(self, func):
        status = ctypes.c_uint()
        e = func(self._handle, ctypes.byref(status))
        Port._check_error(e)
        return status.value

    def _ctypes_get_struct(self, func, fmt, initial):
        buf = struct.pack(fmt, *initial)
        e = func(self._handle, buf)
        Port._check_error(e)
        return struct.unpack(fmt, buf)

    def track_interrupts(self, interrupts, timeout=None):
        """Tracks interrupts."""
        matches = ctypes.c_uint()

        if timeout:
            e = lib.fscc_track_interrupts_with_timeout(self._handle, interrupts,
                                           ctypes.byref(matches), int(timeout))
        else:
            e = lib.fscc_track_interrupts_with_blocking(self._handle, interrupts,
                                            ctypes.byref(matches))

        Port._check_error(e)

        return matches.value

    @staticmethod
    def __parse_output_windows(append_status, append_timestamp, packet):
        data, status, timestamp = packet, None, None

        if (append_status and append_timestamp):
            status = packet[-10:-8]
            timestamp = struct.unpack('q', packet[-8:])[0]
            data = packet[:-10]
        elif (append_status):
            status = packet[-2:]
            data = packet[:-2]
        elif (append_timestamp):
            timestamp = struct.unpack('q', packet[-8:])[0]
            data = packet[:-8]

        if timestamp:
            timestamp = timestamp / 10000000 - 11644473600

        return (data, status, timestamp)

    @staticmethod
    def __parse_output_linux(append_status, append_timestamp, packet):
        data, status, timestamp = packet, None, None

        if ctypes.sizeof(ctypes.c_voidp) == 4:
            if (append_status and append_timestamp):
                status = packet[-10:-8]
                timestamp = struct.unpack('ll', packet[-8:])
                data = packet[:-10]
            elif (append_status):
                status = packet[-2:]
                data = packet[:-2]
            elif (append_timestamp):
                timestamp = struct.unpack('ll', packet[-8:])
                data = packet[:-8]

            if timestamp:
                timestamp = timestamp[0] + (float(timestamp[1]) / 1000000)
        else:
            if (append_status and append_timestamp):
                status = packet[-18:-16]
                timestamp = struct.unpack('ll', packet[-16:])
                data = packet[:-18]
            elif (append_status):
                status = packet[-2:]
                data = packet[:-2]
            elif (append_timestamp):
                timestamp = struct.unpack('ll', packet[-16:])
                data = packet[:-16]

            if timestamp:
                timestamp = timestamp[0] + (float(timestamp[1]) / 1000000)

        return (data, status, timestamp)

    def __parse_output(self, packet):
        if not packet:
            return (None, None, None)

        _append_status = self.append_status
        _append_timestamp = self.append_timestamp

        data, status, timestamp = packet, None, None

        if not self.rx_multiple:
            if os.name == 'nt':
                data, status, timestamp = Port.__parse_output_windows(
                    _append_status, _append_timestamp, packet)
            else:
                data, status, timestamp = Port.__parse_output_linux(
                    _append_status, _append_timestamp, packet)

        return (data, status, timestamp)

    def read(self, timeout=None, size=4096):
        """Reads data from the card."""
        bytes_read = ctypes.c_uint()
        data = bytes(int(size))

        if timeout:
            e = lib.fscc_read_with_timeout(self._handle, data, int(size),
                                           ctypes.byref(bytes_read), int(timeout))
        else:
            e = lib.fscc_read_with_blocking(self._handle, data, int(size),
                                            ctypes.byref(bytes_read))

        Port._check_error(e)

        return self.__parse_output(data[:bytes_read.value])

    def write(self, data):
        bytes_written = ctypes.c_uint()
        e = lib.fscc_write_with_blocking(self._handle, data, len(data),
                                         ctypes.byref(bytes_written))

        Port._check_error(e)

        return bytes_written.value

    def close(self):
        lib.fscc_disconnect(self._handle)

    def __str__(self):
        if os.name == 'nt':
            return 'FSCC{}'.format(self._port_num)
        else:
            return '/dev/fscc{}'.format(self._port_num)

    def __repr__(self):
        return 'fscc.Port({}, {}, {})'.format(self._port_num,
                                              self.append_status,
                                              self.append_timestamp)


if __name__ == '__main__':
    p = Port(0)

    print("Append Status", p.append_status)
    print("Append Timestamp", p.append_timestamp)
    print("Input Memory Cap", p.memory_cap.input)
    print("Output Memory Cap", p.memory_cap.output)
    print("Ignore Timeout", p.ignore_timeout)
    print("Transmit Modifiers", p.tx_modifiers)
    print("RX Multiple", p.rx_multiple)

    for reg_name, reg_value in p.registers:
        if reg_value is not None:
            print(reg_name, hex(reg_value))

    p.append_status = True
    p.append_timestamp = True
    p.memory_cap.input = 1000000
    p.memory_cap.output = 1000000
    p.ignore_timeout = False
    p.tx_modifiers = 0
    p.rx_multiple = False
    p.clock_frequency = 18432000

    p.purge()

    p.write(b'U')
    print(p.read(100))

    p.close()
