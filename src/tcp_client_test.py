
import sys
import socket


HOST = 'localhost'
PORT = 5001


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (HOST, PORT)
    print('connecting to %s on port %s' % server_address, file=sys.stderr)
    sock.connect(server_address)

    commands = [
        'Hello from Python!'.encode('utf-8'),
        'COMMAND 1'.encode('utf-8'),
        'KEYON.123.456'.encode('utf-8'),
        'VOICE0.ALGORITHM=10'.encode('utf-8'),
        'VOICE0.OP1.PHASE_STEP=450'.encode('utf-8'),
    ]

    try:
        for command in commands:
            # Send data
            # message = 'Hello from Python!'.encode('utf-8')
            message = command

            print('sending "%s"' % message, file=sys.stderr)
            sock.sendall(message)

            # Look for the response
            amount_received = 0
            amount_expected = len(message)

            while amount_received < amount_expected:
                data = sock.recv(16)
                amount_received += len(data)
                print('received "%s"' % data, file=sys.stderr)

    finally:
        print('closing socket', file=sys.stderr)
        sock.close()


if __name__ == '__main__':
    sys.exit(main())
