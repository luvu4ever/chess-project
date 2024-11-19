#include <arpa/inet.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "board.h"
#include "check_game.h"
#include "user.h"
#include "globals.h"
#include "logging.h"

#define PORT 8080;

// Match player
int challenging_player = 0;

void handleLogin(int player)
{
    char login[48];
    bzero(login, 48);
    if (read(player, login, 48) < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("Login: %s\n", login);

    char *username = strtok(login, " ");
    char *password = strtok(NULL, " ");

    if (checkLogin(username, password) && checkLogged(username))
    {
        send(player, "t", 1, 0);
        pthread_mutex_lock(&general_mutex);
        strcpy(users[numbers].name, username);
        users[numbers].client_socket = player;
        users[numbers].ongame = false;
        numbers++;
        pthread_mutex_unlock(&general_mutex);
        logLogin(username);
        int waiting = lobby(player, username);
        if (waiting == 1)
        {
            getAllUser(player);
        }
        else if (waiting == 2)
        {
            pthread_mutex_lock(&general_mutex);
            for (int i = 0; i < numbers; i++)
            {
                if (strcmp(users[i].name, username) == 0 && users[i].ongame == false)
                {
                    strcpy(users[i].name, "");
                    users[i].client_socket = -1;
                }
            }
            pthread_mutex_unlock(&general_mutex);
        }
    }
    else
    {
        send(player, "f", 1, 0);
    }
}

// Hàm xử lý đăng ký
void handleRegister(int player)
{
    printf("handleRegister\n");
    char registers[48];
    bzero(registers, 48);
    if (read(player, registers, 48) < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("Register: %s\n", registers);

    char *username = strtok(registers, " ");
    char *password = strtok(NULL, " ");

    if (registerAccount(username, password))
    {
        send(player, "t", 1, 0);
    }
    else
    {
        send(player, "f", 1, 0);
    }
}

// Hàm xử lý thoát
void handleExit(int player)
{
    printf("Client %d exited.\n", player);
}

void *user(void *client_socket)
{
    int player = *(int *)client_socket;

    while (1)
    {
        char userchoose[10];
        bzero(userchoose, 10);
        if (read(player, userchoose, 10) < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        // Sử dụng switch-case để xử lý
        if (strcmp(userchoose, "exit") == 0)
            break;
        switch (userchoose[0])
        {
        case 'l': // login
            handleLogin(player);
            break;
        case 'r': // register
            handleRegister(player);
            break;
        default:
            send(player, "invalid", 7, 0);
            break;
        }
    }
    handleExit(player);
    return NULL;
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");

    int sockfd, client_socket, port_number, client_length;
    char buffer[64];
    struct sockaddr_in server_address, client;
    int n;

    // Conditional variable
    pthread_mutex_init(&general_mutex, NULL);

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *)&server_address, sizeof(server_address));
    port_number = PORT;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    /* MAX_QUEUE */
    listen(sockfd, 20);
    printf("Server listening on port %d\n", port_number);

    while (1)
    {
        client_length = sizeof(client);
        // CHECK IF WE'VE A WAITING USER

        /* Accept actual connection from the client */
        client_socket = accept(sockfd, (struct sockaddr *)&client, (unsigned int *)&client_length);
        printf("– Connection accepted from %d at %d.%d.%d.%d:%d –\n", client_socket, client.sin_addr.s_addr & 0xFF, (client.sin_addr.s_addr & 0xFF00) >> 8, (client.sin_addr.s_addr & 0xFF0000) >> 16, (client.sin_addr.s_addr & 0xFF000000) >> 24, client.sin_port);

        if (client_socket < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }
        pthread_t thread[1];
        pthread_create(&thread[0], NULL, &user, &client_socket);
    }
    close(sockfd);
    return 0;
}
