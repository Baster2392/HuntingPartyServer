import math
import socket
import struct
import threading
import time
import pygame
import sys
import ctypes
ctypes.windll.user32.SetProcessDPIAware()

Rozmiar_balona_x = 214
Rozmiar_balona_y = 244
gameStartTime = 0
game_started = False

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
    global targets, gameStartTime, game_started, leaderboard
    leaderboard = []
    last_print_time = time.time()
    while True:
        server_response = receive_message(client_socket)

        if server_response is None:
            continue

        print(server_response)
        if server_response == "starting_game":
            gameStartTime = time.time()
            game_started = True
            print("Start time:" + str(gameStartTime))
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
        elif server_response == "leaderboard":
            leaderboard = receive_leaderboard(client_socket)
        else:
            time.sleep(1)

        current_time = time.time()
        if current_time - last_print_time >= 1:
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
    global game_started,game_ended



    server_address = '127.0.0.1'
    #do podlaczania innych 192.168.56.1
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

        pygame.init()
        info = pygame.display.Info()
        SCREEN_WIDTH = info.current_w
        SCREEN_HEIGHT = info.current_h

        MAP_WIDTH = 4 * SCREEN_WIDTH
        MAP_HEIGHT = 1080
        WHITE = (255, 255, 255)
        BLACK = (0, 0, 0)
        GOLD = (187,165,61)
        SILVER = (165,169,180)
        BRONZE = (110,77,37)
        DEFAULT = (90,77,65)
        colors = [GOLD, SILVER, BRONZE, DEFAULT]
        game_ended = False
        screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.FULLSCREEN | pygame.HWSURFACE | pygame.DOUBLEBUF)
        pygame.display.set_caption("Przesuwająca się mapa")

        font = pygame.font.Font(None, 74)
        leaderboard_font = pygame.font.Font(None, 36)
        map_image = pygame.transform.scale(pygame.image.load("mapa.png"), ( MAP_WIDTH+10, SCREEN_HEIGHT))
        crosshair_image = pygame.image.load("celownik.png").convert_alpha()
        target_image = pygame.image.load("cel.png").convert_alpha()
        map_x = 0
        map_y = 0
        scroll_speed = 50

        pygame.mouse.set_visible(False)

        input_thread = threading.Thread(target=handle_user_input111, args=(client_socket,))
        input_thread.daemon = True
        input_thread.start()
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                elif event.type == pygame.MOUSEBUTTONDOWN and game_started:
                    if event.button == 1:
                        mouse_x, mouse_y = event.pos
                        map_cursor_x = (mouse_x - map_x) % MAP_WIDTH
                        map_cursor_y = mouse_y
                        shot = Shot(map_cursor_x, map_cursor_y)
                        send_shot(client_socket, shot)
                        print(f"Oddano strzał – Współrzędne kursora względem mapy: {map_cursor_x}, {map_cursor_y}")
            if game_started and not game_ended and time.time() - gameStartTime > 60:
                game_ended = True
            if game_ended:
                screen.fill(BLACK)
                font_large = pygame.font.Font(None, 144)
                text = font_large.render("Game Over", True, WHITE)
                text_rect = text.get_rect(center=(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4))
                screen.blit(text, text_rect)

                entry_surface = leaderboard_font.render("Final Leaderboard: ", True, WHITE)
                screen.blit(entry_surface, (10, SCREEN_HEIGHT / 2 - 40))
                try:
                    part1 = leaderboard[0][:12]

                    entry_surface = leaderboard_font.render(str(part1), True, colors[0])
                    screen.blit(entry_surface, (10, SCREEN_HEIGHT / 2+40))

                    part2 = leaderboard[0][12:24]

                    entry_surface = leaderboard_font.render(str(part2), True, colors[1])
                    screen.blit(entry_surface, (10, SCREEN_HEIGHT / 2+40 * 2))

                    part3 = leaderboard[0][24:37]

                    entry_surface = leaderboard_font.render(str(part3), True, colors[2])
                    screen.blit(entry_surface, (10, SCREEN_HEIGHT / 2+40 * 3))

                    part4 = leaderboard[0][36:49]

                    entry_surface = leaderboard_font.render(str(part4), True, colors[3])
                    screen.blit(entry_surface, (10, SCREEN_HEIGHT / 2+40 * 4))
                except Exception as e:
                    print(f"Error rendering final leaderboard: {e}")


            if game_started and not game_ended:
                keys = pygame.key.get_pressed()
                if keys[pygame.K_LEFT]:
                    map_x = (map_x + scroll_speed) % MAP_WIDTH
                if keys[pygame.K_RIGHT]:
                    map_x = (map_x - scroll_speed) % MAP_WIDTH

                for target in targets:
                    current_time = time.time() - gameStartTime

                    if target["type"] == 0:

                        target["position_y"] = SCREEN_HEIGHT / 2 + 100 * math.cos(current_time + target['id'])
                        target["position_x"] = (100 * current_time + target['id'] * 50) % MAP_WIDTH

                    elif target["type"] == 1:
                        radius = 250
                        center_x = (100 * current_time + target['id'] * 50) % MAP_WIDTH
                        center_y = SCREEN_HEIGHT / 2
                        target["position_x"] = center_x + radius * math.cos(current_time + target['id'])
                        target["position_y"] = center_y + radius * math.sin(current_time + target['id'])
                    else:
                        target["position_y"] = SCREEN_HEIGHT / 2 + 100 * math.cos(current_time + target['id'])
                        target["position_x"] = (100 * (60-current_time) + target['id'] * 50) % MAP_WIDTH

                    if target["position_x"] > MAP_WIDTH:
                        target["position_x"] -= MAP_WIDTH
                    if target["position_y"] > MAP_HEIGHT:
                        target["position_y"] -= MAP_HEIGHT

                screen.fill(BLACK)
                screen.blit(map_image, (map_x, map_y))
                if map_x > 0:
                    screen.blit(map_image, (map_x - MAP_WIDTH, map_y))
                elif map_x < SCREEN_WIDTH - MAP_WIDTH:
                    screen.blit(map_image, (map_x + MAP_WIDTH, map_y))

                for target in targets:
                    x = target["position_x"] + map_x
                    while x > MAP_WIDTH - Rozmiar_balona_x: x -= MAP_WIDTH
                    while x < -Rozmiar_balona_x: x += MAP_WIDTH
                    screen.blit(target_image, (x, target["position_y"]))

                mouse_x, mouse_y = pygame.mouse.get_pos()
                screen.blit(crosshair_image, (mouse_x - crosshair_image.get_width() / 2, mouse_y - crosshair_image.get_height() / 2))

                pygame.draw.rect(screen, BLACK, (0, 0, 300, 200))

                entry_surface = leaderboard_font.render("Player Leaderboard: ", True, WHITE)
                screen.blit(entry_surface, (10, 0))
                try:
                    part1 = leaderboard[0][:12]

                    entry_surface = leaderboard_font.render(str(part1), True, colors[0])
                    screen.blit(entry_surface, (10, 40))

                    part2 = leaderboard[0][12:24]

                    entry_surface = leaderboard_font.render(str(part2), True, colors[1])
                    screen.blit(entry_surface, (10, 40 * 2))

                    part3 = leaderboard[0][24:38]

                    entry_surface = leaderboard_font.render(str(part3), True, colors[2])
                    screen.blit(entry_surface, (10, 40 * 3))

                    part4 = leaderboard[0][36:49]

                    entry_surface = leaderboard_font.render(str(part4), True, colors[3])
                    screen.blit(entry_surface, (10, 40 * 4))
                except Exception as e:
                    print(f"Error rendering leaderboard")





            elif not game_ended and not game_started:
                    screen.fill(BLACK)
                    text = font.render("Oczekiwanie na wszystkich graczy...", True, WHITE)
                    text_rect = text.get_rect(center=(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2))
                    screen.blit(text, text_rect)

            pygame.display.flip()

    except ConnectionRefusedError:
        print("Unable to connect to the server.")
    finally:
        client_socket.close()
        pygame.quit()
        sys.exit()


