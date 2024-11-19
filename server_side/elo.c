#include <stdio.h>
#include <string.h>
#include <math.h>
#include "elo.h"
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

// Hệ số K dựa trên cấp độ người chơi
const int K_NEW_PLAYER = 40;
const int K_INTERMEDIATE = 20;
const int K_HIGH_RATED = 10;

// Hàm tính kỳ vọng thắng (E)
double calculateExpectedScore(double playerRating, double opponentRating)
{
    return 1.0 / (1.0 + pow(10.0, (opponentRating - playerRating) / 400.0));
}

// Hàm xác định hệ số K
int getKFactor(int gamesPlayed, double playerRating)
{
    if (gamesPlayed < 30)
    {
        return K_NEW_PLAYER;
    }
    else if (playerRating < 2400)
    {
        return K_INTERMEDIATE;
    }
    else
    {
        return K_HIGH_RATED;
    }
}

// Hàm lấy thông tin người chơi từ tệp
int loadPlayersFromFile(const char *filename, Player players[], int *numPlayers)
{
    FILE *file = fopen("user.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    int count = 0;
    while (fscanf(file, "%s %s %d %lf", players[count].name, players[count].password,
                  &players[count].gamesPlayed, &players[count].rating) == 4)
    {
        count++;
    }

    fclose(file);
    *numPlayers = count;
    return 1;
}

// Hàm lưu thông tin người chơi vào tệp
void savePlayersToFile(const char *filename, Player players[], int numPlayers)
{
    FILE *file = fopen("user.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
    }

    for (int i = 0; i < numPlayers; i++)
    {
        fprintf(file, "%s %s %d %.2f\n", players[i].name, players[i].password,
                players[i].gamesPlayed, players[i].rating);
    }

    fclose(file);
}

// Hàm tính và cập nhật Elo giữa hai người chơi
void handleCalculateElo(int player1Index, int player2Index, int result)
{
    Player players[100]; // Giới hạn tối đa 100 người chơi
    int numPlayers = 0;

    // Đọc danh sách người chơi từ tệp
    if (!loadPlayersFromFile("user.txt", players, &numPlayers))
    {
        printf("Không thể tải danh sách người chơi.\n");
        return;
    }

    // Kiểm tra hợp lệ của số thứ tự người chơi
    if (player1Index < 0 || player1Index >= numPlayers ||
        player2Index < 0 || player2Index >= numPlayers)
    {
        printf("Số thứ tự người chơi không hợp lệ.\n");
        return;
    }

    // Lấy người chơi theo số thứ tự
    Player *player1 = &players[player1Index];
    Player *player2 = &players[player2Index];

    // Tính điểm kỳ vọng
    double player1Expected = calculateExpectedScore(player1->rating, player2->rating);
    double player2Expected = calculateExpectedScore(player2->rating, player1->rating);

    // Lấy hệ số K
    int player1K = getKFactor(player1->gamesPlayed, player1->rating);
    int player2K = getKFactor(player2->gamesPlayed, player2->rating);

    // Kết quả thực tế
    double player1Result = result == 1 ? 1.0 : (result == 0 ? 0.5 : 0.0);
    double player2Result = 1.0 - player1Result;

    // Cập nhật điểm Elo
    player1->rating += player1K * (player1Result - player1Expected);
    player2->rating += player2K * (player2Result - player2Expected);

    // Tăng số trận đã chơi
    player1->gamesPlayed++;
    player2->gamesPlayed++;

    // Lưu kết quả vào tệp
    savePlayersToFile("user.txt", players, numPlayers);

    printf("Cập nhật Elo thành công:\n");
    printf("%s: %.2f (%d trận)\n", player1->name, player1->rating, player1->gamesPlayed);
    printf("%s: %.2f (%d trận)\n", player2->name, player2->rating, player2->gamesPlayed);
}