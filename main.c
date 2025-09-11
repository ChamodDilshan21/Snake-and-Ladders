#include "maze.h"

// ----------------------------------------GLOBAL VARIABLES---------------------------------------
struct Cell maze[FLOORS][WIDTH][LENGTH];

CellCord Flag;

struct Stair *stairs = NULL;
struct Pole *poles = NULL;
struct Wall *walls = NULL;
struct BawanaCell *bawanaCells = NULL;

int stairsCount = 0;
int polesCount = 0;
int wallsCount = 0;
int bawanaCellCount = 0;

void printFloors()
{
    for (int f = 0; f < FLOORS; f++)
    {
        printf("\n\n======================================= Floor %d =======================================\n\n", f);
        for (int w = 0; w < WIDTH; w++)
        {
            for (int l = 0; l < LENGTH; l++)
            {
                char symbol1 = ' ';
                switch (maze[f][w][l].cellType)
                {
                case ACTIVE_CELL:
                    symbol1 = 'A';
                    break;
                case STAIR_CELL:
                    symbol1 = 'S';
                    break;
                case POLE_CELL:
                    symbol1 = 'P';
                    break;
                case WALL_CELL:
                    symbol1 = 'W';
                    break;
                case FLAG_CELL:
                    symbol1 = 'F';
                    break;
                case STARTING_AREA_CELL:
                    symbol1 = '*';
                    break;
                case BAWANA_CELL:
                    symbol1 = 'B';
                    break;
                case BAWANA_ENTRY:
                    symbol1 = 'E';
                    break;
                case EMPTY_CELL:
                    symbol1 = 'x';
                    break;
                }

                char symbol2 = ' ';
                int mp = 0;
                if (maze[f][w][l].cellType == BAWANA_CELL)
                {
                    switch (bawanaCells[maze[f][w][l].cellTypeId].type)
                    {
                    case POISONED_CELL:
                        symbol2 = 'p';
                        break;

                    case DISORIENTED_CELL:
                        symbol2 = 'd';
                        break;

                    case TRIGGERED_CELL:
                        symbol2 = 't';
                        break;

                    case HAPPY_CELL:
                        symbol2 = 'h';
                        break;

                    case RANDOM_CELL:
                        symbol2 = 'r';
                        break;
                    }
                    mp = bawanaCells[maze[f][w][l].cellTypeId].movementPoints;
                }
                else
                {
                    switch (maze[f][w][l].effectType)
                    {
                    case MP_NONE:
                        symbol2 = 'n';
                        break;
                    case MP_ADD:
                        symbol2 = '+';
                        break;
                    case MP_CONSUME:
                        symbol2 = '-';
                        break;
                    case MP_MULTIPLY:
                        symbol2 = '*';
                        break;
                    }
                    mp = maze[f][w][l].effectValue;
                }

                printf("%c%c%d ", symbol1, symbol2, mp);
            }
            printf("\n");
        }
    }
}

// --------------------MAIN--------------------
int main()
{
    printf("Game started!..\n");
    loadSeed();
    intializeMaze();
    printFloors();
    return 0;
}