#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5000

// to jest useless jak coś

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    // Tworzenie gniazda klienta
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    // Konfiguracja struktury adresu serwera
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Konwersja adresu IP z kropkowymi dziesiatnymi na binarne
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Nawiazanie polaczenia z serwerem
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Wyslanie wiadomosci do serwera
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent to server\n");

    // Odbior odpowiedzi od serwera
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);

    return 0;
}
