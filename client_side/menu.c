#include "menu.h"

extern int loggedIn;

void menuLogin(void *sockfd)
{
    int mainChoice;
    char username[24], password[24];
    char buffer[4];
    int n;
    int socket = *(int *)sockfd;
    while (1)
    {
        printf("Menu:\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Exit\n");
        printf("Select: ");
        scanf("%d", &mainChoice);
        switch (mainChoice)
        {
        case 1:
            n = write(socket, "login", 5);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            printf("Input user name: ");
            scanf("%s", username);
            printf("Input password: ");
            scanf("%s", password);
            strcat(username, " ");
            strcat(username, password);
            n = write(socket, username, strlen(username));
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            while (1)
            {
                bzero(buffer, 4);
                n = read(socket, buffer, 4);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                if (buffer[0] == 't')
                {
                    printf("Login success\n");
                    loggedIn = 1;
                    return;
                }
                else if (buffer[0] == 'f')
                {
                    printf("Login false\n");
                    break;
                }
            }
            break;
        case 2:
            n = write(socket, "register", 8);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            printf("Input user name: ");
            scanf("%s", username);
            printf("Input password: ");
            scanf("%s", password);
            strcat(username, " ");
            strcat(username, password);
            n = write(socket, username, strlen(username));
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            while (1)
            {
                bzero(buffer, 4);
                n = read(socket, buffer, 4);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                if (buffer[0] == 't')
                {
                    printf("Register success\n");
                    break;
                }
                else if (buffer[0] == 'f')
                {
                    printf("Register false\n");
                    break;
                }
            }
            break;
        case 3:
            n = write(socket, "exit", 4);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            printf("Exit Success.\n");
            exit(1);
        default:
            printf("Reselect.\n");
            break;
        }
    }
}

void menuGame(void *sockfd)
{
    int mainChoice;
    char buffer[8];
    char invite[6];
    char username[24];
    char oldPassword[24], newPassword[24];
    int n, check;
    int socket = *(int *)sockfd;
    while (1)
    {
        printf("Menu:\n");
        printf("1. Create Room\n");
        printf("2. Waiting\n");
        printf("3. Change Password\n");
        printf("4. Logout\n");
        printf("Select: ");
        scanf("%d", &mainChoice);
        switch (mainChoice)
        {
        case 1:
            n = write(socket, "cre-room", 8);
            while (1)
            {
                bzero(buffer, 8);
                n = read(socket, buffer, 8);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                if (strcmp(buffer, "cre-true"))
                {
                    printf("Create room\n");
                    loggedIn = 2;
                    return;
                }
            }
            break;
        case 2:
            n = write(socket, "waitting", 8);
            while (1)
            {
                // Code waiting invite
                bzero(invite, 6);
                n = read(socket, invite, 6);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                if (invite[0] == 'i' && invite[1] == 'n')
                {
                    bzero(username, 24);
                    n = read(socket, username, 24);
                    if (n < 0)
                    {
                        perror("ERROR reading from socket");
                        exit(1);
                    }
                    printf("Player %s invites you into the room\n", username);
                    printf("1. Accept\n");
                    printf("2. Refuses\n");
                    printf("Input Chooses: ");
                    scanf("%d", &check);
                    if (check == 1)
                    {
                        n = write(socket, "accept", 6);
                        if (n < 0)
                        {
                            perror("ERROR writing to socket");
                            exit(1);
                        }
                        n = write(socket, username, strlen(username));
                        if (n < 0)
                        {
                            perror("ERROR writing to socket");
                            exit(1);
                        }
                        loggedIn = 3;
                        return;
                    }
                    else if (check == 2)
                    {
                        n = write(socket, "refuse", 6);
                        if (n < 0)
                        {
                            perror("ERROR writing to socket");
                            exit(1);
                        }
                        n = write(socket, username, strlen(username));
                        if (n < 0)
                        {
                            perror("ERROR writing to socket");
                            exit(1);
                        }
                        break;
                    }
                }
            }
            break;
        case 3:
            n = write(socket, "changepa", 8);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            printf("Input Old Password: ");
            scanf("%s", oldPassword);
            printf("Input New Password: ");
            scanf("%s", newPassword);
            strcat(oldPassword, " ");
            strcat(oldPassword, newPassword);
            n = write(socket, oldPassword, strlen(oldPassword));
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            while (1)
            {
                bzero(buffer, 4);
                n = read(socket, buffer, 4);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                if (buffer[0] == 't')
                {
                    printf("Change Password success\n");
                    break;
                }
                else if (buffer[0] == 'f')
                {
                    printf("Change Password false\n");
                    break;
                }
            }
            break;
        case 4:
            n = write(socket, "log--out", 8);
            while (1)
            {
                bzero(buffer, 8);
                n = read(socket, buffer, 8);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                buffer[8] = '\0';
                printf("%s-%d\n", buffer, strcmp(buffer, "log-true"));
                if (strcmp(buffer, "log-true") == 0)
                {
                    printf("Logout Success\n");
                    loggedIn = 0;
                    return;
                }
            }
            break;
        default:
            printf("Reselect.\n");
            break;
        }
    }
}

void menuOnRoom(void *sockfd)
{
    int mainChoice;
    char buffer[1024];
    char dataread[7];
    char username[24];
    int n;
    int socket = *(int *)sockfd;
    while (1)
    {
        printf("Menu:\n");
        printf("1. Get User Online\n");
        printf("2. Invite Players\n");
        printf("3. Remove Room\n");
        printf("Select: ");
        scanf("%d", &mainChoice);
        switch (mainChoice)
        {
        case 1:
            n = write(socket, "get-user", 8);
            while (1)
            {
                bzero(buffer, 1024);
                n = read(socket, buffer, 1024);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                printf("User Online:\n");
                printf("%s\n", buffer);
                if (n >= 0)
                {
                    break;
                }
            }
            break;
        case 2:
            n = write(socket, "invite--", 8);
            printf("Input opponent: ");
            scanf("%s", username);
            n = write(socket, username, strlen(username));
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            while (1)
            {
                bzero(dataread, 6);
                n = read(socket, dataread, 6);
                if (n < 0)
                {
                    perror("ERROR reading from socket");
                    exit(1);
                }
                dataread[6] = '\0';
                if (strcmp(dataread, "accept") == 0)
                {
                    printf("The opponent party success\n");
                    n = write(socket, "accept", 6);
                    if (n < 0)
                    {
                        perror("ERROR writing to socket");
                        exit(1);
                    }
                    loggedIn = 3;
                    return;
                }
                else if (strcmp(dataread, "refuse") == 0)
                {
                    printf("The opponent party refuses\n");
                    n = write(socket, "refuse", 6);
                    if (n < 0)
                    {
                        perror("ERROR writing to socket");
                        exit(1);
                    }
                    break;
                }
            }
            break;
        case 3:
            n = write(socket, "remove--", 8);
            printf("Remove Success!\n");
            loggedIn = 1;
            return;
        default:
            printf("Reselect.\n");
            break;
        }
    }
}
