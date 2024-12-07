#ifndef CLIENT_MENU_H
#define CLIENT_MENU_H

#include <stdio.h>  // Các hàm IO cơ bản
#include <stdlib.h> // Hàm exit
#include <string.h> // Hàm xử lý chuỗi
#include <unistd.h> // Hàm read, write

// Hiển thị menu đăng nhập
void display_login_menu(void *sock);

// Hiển thị menu chính
void display_main_menu(int sock);

// Xử lý lựa chọn từ menu chung
void handle_menu_choice(int sock, char *choices[], int num_choices, void (*handle_selection)(int, int));

// Xử lý lựa chọn trong menu chính
void handle_main_menu_selection(int choice, int sock);

// Xử lý lựa chọn trong menu đăng nhập
void handle_login_menu_selection(int choice, int sock);

// Xử lý logic đăng nhập
void handle_login(int socket);

// Xử lý logic đăng ký
void handle_register(int socket);

// Xử lý thoát chương trình
void handle_exit(int socket);

// Xử lý tạo phòng
void handle_create_room(int socket);

// Xử lý trạng thái chờ
void handle_waiting(int socket);

// Xử lý thay đổi mật khẩu
void handle_change_password(int socket);

// Xử lý đăng xuất
void handle_logout(int socket);

void handle_room_menu_selection(int choice, int sock);

#endif // CLIENT_MENU_H
