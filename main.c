#include "functions.h"

// --------------------GLOBAL VARIABLES--------------------
struct Cell maze[FLOORS][WIDTH][LENGTH];

CellCord Flag;

struct Stair *stairs = NULL;
struct Pole *poles = NULL;
struct Wall *walls = NULL;

int stairsCount = 0;
int polesCount = 0;
int wallsCount = 0;
int seed = 0;

void printFloors()
{
    for (int f = 0; f < FLOORS; f++)
    {
        printf("\n\n======================================= Floor %d =======================================\n\n", f);
        for (int w = 0; w < WIDTH; w++)
        {
            for (int l = 0; l < LENGTH; l++)
            {
                char symbol = ' ';
                switch (maze[f][w][l].cellType)
                {
                case ACTIVE_CELL:
                    symbol = 'A';
                    break;
                case STAIR_CELL:
                    symbol = 'S';
                    break;
                case POLE_CELL:
                    symbol = 'P';
                    break;
                case WALL_CELL:
                    symbol = 'W';
                    break;
                case FLAG_CELL:
                    symbol = 'F';
                    break;
                case STARTING_AREA_CELL:
                    symbol = '*';
                    break;
                case BAWANA_CELL:
                    symbol = 'B';
                    break;
                case BAWANA_ENTRY:
                    symbol = 'E';
                    break;
                case EMPTY_CELL:
                    symbol = 'x';
                    break;
                }
                printf("%c ", symbol);
            }
            printf("\n");
        }
    }
}

// --------------------MAIN--------------------
int main()
{
    printf("Game started!..\n");
    initMaze(maze);
    loadFiles();
    printFloors();
    return 0;
}