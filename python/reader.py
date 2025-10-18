import argparse
import serial
import sys
import time

from enum import Enum

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

if len(args.command) == 0:
    print("No command was passed. Exiting.")
    exit(0)

command = " ".join(args.command)

class ResponseType(Enum):
    OK                      = 0
    UNKNOWN_COMMAND         = 1

    # Broken Cartridge
    INVALID_NUM_ROM_BANKS   = 10
    INVALID_NUM_RAM_BANKS   = 11
    INVALID_CARTRIDGE_TYPE  = 12

    # PC tries op that the cart cannot handle
    INVALID_RAM_WRITE_SIZE  = 21
    CARTRIDGE_HAS_NO_RAM    = 22
    CARTRIDGE_HAS_NO_RTC    = 23
    INVALID_RTC_WRITE_SIZE  = 24

# Waits until atleast n bytes are in our RX buffer.
# Times out if last receive is more than 3 seconds ago.
def wait_for_n_serial_bytes(count):
    global serial

    def timeout():
        _TRANSFER_TIMEOUT = 30

        if not hasattr(timeout, "last_comm"):
            timeout.last_comm = time.time()

        return ((time.time() - last_comm) >= _TRANSFER_TIMEOUT)

    while not timeout() and link.in_waiting < count:
        pass

    if timeout():
        log("Transfer timed out. Exiting.")
        exit(0)

    timeout.last_comm = time.time()


# We do our logging into stderr so the data can be piped from stdout to a file with the shell.
def log(message, end="\n"):
    print(message, end=end, flush=True, file=sys.stderr)

with serial.Serial(args.port, args.baudrate, bytesize=8, parity="N", stopbits=1) as link:

    log(f"Sending command: {command}")
    link.write((command + "\r").encode("ascii"))
    last_comm = time.time()

    wait_for_n_serial_bytes(1)

    try:
        response = int.from_bytes(link.read(1))

        match ResponseType(response):
            case ResponseType.OK:
                pass

            case ResponseType.UNKNOWN_COMMAND:
                log("Command not recognized. Try \"help\" for command reference.")
                exit(0)

            case ResponseType.INVALID_NUM_ROM_BANKS:
                log("Cartridge has invalid amount of ROM banks. Broken cartridge/Bad connection?")
                exit(0)

            case ResponseType.CARTRIDGE_HAS_NO_RAM:
                log("Cartridge has no RAM.")
                exit(0)

            case ResponseType.INVALID_NUM_RAM_BANKS:
                log("Cartridge has invalid amount of RAM banks. Broken cartridge/Bad connection?")
                exit(0)

            case ResponseType.INVALID_CARTRIDGE_TYPE:
                log("Cartridge type not recognized. Broken cartridge/Bad connection?")
                exit(0)

    except ValueError:
        log(f"Invalid response type: {response}")
        exit(0)


    if command == "help" or "parse header" == command or "read" in command:
        log("Receiving data...", "")

        wait_for_n_serial_bytes(4)

        bytes_received = 0
        bytes_to_receive = int.from_bytes(link.read(4), byteorder="little")

        while bytes_received != bytes_to_receive:
            # The contents of "help" and "parse header" may not be divisible by 64.
            bytes_to_read = 64 if ((bytes_to_receive - bytes_received) >= 64) else 1

            wait_for_n_serial_bytes(bytes_to_read)

            bytes_received += sys.stdout.buffer.write(link.read(bytes_to_read))

            if bytes_to_receive < 1024:
                log(f"\rReceiving data...{bytes_received}/{bytes_to_receive}B", "")
            else:
                log(f"\rReceiving data...{bytes_received//1024}K/{bytes_to_receive//1024}K", "")

        log("...done!")

    elif "write" in command:
        log("Sending size...", "")

        buffer = sys.stdin.buffer.read()
        buffer_length = len(buffer)

        link.write(buffer_length.to_bytes(4, byteorder="little"))
        log("done!")

        wait_for_n_serial_bytes(1)

        response = int.from_bytes(link.read(1))

        match ResponseType(response):
            case ResponseType.OK:
                pass

            case ResponseType.INVALID_RAM_WRITE_SIZE:
                log("RAM write size does not match cartridge RAM size.")
                exit(0)

            case ResponseType.INVALID_RTC_WRITE_SIZE:
                log("RTC write size does not match cartridge RTC size.")
                exit(0)

        log("Sending data...", "")

        RAM_BANK_SIZE = 0x2000
        BUFFER_CHUNK_SIZE = 64 # Zynq only has a 64 byte RX buffer
        bytes_sent = 0

        for i in range(buffer_length // BUFFER_CHUNK_SIZE):
            bytes_sent += link.write(buffer[i * BUFFER_CHUNK_SIZE: (i+1) * BUFFER_CHUNK_SIZE])

            wait_for_n_serial_bytes(BUFFER_CHUNK_SIZE)
            link.read(BUFFER_CHUNK_SIZE)

            log(f"\rSending data...{bytes_sent//1024}K/{buffer_length//1024}K", "")

        log("done!")

exit(0)
