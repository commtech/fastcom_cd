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

using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace Fscc
{
    public enum TxModifiers { XF = 0, XREP = 1, TXT = 2, TXEXT = 4 };
    enum ErrorTypes { FSCC_TIMEOUT=16000, FSCC_INCORRECT_MODE, FSCC_BUFFER_TOO_SMALL, FSCC_PORT_NOT_FOUND, FSCC_INVALID_ACCESS, FSCC_INVALID_PARAMETER };

    [SerializableAttribute]
    public class PortNotFoundException : FileNotFoundException
    {
        public PortNotFoundException(uint port_num) : base(string.Format("Port {0} not found", port_num)) {}
    }

    [SerializableAttribute]
    public class InvalidAccessException : UnauthorizedAccessException
    {
        public InvalidAccessException() : base("Invalid access") {}
    }

    [SerializableAttribute]
    public class TimeoutException : SystemException
    {
        public TimeoutException() : base("Command timed out (missing clock)") {}
    }

    [SerializableAttribute]
    public class BufferTooSmallException : SystemException
    {
        public BufferTooSmallException() : base("Buffer too small") {}
    }

    [SerializableAttribute]
    public class IncorrectModeException : SystemException
    {
        public IncorrectModeException() : base("Incorrect mode") {}
    }

    [SerializableAttribute]
    public class InvalidParameterException : ArgumentException
    {
        public InvalidParameterException() : base("Invalid parameter") {}
    }

    public class Port : IDisposable
    {
        IntPtr _handle;
        uint _port_num;
        Registers _registers;
        MemoryCap _memcap;
        bool _disposed = false;

        public override string ToString()
        {
            return String.Format("FSCC{0}", _port_num);
        }

        public Port(uint port_num)
        {
            int e = NativeMethods.fscc_connect(port_num, out this._handle);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_PORT_NOT_FOUND:
                throw new PortNotFoundException(port_num);

            case (int)ErrorTypes.FSCC_INVALID_ACCESS:
                throw new InvalidAccessException();

            default:
                throw new SystemException(e.ToString());
            }

            this._port_num = port_num;
            this._registers = new Registers(this._handle);
            this._memcap = new MemoryCap(this._handle);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if(!_disposed)
            {
                NativeMethods.fscc_disconnect(_handle);

                _handle = IntPtr.Zero;
                _disposed = true;
            }
        }

        ~Port()
        {
            Dispose(false);
        }

        public TxModifiers TxModifiers
        {
            set
            {
                NativeMethods.fscc_set_tx_modifiers(this._handle, (uint)value);
            }

            get
            {
                uint modifiers;

                NativeMethods.fscc_get_tx_modifiers(this._handle, out modifiers);

                return (TxModifiers)modifiers;
            }
        }

        public bool AppendStatus
        {
            set
            {
                if (value == true)
                    NativeMethods.fscc_enable_append_status(this._handle);
                else
                    NativeMethods.fscc_disable_append_status(this._handle);
            }

            get
            {
                bool status;

                NativeMethods.fscc_get_append_status(this._handle, out status);

                return status;
            }
        }

        public bool AppendTimestamp
        {
            set
            {
                if (value == true)
                    NativeMethods.fscc_enable_append_timestamp(this._handle);
                else
                    NativeMethods.fscc_disable_append_timestamp(this._handle);
            }

            get
            {
                bool timestamp;

                NativeMethods.fscc_get_append_timestamp(this._handle, out timestamp);

                return timestamp;
            }
        }

        public bool IgnoreTimeout
        {
            set
            {
                if (value == true)
                    NativeMethods.fscc_enable_ignore_timeout(this._handle);
                else
                    NativeMethods.fscc_disable_ignore_timeout(this._handle);
            }

            get
            {
                bool status;

                NativeMethods.fscc_get_ignore_timeout(this._handle, out status);

                return status;
            }
        }

        public bool RxMultiple
        {
            set
            {
                if (value == true)
                    NativeMethods.fscc_enable_rx_multiple(this._handle);
                else
                    NativeMethods.fscc_disable_rx_multiple(this._handle);
            }

            get
            {
                bool status;

                NativeMethods.fscc_get_rx_multiple(this._handle, out status);

                return status;
            }
        }

        public void Purge(bool tx, bool rx)
        {
            int e = NativeMethods.fscc_purge(this._handle, tx, rx);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_TIMEOUT:
                throw new TimeoutException();

            default:
                throw new SystemException(e.ToString());
            }
        }

        public void Purge()
        {
            Purge(true, true);
        }

        public uint ClockFrequency
        {
            set
            {
                int e = NativeMethods.fscc_set_clock_frequency(this._handle, value);

                switch (e) {
                case 0:
                    break;

                case (int)ErrorTypes.FSCC_INVALID_PARAMETER:
                    throw new InvalidParameterException();

                default:
                    throw new SystemException(e.ToString());
                }
            }
        }

        public int TrackInterrupts(uint interrupts, out uint matches, out NativeOverlapped o)
        {
            int e = NativeMethods.fscc_track_interrupts(this._handle, interrupts, out matches, out o);

            switch (e) {
            case 0:
            case 997: // ERROR_IO_PENDING
                break;

            default:
                throw new SystemException(e.ToString());
            }

            return e;
        }

        public uint TrackInterrupts(uint interrupts)
        {
            uint matches;

            int e = NativeMethods.fscc_track_interrupts_with_blocking(this._handle, interrupts, out matches);

            if (e != 0)
                throw new SystemException(e.ToString());

            return matches;
        }

        public uint TrackInterrupts(uint interrupts, uint timeout)
        {
            uint matches;

            int e = NativeMethods.fscc_track_interrupts_with_timeout(this._handle, interrupts, out matches, timeout);

            if (e != 0)
                throw new SystemException(e.ToString());

            return matches;
        }

        public int Write(byte[] buf, uint size, out NativeOverlapped o)
        {
            uint bytes_written;

            int e = NativeMethods.fscc_write(this._handle, buf, size, out bytes_written, out o);

            switch (e) {
            case 0:
            case 997: // ERROR_IO_PENDING
                break;

            case (int)ErrorTypes.FSCC_BUFFER_TOO_SMALL:
                throw new BufferTooSmallException();

            case (int)ErrorTypes.FSCC_TIMEOUT:
                throw new TimeoutException();

            case (int)ErrorTypes.FSCC_INCORRECT_MODE:
                throw new IncorrectModeException();

            default:
                throw new SystemException(e.ToString());
            }

            return e;
        }

        public uint Write(byte[] buf, uint size)
        {
            uint bytes_written;

            int e = NativeMethods.fscc_write_with_blocking(this._handle, buf, size, out bytes_written);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_BUFFER_TOO_SMALL:
                throw new BufferTooSmallException();

            case (int)ErrorTypes.FSCC_TIMEOUT:
                throw new TimeoutException();

            case (int)ErrorTypes.FSCC_INCORRECT_MODE:
                throw new IncorrectModeException();

            default:
                throw new SystemException(e.ToString());
            }

            return bytes_written;
        }

        public uint Write(string str)
        {
            return Write(Encoding.ASCII.GetBytes(str), (uint)str.Length);
        }

        public int Read(byte[] buf, uint size, out uint bytes_read, out NativeOverlapped o)
        {
            int e = NativeMethods.fscc_read(this._handle, buf, size, out bytes_read, out o);

            switch (e) {
            case 0:
            case 997: // ERROR_IO_PENDING
                break;

            case (int)ErrorTypes.FSCC_BUFFER_TOO_SMALL:
                throw new BufferTooSmallException();

            case (int)ErrorTypes.FSCC_INCORRECT_MODE:
                throw new IncorrectModeException();

            default:
                throw new SystemException(e.ToString());
            }

            return e;
        }

        public uint Read(byte[] buf, uint size)
        {
            uint bytes_read;

            int e = NativeMethods.fscc_read_with_blocking(this._handle, buf, size, out bytes_read);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_BUFFER_TOO_SMALL:
                throw new BufferTooSmallException();

            case (int)ErrorTypes.FSCC_INCORRECT_MODE:
                throw new IncorrectModeException();

            default:
                throw new SystemException(e.ToString());
            }

            return bytes_read;
        }

        public uint Read(byte[] buf, uint size, uint timeout)
        {
            uint bytes_read;

            int e = NativeMethods.fscc_read_with_timeout(this._handle, buf, size, out bytes_read, timeout);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_BUFFER_TOO_SMALL:
                throw new BufferTooSmallException();

            case (int)ErrorTypes.FSCC_INCORRECT_MODE:
                throw new IncorrectModeException();

            default:
                throw new SystemException(e.ToString());
            }

            return bytes_read;
        }

        public string Read(uint size)
        {
            System.Text.ASCIIEncoding encoder = new System.Text.ASCIIEncoding();
            byte[] input_bytes = new byte[size];
            uint bytes_read = 0;

            bytes_read = Read(input_bytes, (uint)input_bytes.Length);

            return encoder.GetString(input_bytes, 0, (int)bytes_read);
        }

        public string Read()
        {
            return Read(4096);
        }

        public string Read(uint size, uint timeout)
        {
            System.Text.ASCIIEncoding encoder = new System.Text.ASCIIEncoding();
            byte[] input_bytes = new byte[size];
            uint bytes_read = 0;

            bytes_read = Read(input_bytes, (uint)input_bytes.Length, timeout);

            return encoder.GetString(input_bytes, 0, (int)bytes_read);
        }

        public string Firmware
        {
            get
            {
                uint vstr = this.Registers.VSTR;

                byte PREV = (byte)((vstr & 0x0000FF00) >> 8);
                byte FREV = (byte)((vstr & 0x000000FF));

                return String.Format("{0:X}.{1:X2}", PREV, FREV);
            }
        }

        public Registers Registers
        {
            get
            {
                return this._registers;
            }
        }

        public MemoryCap MemoryCap
        {
            get
            {
                return this._memcap;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct _Registers
    {
        /* BAR 0 */
        private Int64 __reserved11;
        private Int64 __reserved12;

        public Int64 FIFOT;

        private Int64 __reserved21;
        private Int64 __reserved22;

        public Int64 CMDR;
        public Int64 STAR; /* Read-only */
        public Int64 CCR0;
        public Int64 CCR1;
        public Int64 CCR2;
        public Int64 BGR;
        public Int64 SSR;
        public Int64 SMR;
        public Int64 TSR;
        public Int64 TMR;
        public Int64 RAR;
        public Int64 RAMR;
        public Int64 PPR;
        public Int64 TCR;
        public Int64 VSTR;

        private Int64 __reserved41;

        public Int64 IMR;
        public Int64 DPLLR;

        /* BAR 2 */
        public Int64 FCR;

        public _Registers(bool init)
        {
            /* BAR 0 */
            __reserved11 = -1;
            __reserved12 = -1;

            FIFOT = -1;

            __reserved21 = -1;
            __reserved22 = -1;

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

            __reserved41 = -1;

            IMR = -1;
            DPLLR = -1;

            /* BAR 2 */
            FCR = -1;
        }

    };

    public class Registers {
        IntPtr _handle;
        const int FSCC_UPDATE_VALUE = -2;

        public Registers(IntPtr h)
        {
            this._handle = h;
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
        private _Registers GetRegisters(_Registers r)
        {
            IntPtr buffer = Marshal.AllocHGlobal(Marshal.SizeOf(r));
            Marshal.StructureToPtr(r, buffer, false);

            NativeMethods.fscc_get_registers(this._handle, buffer);

            r = (_Registers)Marshal.PtrToStructure(buffer, typeof(_Registers));
            Marshal.FreeHGlobal(buffer);

            return r;
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
        private void SetRegisters(_Registers r)
        {
            IntPtr buffer = Marshal.AllocHGlobal(Marshal.SizeOf(r));
            Marshal.StructureToPtr(r, buffer, false);

            int e = NativeMethods.fscc_set_registers(this._handle, buffer);

            Marshal.FreeHGlobal(buffer);

            switch (e) {
            case 0:
                break;

            case (int)ErrorTypes.FSCC_TIMEOUT:
                throw new TimeoutException();

            default:
                throw new SystemException(e.ToString());
            }
        }

        public UInt32 FIFOT
        {
            set
            {
                _Registers r = new _Registers(true);

                r.FIFOT = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.FIFOT = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).FIFOT;
            }
        }

        public UInt32 CMDR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.CMDR = value;

                SetRegisters(r);
            }
        }

        public UInt32 STAR
        {
            get
            {
                _Registers r = new _Registers(true);

                r.STAR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).STAR;
            }
        }

        public UInt32 CCR0
        {
            set
            {
                _Registers r = new _Registers(true);

                r.CCR0 = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.CCR0 = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).CCR0;
            }
        }

        public UInt32 CCR1
        {
            set
            {
                _Registers r = new _Registers(true);

                r.CCR1 = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.CCR1 = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).CCR1;
            }
        }

        public UInt32 CCR2
        {
            set
            {
                _Registers r = new _Registers(true);

                r.CCR2 = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.CCR2 = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).CCR2;
            }
        }

        public UInt32 BGR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.BGR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.BGR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).BGR;
            }
        }

        public UInt32 SSR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.SSR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.SSR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).SSR;
            }
        }

        public UInt32 SMR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.SMR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.SMR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).SMR;
            }
        }

        public UInt32 TSR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.TSR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.TSR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).TSR;
            }
        }

        public UInt32 TMR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.TMR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.TMR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).TMR;
            }
        }

        public UInt32 RAR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.RAR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.RAR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).RAR;
            }
        }

        public UInt32 RAMR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.RAMR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.RAMR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).RAMR;
            }
        }

        public UInt32 PPR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.PPR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.PPR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).PPR;
            }
        }

        public UInt32 TCR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.TCR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.TCR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).TCR;
            }
        }

        public UInt32 VSTR
        {
            get
            {
                _Registers r = new _Registers(true);

                r.VSTR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).VSTR;
            }
        }

        public UInt32 IMR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.IMR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.IMR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).IMR;
            }
        }

        public UInt32 DPLLR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.DPLLR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.DPLLR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).DPLLR;
            }
        }

        public UInt32 FCR
        {
            set
            {
                _Registers r = new _Registers(true);

                r.FCR = value;

                SetRegisters(r);
            }

            get
            {
                _Registers r = new _Registers(true);

                r.FCR = FSCC_UPDATE_VALUE;

                return (UInt32)GetRegisters(r).FCR;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct _MemoryCap
    {
        public int input;
        public int output;

        public _MemoryCap(bool init)
        {
            input = -1;
            output = -1;
        }
    };

    public class MemoryCap {
        IntPtr _handle;

        public MemoryCap(IntPtr h)
        {
            this._handle = h;
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
        private _MemoryCap GetMemoryCap(_MemoryCap m)
        {
            IntPtr buffer = Marshal.AllocHGlobal(Marshal.SizeOf(m));
            Marshal.StructureToPtr(m, buffer, false);

            NativeMethods.fscc_get_memory_cap(this._handle, buffer);

            m = (_MemoryCap)Marshal.PtrToStructure(buffer, typeof(_MemoryCap));
            Marshal.FreeHGlobal(buffer);

            return m;
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands")]
        private void SetMemoryCap(_MemoryCap m)
        {
            IntPtr buffer = Marshal.AllocHGlobal(Marshal.SizeOf(m));
            Marshal.StructureToPtr(m, buffer, false);

            NativeMethods.fscc_set_memory_cap(this._handle, buffer);

            Marshal.FreeHGlobal(buffer);
        }

        public uint Input
        {
            set
            {
                _MemoryCap m = new _MemoryCap(true);

                m.input = (int)value;

                SetMemoryCap(m);
            }

            get
            {
                _MemoryCap m = new _MemoryCap(true);

                return (uint)GetMemoryCap(m).input;
            }
        }

        public uint Output
        {
            set
            {
                _MemoryCap m = new _MemoryCap(true);

                m.output = (int)value;

                SetMemoryCap(m);
            }

            get
            {
                _MemoryCap m = new _MemoryCap(true);

                return (uint)GetMemoryCap(m).output;
            }
        }
    }

    internal static class NativeMethods
    {
#if DEBUG
        public const string DLL_PATH = "cfsccd.dll";
#else
        public const string DLL_PATH = "cfscc.dll";
#endif

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_connect(uint port_num, out IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_disconnect(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_set_tx_modifiers(IntPtr h, uint modifiers);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_tx_modifiers(IntPtr h, out uint modifiers);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_enable_append_status(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_disable_append_status(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_append_status(IntPtr h, out bool status);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_enable_append_timestamp(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_disable_append_timestamp(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_append_timestamp(IntPtr h, out bool timestamp);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_enable_ignore_timeout(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_disable_ignore_timeout(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_ignore_timeout(IntPtr h, out bool status);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_enable_rx_multiple(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_disable_rx_multiple(IntPtr h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_rx_multiple(IntPtr h, out bool status);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_purge(IntPtr h, bool tx, bool rx);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_set_clock_frequency(IntPtr h, uint frequency);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_track_interrupts(IntPtr h, uint interrupts, out uint matches, out NativeOverlapped overlapped);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_track_interrupts_with_blocking(IntPtr h, uint interrupts, out uint matches);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_track_interrupts_with_timeout(IntPtr h, uint interrupts, out uint matches, uint timeout);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_write(IntPtr h, byte[] buf, uint size, out uint bytes_written, out NativeOverlapped overlapped);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_write_with_blocking(IntPtr h, byte[] buf, uint size, out uint bytes_written);

        [DllImport("kernel32.dll", SetLastError = true)]
        internal static extern bool GetOverlappedResult(
            IntPtr hDevice,
            out NativeOverlapped lpOverlapped,
            out uint lpNumberOfBytesTransferred,
            bool bWait
        );

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_read(IntPtr h, byte[] buf, uint size, out uint bytes_read, out NativeOverlapped overlapped);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_read_with_blocking(IntPtr h, byte[] buf, uint size, out uint bytes_read);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_read_with_timeout(IntPtr h, byte[] buf, uint size, out uint bytes_read, uint timeout);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_set_registers(IntPtr h, IntPtr registers);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_registers(IntPtr h, IntPtr registers);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_set_memory_cap(IntPtr h, IntPtr memcap);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int fscc_get_memory_cap(IntPtr h, IntPtr memcap);
    }
}