# Win32 Wireshark named pipes example
# Requires Python for Windows and the Python for Windows Extensions:
# http://www.python.org
# http://sourceforge.net/projects/pywin32/


import binascii
import argparse
import threading
import signal
import sys
import fscc
import os

if os.name == 'nt':
    import win32pipe
    import win32file
    import win32event

if os.name == 'nt':
    default_pipe_dir = r'\\.\pipe\wireshark'
else:
    default_pipe_dir = r'/tmp/'

app_description = 'Open a pipe for Wireshark analysis.'

verbose_help = 'display incoming packets on screen'
capture_help = 'the directory to store the pcap files'
pipe_help = 'the directory to store the pipes'
prepend_help = 'prepend data with port number'
port_nums_help = 'the port\'s to capture'


def to_hex(data):
    return binascii.a2b_hex(''.join(data.split()))


def global_header():
    #Global header for pcap 2.4
    pcap_global_header = ('D4 C3 B2 A1'
                          '02 00'  # File format major revision (pcap <2>.4)
                          '04 00'  # File format minor revision (pcap 2.<4>)
                          '00 00 00 00'
                          '00 00 00 00'
                          'FF FF 00 00'
                          '93 00 00 00')

    return to_hex(pcap_global_header)


def packet_header(data, timestamp):
    #pcap packet header that must preface every packet
    pcap_packet_header = ('AA 77 9F 47'
                          '90 A2 04 00'
                          'XX XX XX XX'   # Frame Size (little endian)
                          'YY YY YY YY')  # Frame Size (little endian)

    hex_str = "%08x" % len(data)
    reverse_hex_str = hex_str[6:] + hex_str[4:6] + hex_str[2:4] + hex_str[:2]
    pcaph = pcap_packet_header.replace('XX XX XX XX', reverse_hex_str)
    pcaph = pcaph.replace('YY YY YY YY', reverse_hex_str)

    hex_str = "%08x" % int(timestamp)
    reverse_hex_str = hex_str[6:] + hex_str[4:6] + hex_str[2:4] + hex_str[:2]
    pcaph = pcaph.replace('AA 77 9F 47', reverse_hex_str)

    hex_str = "%08x" % ((timestamp % 1) * 1000000)
    reverse_hex_str = hex_str[6:] + hex_str[4:6] + hex_str[2:4] + hex_str[:2]
    pcaph = pcaph.replace('90 A2 04 00', reverse_hex_str)

    return to_hex(pcaph)


class ThreadClass(threading.Thread):

    def __init__(self, port_num, verbose, prepend, capture_dir, pipe_dir):
        super(ThreadClass, self).__init__()

        self.port_num = port_num
        self.verbose = verbose
        self.prepend = prepend
        self.capture_dir = capture_dir

        self.shutdown = True
        self.connected = False
        self.connection_started = False

        self.port = fscc.Port(self.port_num, 'r')

        pipe_name = 'fscc{}'.format(self.port_num)
        self.pipe_path = os.path.join(pipe_dir, pipe_name)

        self.file_name = r'fscc{}.pcap'.format(self.port_num)

    def stop(self):
        self.shutdown = True

    def _create_pipe(self):
        if os.name == 'nt':
            pipe = win32pipe.CreateNamedPipe(
                self.pipe_path,
                win32pipe.PIPE_ACCESS_OUTBOUND | win32file.FILE_FLAG_OVERLAPPED,
                win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
                1, 65536, 65536,
                300,
                None)

            ol = win32file.OVERLAPPED()
            ol.hEvent = win32event.CreateEvent(None, 0, 0, None)
        else:
            try:
                os.mkfifo(self.pipe_path)
            except FileExistsError:
                pass

            pipe, ol = None, None

        return pipe, ol

    def _connect_pipe(self, pipe, ol):
        if self.connected:
            return True, pipe

        if self.connection_started is False:
            if os.name == 'nt':
                win32pipe.ConnectNamedPipe(pipe, ol)

            self.connection_started = True

        if os.name == 'nt':
            r = win32event.WaitForSingleObject(ol.hEvent, 0)
            if r == win32event.WAIT_OBJECT_0:
                self.connected = True
                self.connection_started = False
                return True, pipe
            else:
                return False, None
        else:
            try:
                pipe = os.open(self.pipe_path, os.O_WRONLY | os.O_NONBLOCK)
            except OSError:
                return False, None
            else:
                self.connected = True
                self.connection_started = False
                return True, pipe

    def _write_packet(self, data, pipe, file, ol):
        # send data to terimanl if in verbose mode
        if self.verbose:
            print('fscc{}'.format(self.port_num), data)

        if self.prepend:
            raw_data = bytes((self.port_num,)) + data[0]
        else:
            raw_data = data[0]

        # Send data to pipe. if pipe is disconnected, reconnect
        if self.connected:
            if os.name == 'nt':
                try:
                    win32file.WriteFile(pipe, packet_header(data))
                    win32file.WriteFile(pipe, data[0])
                except:
                    win32pipe.DisconnectNamedPipe(pipe)
                    ol.hEvent = win32event.CreateEvent(None, 0, 0, None)
                    self.connected = False
            else:
                os.write(pipe, packet_header(data))
                os.write(pipe, data[0])

        # write data to file if in capture mode
        if file:
            file.write(packet_header(raw_data, data[2]))
            file.write(raw_data)

    def run(self):
        self.shutdown = False
        pipe, ol = self._create_pipe()

        if self.capture_dir:
            path = os.path.join(self.capture_dir, self.file_name)
            file = open(path, 'wb+')
            file.write(global_header())
        else:
            file = None

        while self.shutdown is False:
            data = self.port.read(10)

            if not self.connected:
                connected, pipe = self._connect_pipe(pipe, ol)
                if connected:
                    if os.name == 'nt':
                        win32file.WriteFile(pipe, global_header())
                        win32file.CloseHandle(ol.hEvent)
                    else:
                        os.write(pipe, global_header())

            # Length check added due to customer seeing data[0] being
            # blank but not None
            if data[0] is None or len(data[0]) == 0:
                continue

            self._write_packet(data, pipe, file, ol)

        if file:
            file.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description=app_description)
    parser.add_argument('-v', '--verbose', action='store_true',
                        help=verbose_help)
    parser.add_argument('-p', '--prepend', action='store_true',
                        help=prepend_help)
    parser.add_argument('-c', '--capture_dir', help=capture_help)
    parser.add_argument('-d', '--pipe_dir', default=default_pipe_dir,
                        help=pipe_help)
    parser.add_argument('port_nums', metavar='PORT_NUM', type=int, nargs='+',
                        help=port_nums_help)

    args = parser.parse_args()

    threads = []

    if args.capture_dir:
        capture_dir = os.path.abspath(args.capture_dir)
    else:
        capture_dir = None

    if args.pipe_dir:
        pipe_dir = os.path.abspath(args.pipe_dir)
    else:
        pipe_dir = None

    for port_num in args.port_nums:
        try:
            threads.append(ThreadClass(port_num, args.verbose, args.prepend,
                                       capture_dir, pipe_dir))
        except fscc.PortNotFoundError:
            print('Port fscc{} not found.'.format(port_num))

    if len(threads) == 0:
        exit(1)

    for t in threads:
        t.start()

    def signal_handler(signal, frame):
        print('Shutting down...')

        for t in threads:
            t.stop()
            t.join()

        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    print('Press Ctrl+C to exit')

    while True:
        pass
