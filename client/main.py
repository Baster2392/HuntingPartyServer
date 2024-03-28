import socket
import time

def main():
    server_address = '127.0.0.1'
    server_port = 5000

    # Create socket of client
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Try to connect to server
        client_socket.connect((server_address, server_port))
        # Get id
        server_response = client_socket.recv(1024).decode()
        player_id = little_endian_to_int(server_response)
        print("Przydzielone id:", str(player_id))

        # Send confirmation of receiving id
        message = "received_id"
        client_socket.sendall(message.encode())
        time.sleep(0.5)

        # Send "ready" status
        message = "ready"
        client_socket.sendall(message.encode())

        # Wait for "starting_game" status
        server_response = client_socket.recv(1024).decode()
        print(server_response)
        if server_response == "starting_game":
            print("Game is starting")

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


if __name__ == "__main__":
    main()
