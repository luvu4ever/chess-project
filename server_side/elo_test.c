#include <stdio.h>
#include "elo.h"
#include "globals.h"

int main()
{
    // Kết quả trận đấu: 0 = hòa, 1 = Player1 thắng, 2 = Player2 thắng
    handleCalculateElo(0, 1, 1); // Alice thắng Bob
    handleCalculateElo(0, 2, 0); // Alice hòa Charlie
    handleCalculateElo(1, 2, 2); // Charlie thắng Bob

    return 0;
}