def receive_message(client_socket):
    try:
        server_response = client_socket.recv(BUFFER_SIZE)
        if not server_response or len(server_response) != BUFFER_SIZE:
            return None
        content, control_sum = struct.unpack("256si", server_response)
        if control_sum == 0:
            control_sum = (sum(ord(char) for char in str(content)) % 256)//2+1

        content = content.decode().rstrip('\x00')
        i = 0
        while content[i] not in ('t','l','s','d','e','P','1','2','3'):
            i+=1
        content = content[i:]
        print("content:",content)
        print(Message(content).control_sum)
        print(control_sum)

        if Message(content).control_sum != control_sum:
            print("Data has somehow been corrupted.")
            return None

        return content
    except UnicodeDecodeError:
        print("UnicodeDecodeError occurred. Skipping the corrupted message.")
        return None
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return None


def receive_leaderboard(client_socket):
    sentence = ""
    num_entries_data = client_socket.recv(256)
    print("data:",num_entries_data)
    i=0
    while chr(num_entries_data[i]) != 'P' and num_entries_data[i] != '\0':
        i += 1
    num_entries_data = num_entries_data[i:]
    num_entries_data = num_entries_data.decode().rstrip('\x00')

    a = 0
    while a < len(num_entries_data)-1:
        sentence = sentence + num_entries_data[a]
        a += 1
    print("data sentence:", sentence)

    leaderboard = [sentence]

    return leaderboard


if __name__ == "__main__":
    main()
