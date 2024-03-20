import socket

def main():
    server_address = '127.0.0.1'
    server_port = 5000

    # Create socket of client
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Try to connect to server
        client_socket.connect((server_address, server_port))

        # Read messages from server
        print("Otrzymano wiadomości od serwera:")

        # Send message to server
        while True:
            message = input("Wpisz wiadomość (pusty wiersz aby zakończyć): ")
            if not message:
                break
            client_socket.sendall(message.encode())

            # Get answer from server
            server_response = client_socket.recv(1024).decode()
            print("Odpowiedź serwera:", server_response)

    except ConnectionRefusedError:
        print("Nie można połączyć się z serwerem.")
    finally:
        # Close socket
        client_socket.close()


if __name__ == "__main__":
    main()
