#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "client_menu.h"

#define BUFFER_SIZE 256

extern int loggedIn;

volatile int invite_processing = 0;

void display_login_menu(void *sockfd)
{
    char *choices[] = {
        "Login",
        "Register",
        "Exit"};
    int num_choices = sizeof(choices) / sizeof(char *);
    int socket = *(int *)sockfd;
    handle_menu_choice(socket, choices, num_choices, handle_login_menu_selection);
}

void handle_login_menu_selection(int choice, int sock)
{
    switch (choice)
    {
    case 0:
        handle_login(sock);
        break;
    case 1:
        handle_register(sock);
        break;
    case 2:
        handle_exit(sock);
        exit(0);
        break;
    default:
        break;
    }
}

void handle_login(int socket)
{
    char username[50];
    char password[50];
    char buffer[BUFFER_SIZE];

    int n = write(socket, "login", 5);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Clear screen and prompt user for login information
    clear();
    echo();
    mvprintw(1, 2, "Enter Username: ");
    getstr(username);
    mvprintw(2, 2, "Enter Password: ");
    getstr(password);
    noecho();

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
            mvprintw(4, 2, "Login successful. Press any key to continue to the main menu...");
            getch();
            clear();
            display_main_menu(socket);

            loggedIn = 1;
        }
        else if (buffer[0] == 'f')
        {
            mvprintw(4, 2, "Login failed: %s", buffer);
            mvprintw(5, 2, "Press any key to try again...");
            getch();
            clear();
            display_login_menu(&socket); // Retry login
        }
    }
}

void handle_register(int socket)
{
    char username[50];
    char password[50];
    char buffer[BUFFER_SIZE];
    int n;

    // Gửi yêu cầu "register" đến server
    n = write(socket, "register", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Hiển thị giao diện nhập thông tin
    clear();
    echo();
    mvprintw(1, 2, "Enter Username: ");
    getstr(username);
    mvprintw(2, 2, "Enter Password: ");
    getstr(password);
    noecho();

    // Gửi thông tin đăng ký tới server
    strcat(username, " ");
    strcat(username, password);
    n = write(socket, username, strlen(username));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Chờ phản hồi từ server
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(socket, buffer, sizeof(buffer));
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (buffer[0] == 't') // Đăng ký thành công
        {
            mvprintw(4, 2, "Registration successful! Press any key to return to the login menu...");
            getch();
            clear();
            display_login_menu(&socket); // Quay lại menu đăng nhập
            return;                      // Kết thúc hàm
        }
        else if (buffer[0] == 'f') // Đăng ký thất bại
        {
            mvprintw(4, 2, "Registration failed: %s", buffer);
            mvprintw(5, 2, "Press any key to try again...");
            getch();
            clear();
            display_login_menu(&socket); // Quay lại menu đăng ký
            return;                      // Kết thúc hàm
        }
    }
}

void handle_exit(int socket)
{
    int n;

    n = write(socket, "exit", 4);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    printf("Exit Success.\n");
    exit(1);
}

void display_main_menu(int sock)
{

    char *choices[] = {
        "Create Room",
        "Waiting",
        "Change Password",
        "Logout"};
    int num_choices = sizeof(choices) / sizeof(char *);
    handle_menu_choice(sock, choices, num_choices, handle_main_menu_selection);
}

void handle_main_menu_selection(int choice, int sock)
{
    switch (choice)
    {
    case 0:
        handle_create_room(sock);
        break;
    case 1:
        handle_waiting(sock);
        break;
    case 2:
        handle_change_password(sock);
        break;
    case 3:
        handle_logout(sock);
        break;
    case 4:
        // Implement help function
        break;
    case 5:
        endwin();
        exit(0);
        break;
    default:
        break;
    }
}

