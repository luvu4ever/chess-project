#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "board.h"
#include "menu.h"
#include "signal.h"

int loggedIn = 0;

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    setlocale(LC_ALL, "en_US.UTF-8");
    char buffer[64];

    if (argv[2] == NULL)
    {
        portno = 80;
    }
    else
    {
        portno = atoi(argv[2]);
    }

    printf("Connecting to %s:%d\n", argv[1], portno);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }

    /* Now ask for a message from the user, this message
     * will be read by server
     */
    while (1)
    {
        if (loggedIn == 0)
        {
            menuLogin(&sockfd);
        }
        else if (loggedIn == 1)
        {
            menuGame(&sockfd);
        }
        else if (loggedIn == 2)
        {
            menuOnRoom(&sockfd);
        }
        else if (loggedIn == 3)
        {
            pthread_t tid[1];

            pthread_create(&tid[0], NULL, &on_signal, &sockfd);

            while (1)
            {
                bzero(buffer, 64);
                fgets(buffer, 64, stdin);

                /* Send message to the server */
                n = write(sockfd, buffer, strlen(buffer));

                if (n < 0)
                {
                    perror("ERROR writing to socket");
                    exit(1);
                }
            }
        }
    }

    return 0;
}
