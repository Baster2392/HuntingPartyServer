import socket
import time
import struct
from message import Message

BUFFER_SIZE = 260



def main():
    server_address = '127.0.0.1'
    server_port = 5000

    # Create socket of client
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Try to connect to server
        client_socket.connect((server_address, server_port))
        # Get id
        received_id = receive_message(client_socket)
        print("Przydzielone id:", int(received_id))

        # Send "ready" status
        message = Message("ready")
        packed_message = struct.pack("256si", message.content.encode(), message.controlSum)
        client_socket.send(packed_message)
        print("Message sent")

        while True:
            # Wait for "starting_game" status
            server_response = receive_message(client_socket)
            print(server_response)
            if server_response == "starting_game":
                print("Game is starting")
            else:
                time.sleep(1)

    except ConnectionRefusedError:
        print("Nie można połączyć się z serwerem.")
    finally:
        # Close socket
        client_socket.close()


def little_endian_to_int(message):
    decoded_int = 0
    multiplier = 1
    for byte in message.encode():
        decoded_int += int(str(byte), 16) * multiplier
        multiplier *= 256
    return decoded_int


def receive_message(client_socket):
    server_response = client_socket.recv(BUFFER_SIZE)
    if not server_response:
        return None

    received_struct = struct.unpack("256si", server_response)
    content, control_sum = received_struct
    content = content.decode().rstrip('\x00')

    if calculate_control_sum(content) != control_sum:
        print("Data has been lost.")
        return None

    return content


def calculate_control_sum(content):
    csum = 0
    for char in content:
        csum += ord(char)
    return csum % 256


if __name__ == "__main__":
    main()
