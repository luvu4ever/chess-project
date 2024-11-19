#ifndef ELO_H
#define ELO_H

// Cấu trúc thông tin người chơi
typedef struct
{
    char name[50];     // Tên người chơi
    char password[50]; // Mật khẩu người chơi
    int gamesPlayed;   // Số trận đã chơi
    double rating;     // Điểm Elo
} Player;

// Hàm tính kỳ vọng thắng (E)
double calculateExpectedScore(double playerRating, double opponentRating);

// Hàm xác định hệ số K
int getKFactor(int gamesPlayed, double playerRating);

// Hàm lấy thông tin người chơi từ tệp
int loadPlayersFromFile(const char *filename, Player players[], int *numPlayers);

// Hàm lưu thông tin người chơi vào tệp
void savePlayersToFile(const char *filename, Player players[], int numPlayers);

// Hàm tính và cập nhật Elo giữa hai người chơi
void handleCalculateElo(int player1Index, int player2Index, int result);

#endif // ELO_H
