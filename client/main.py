import socket
import struct
import threading
import time

BUFFER_SIZE = 260


class Message:
    def __init__(self, content):
        self.content = content
        self.control_sum = self.calculate_control_sum(content)

    def calculate_control_sum(self, content):
        return sum(ord(char) for char in content) % 256


class Shot:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.control_sum = self.calculate_control_sum()

    def calculate_control_sum(self):
        return sum([self.x, self.y]) % 256


def send_shot(client_socket, shot):
    packed_shot = struct.pack("iiI", shot.x, shot.y, shot.control_sum)
    client_socket.send(packed_shot)


def handle_user_input(client_socket):
    while True:
        try:
            x = int(input("Enter x coordinate for shot: "))
            y = int(input("Enter y coordinate for shot: "))
        except ValueError:
            print("Invalid input. Please enter integers for coordinates.")
            continue

        shot = Shot(x, y)
        send_shot(client_socket, shot)


def receive_target(client_socket):
    target_data = b''
    while len(target_data) < struct.calcsize("iiiII"):
        chunk = client_socket.recv(20)
        if not chunk:
            return None
        target_data += chunk

    target_struct = struct.unpack("iiiII", target_data)
    id, position_x, position_y, type, is_alive = target_struct

    return {
        "id": id,
        "position_x": position_x,
        "position_y": position_y,
        "type": type,
        "is_alive": is_alive
    }


def main():
    server_address = '127.0.0.1'
    server_port = 5000

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        client_socket.connect((server_address, server_port))
        received_id = receive_message(client_socket)
        print("Assigned ID:", int(received_id))

        input("Press Enter when you are ready... ")

        message = Message("ready")
        packed_message = struct.pack("256si", message.content.encode(), message.control_sum)
        client_socket.send(packed_message)
        print("Message sent")

        targets = []
        last_print_time = time.time()  # Initialize last_print_time here

        input_thread = threading.Thread(target=handle_user_input, args=(client_socket,))
        input_thread.daemon = True
        input_thread.start()

        while True:
            server_response = receive_message(client_socket)

            if server_response is None:
                continue

            print(server_response)
            if server_response == "starting_game":
                print("Game is starting")
            elif server_response == "end_game":
                print("Game ended. Exiting...")
                break
            elif server_response == "target":
                target = receive_target(client_socket)
                targets.append(target)
            elif server_response == "destroy":
                target = receive_target(client_socket)
                received_id_t = target['id']
                print("Destroyed target ID:", received_id_t)
                targets = [t for t in targets if t['id'] != received_id_t]
            else:
                time.sleep(1)

            current_time = time.time()
            if current_time - last_print_time >= 10:
                if len(targets) > 0:
                    print("List of targets:")
                    for target in targets:
                        print(f"ID: {target['id']}, X: {target['position_x']}, Y: {target['position_y']}")
                last_print_time = current_time

    except ConnectionRefusedError:
        print("Unable to connect to the server.")
    finally:
        client_socket.close()


def receive_message(client_socket):
    server_response = client_socket.recv(BUFFER_SIZE)
    if not server_response or len(server_response) != BUFFER_SIZE:
        return None

    content, control_sum = struct.unpack("256si", server_response)
    content = content.decode().rstrip('\x00')

    if Message(content).control_sum != control_sum:
        print("Data has been corrupted.")
        return None

    return content


if __name__ == "__main__":
    main()
