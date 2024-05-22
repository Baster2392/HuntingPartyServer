import socket
import struct
import threading
import time
import pygame
import sys
import ctypes

ctypes.windll.user32.SetProcessDPIAware()

BUFFER_SIZE = 260
targets = []


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


def handle_user_input111(client_socket):
    global targets
    last_print_time = time.time()  # Initialize last_print_time here
    while True:
        #### FRAGMENT 2
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

        # input_thread = threading.Thread(target=handle_user_input, args=(client_socket,))
        # input_thread.daemon = True
        # input_thread.start()

        ############################################
        pygame.init()
        info = pygame.display.Info()
        SCREEN_WIDTH = info.current_w
        SCREEN_HEIGHT = info.current_h
        MAP_WIDTH = 5000
        MAP_HEIGHT = 1080
        WHITE = (255, 255, 255)

        # Inicjalizacja ekranu
        screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT),
                                         pygame.FULLSCREEN | pygame.HWSURFACE | pygame.DOUBLEBUF)
        pygame.display.set_caption("Przesuwająca się mapa")
        map_image = pygame.transform.scale(pygame.image.load("mapa.jpg"),
                                           (int(SCREEN_HEIGHT * MAP_WIDTH / MAP_HEIGHT), SCREEN_HEIGHT))
        MAP_WIDTH = map_image.get_width()  # aktualizacja szerokości mapy po skalowaniu
        # Wczytanie obrazka celownika
        crosshair_image = pygame.image.load("celownik.png").convert_alpha()
        target_image = pygame.image.load("cel.png").convert_alpha()

        # Pozycja mapy
        map_x = 0
        map_y = 0

        # Prędkość przesuwania mapy
        scroll_speed = 50  # prędkość przewijania

        # Ukrycie systemowego kursora myszy
        pygame.mouse.set_visible(False)

        # Główna pętla gry
        running = True

        ########################################################

        input_thread = threading.Thread(target=handle_user_input111, args=(client_socket,))
        input_thread.daemon = True
        input_thread.start()

        while True:
            #### FRAGMENT 1
            #######################################################
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False  # dodane wyjście przez ESC
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    if event.button == 1:  # Lewy przycisk myszy
                        mouse_x, mouse_y = event.pos
                        map_cursor_x = (mouse_x - map_x) % MAP_WIDTH
                        map_cursor_y = mouse_y  # Y pozostaje bez zmian, jeśli mapa nie przewija się pionowo
                        shot = Shot(map_cursor_x, map_cursor_y)
                        send_shot(client_socket, shot)
                        print(f"Oddano strzał – Współrzędne kursora względem mapy: {map_cursor_x}, {map_cursor_y}")

            # Płynne przesuwanie mapy
            keys = pygame.key.get_pressed()
            if keys[pygame.K_LEFT]:
                map_x = (map_x + scroll_speed) % MAP_WIDTH
            if keys[pygame.K_RIGHT]:
                map_x = (map_x - scroll_speed) % MAP_WIDTH

            for target in targets:
                target["position_x"]
                target["position_y"]
                # Jeśli cel wyjdzie poza mapę, przenieś go na przeciwną stronę
                if target["position_x"] > MAP_WIDTH:
                    target["position_x"] -= MAP_WIDTH
                if target["position_y"] > MAP_HEIGHT:
                    target["position_y"] -= MAP_HEIGHT

                # zmiana znaku wektora pionowego po uderzeniu w górną lub dolną krawędź
                # if target["position_y"] > MAP_HEIGHT - 256 or target["position_y"] < 0:
                #    target["movement_vector"][1] = -target["movement_vector"][1]

            # Rysowanie
            screen.fill(WHITE)
            screen.blit(map_image, (map_x, map_y))
            # Obsługa zapętlenia mapy
            if map_x > 0:
                screen.blit(map_image, (map_x - MAP_WIDTH, map_y))
            elif map_x < SCREEN_WIDTH - MAP_WIDTH:
                screen.blit(map_image, (map_x + MAP_WIDTH, map_y))

            for target in targets:
                x = target["position_x"] + map_x
                while (x > 5000 - 199): x -= 5000
                while (x < -199): x += 5000
                screen.blit(target_image, (x, target["position_y"]))

            # Rysowanie celownika na pozycji kursora
            mouse_x, mouse_y = pygame.mouse.get_pos()
            screen.blit(crosshair_image,
                        (mouse_x - crosshair_image.get_width() / 2, mouse_y - crosshair_image.get_height() / 2))

            # Aktualizacja ekranu
            pygame.display.flip()

            #################################################










    except ConnectionRefusedError:
        print("Unable to connect to the server.")
    finally:
        client_socket.close()
        pygame.quit()
        sys.exit()


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