void handle_create_room(int socket)
{
    char buffer[8];
    int n;

    n = write(socket, "cre-room", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    while (1)
    {
        bzero(buffer, 8);
        n = read(socket, buffer, 8);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (strcmp(buffer, "cre-true") == 0)
        {
            printf("Create room\n");
            loggedIn = 2;
            return;
        }
    }
}

void handle_waiting(int socket)
{
    char invite[6];
    char username[24];
    int n, check;

    n = write(socket, "waitting", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    while (1)
    {
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
}

void handle_change_password(int socket)
{
    char oldPassword[50];
    char newPassword[50];
    char buffer[BUFFER_SIZE];
    int n;

    // Gửi yêu cầu "changepa" tới server
    n = write(socket, "changepa", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Hiển thị giao diện nhập mật khẩu cũ và mới
    clear();
    echo();
    mvprintw(1, 2, "Enter Old Password: ");
    getstr(oldPassword);
    mvprintw(2, 2, "Enter New Password: ");
    getstr(newPassword);
    noecho();

    // Chuẩn bị chuỗi gửi tới server
    strcat(oldPassword, " ");
    strcat(oldPassword, newPassword);

    n = write(socket, oldPassword, strlen(oldPassword));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Chờ phản hồi từ server
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        n = read(socket, buffer, sizeof(buffer));
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (buffer[0] == 't') // Đổi mật khẩu thành công
        {
            mvprintw(4, 2, "Password changed successfully! Press any key to return to the main menu...");
            getch();
            clear();
            display_main_menu(&socket); // Quay lại menu chính
            return;
        }
        else if (buffer[0] == 'f') // Đổi mật khẩu thất bại
        {
            mvprintw(4, 2, "Password change failed: %s", buffer);
            mvprintw(5, 2, "Press any key to try again...");
            getch();
            clear();
            display_main_menu(&socket); // Quay lại menu đổi mật khẩu
            return;
        }
    }
}

void handle_logout(int socket)
{
    char buffer[8];
    int n;

    n = write(socket, "log--out", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

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
}

void display_room_menu(int sock)
{
    char *choices[] = {
        "Get User Online",
        "Invite Players",
        "Remove Room"};
    int num_choices = sizeof(choices) / sizeof(char *);
    handle_menu_choice(sock, choices, num_choices, handle_room_menu_selection);
}

void handle_room_menu_selection(int choice, int socket)
{
    switch (choice)
    {
    case 1:
        handle_get_user_online(socket);
        break;
    case 2:
        handle_invite_players(socket);
        break;
    case 3:
        handle_remove_room(socket);
        break;
    default:
        printf("Invalid choice. Please select again.\n");
        break;
    }
}

void handle_get_user_online(int socket)
{
    char buffer[1024];
    int n = write(socket, "get-user", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

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
}

void handle_invite_players(int socket)
{
    char username[50];
    char dataread[7];
    char buffer[BUFFER_SIZE];
    int n;

    // Gửi yêu cầu "invite--" tới server
    n = write(socket, "invite--", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Hiển thị giao diện nhập tên đối thủ
    clear();
    echo();
    mvprintw(1, 2, "Enter opponent's username: ");
    getstr(username);
    noecho();

    // Gửi tên đối thủ tới server
    n = write(socket, username, strlen(username));
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    // Chờ phản hồi từ server
    while (1)
    {
        memset(dataread, 0, sizeof(dataread));
        n = read(socket, dataread, sizeof(dataread) - 1);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }
        dataread[n] = '\0';

        if (strcmp(dataread, "accept") == 0) // Đối thủ chấp nhận
        {
            mvprintw(3, 2, "The opponent accepted the invite!");
            mvprintw(4, 2, "Press any key to proceed...");
            getch();
            loggedIn = 3; // Cập nhật trạng thái đăng nhập
            clear();
            return;
        }
        else if (strcmp(dataread, "refuse") == 0) // Đối thủ từ chối
        {
            mvprintw(3, 2, "The opponent refused the invite.");
            mvprintw(4, 2, "Press any key to try again...");
            getch();
            clear();
            return;
        }
    }
}

void handle_remove_room(int socket)
{
    int n = write(socket, "remove--", 8);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

    printf("Room removed successfully!\n");
    loggedIn = 1; // Quay lại trạng thái trước khi vào phòng
}

void handle_menu_choice(int sock, char *choices[], int num_choices, void (*handle_selection)(int, int))
{
    int highlight = 0;
    int choice = 0;
    int c;

    initscr(); // Start ncurses mode
    clear();
    noecho();
    cbreak();
    curs_set(0);          // Hide the cursor
    keypad(stdscr, TRUE); // Enable keypad input for arrow keys

    // Create the menu loop
    while (1)
    {
        // if (invite_processing)
        // {
        //     continue;
        // }
        for (int i = 0; i < num_choices; ++i)
        {
            if (i == highlight)
            {
                attron(A_REVERSE); // Highlight the current choice
                mvprintw(i + 1, 2, choices[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(i + 1, 2, choices[i]);
            }
        }
        c = getch();

        switch (c)
        {
        case KEY_UP:
        case 'w':
        case 'W':
            highlight = (highlight - 1 + num_choices) % num_choices;
            break;
        case KEY_DOWN:
        case 's':
        case 'S':
            highlight = (highlight + 1) % num_choices;
            break;
        case 10: // Enter key
            choice = highlight;
            handle_selection(choice, sock);
            break;
        default:
            break;
        }
    }
}
