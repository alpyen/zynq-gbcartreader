import argparse
import serial
import sys
import time

parser = argparse.ArgumentParser(
    prog="reader",
    description=
        "Dispatches serial commmands to the board on a given port and baudrate as 8N1.\r\n"
        "Use this tool with the standalone implementations of the ZYNQ GBCartReader.",
    formatter_class=argparse.RawTextHelpFormatter
)
parser.add_argument("-p", "--port", type=str, required=True, help="Serial port the board is connected to")
parser.add_argument("-b", "--baudrate", type=int, required=True, default=115200, help="Baudrate of the connection (default: 115200)")
parser.add_argument("command", nargs=argparse.REMAINDER, help="Command to send (show header, read rom, help, ...)")

args = parser.parse_args(args=None if sys.argv[1:] else ["--help"])

command = " ".join(args.command)

# For now there is no mechanism to detect the number of rom or ram banks
# to terminate the serial connection so we rely on a timeout of 3 seconds
# because reading out takes less than 3 seconds.
# TODO: Read out header here and wait for correct amount of bytes?
# TODO: Add congestion control by ACKing after x amount of bytes in long responses (both sides!)
TRANSFER_FINISH_TIMEOUT = 3

with serial.Serial(args.port, args.baudrate, bytesize=8, parity="N", stopbits=1) as link:
    print(f"Sending command: {command}", file=sys.stderr)
    link.write((command + "\r").encode("ascii"))

    if command == "write ram":
        ram = sys.stdin.buffer.read()

        print(f"Sending data...", end="", flush=True, file=sys.stderr)

        for bank in range(len(ram) // 8192):
            for burst in range(8192 // 64):
                link.write(ram[bank*8192+burst*64:bank*8192+(burst+1)*64])
                received = 0
                while received < 64:
                    in_waiting = link.in_waiting
                    if in_waiting > 0:
                        link.read(in_waiting)
                        received += in_waiting

                print(f"\rSending data...{(bank*8192+(burst+1)*64)//1024}K/{len(ram)//1024}K", end="", flush=True, file=sys.stderr)

        print("...done!")
        exit(0)

    bytes_received = 0

    print(f"Receiving data...", end="", flush=True, file=sys.stderr)
    last_read = time.time()

    try:
        while time.time() - last_read <= TRANSFER_FINISH_TIMEOUT:
            if link.in_waiting > 0:
                # If we don't store the value it may change in between
                # adding it to bytes_received and calling the actual link.read
                in_waiting = link.in_waiting
                bytes_received += in_waiting
                sys.stdout.buffer.write(link.read(in_waiting))
                sys.stdout.flush()
                last_read = time.time()

                print(f"\rReceiving data...{bytes_received//1024}K", end="", flush=True, file=sys.stderr)

    except KeyboardInterrupt:
        pass

print("...done!", file=sys.stderr)
