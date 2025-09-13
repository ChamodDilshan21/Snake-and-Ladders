#include "maze.h"
#include "play.h"

// ----------------------------------------GLOBAL VARIABLES---------------------------------------

// constants
const CellCord BawanaEntry = {0, 9, 19};

const CellCord specialCells[] = {BawanaEntry, {0, 6, 12}, {0, 5, 12}, {0, 9, 8}, {0, 9, 7}, {0, 9, 16}, {0, 9, 17}};

const char *stringDirections[] = {"NORTH", "EAST", "SOUTH", "WEST", "EMPTY"};

const char *stringBawanaEffects[] = {"POISONED", "DISORIENTED", "TRIGGERED", "HAPPY", "RANDOM"};

// arrays
struct Cell maze[FLOORS][WIDTH][LENGTH];
Player players[NO_PLAYERS];

CellCord Flag;

// array pointers
struct Stair *stairs = NULL;
struct Pole *poles = NULL;
struct Wall *walls = NULL;
struct BawanaCell *bawanaCells = NULL;

// counters
int no_Stairs = 0;
int no_Poles = 0;
int no_Walls = 0;
int no_BawanaCells = 0;

int gameRound = 0;

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
    freopen("log.txt", "a", stderr);

    printf("\n   ___   _   __  __ ___   ___ ___ ___ ___ _  _ ___ _ \r\n  / __| /_\\ |  \\/  | __| | _ ) __/ __|_ _| \\| / __| |\r\n | (_ |/ _ \\| |\\/| | _|  | _ \\ _| (_ || || .` \\__ \\_|\r\n  \\___/_/ \\_\\_|  |_|___| |___/___\\___|___|_|\\_|___(_)\r\n                                                     \n");
    intializeMaze();
    initPlayers();

    printFloors();

    while (true)
    {
        printf("\n \tRound %d \n", gameRound + 1);
        printf(" ===================== \n");
        for (int i = 0; i < NO_PLAYERS; i++)
        {
            printf("\n----%c's turn:----\n", players[i].name);
            // printf("----------- \n");
            playerTurn(&players[i]);
        }

        gameRound++;
        if (gameRound % 5 == 0)
        {
            printf("\n \\\\---Five rounds has passed. The direction of the stairs change randomly.---\\\\ \n");
            changeStairDirection();
        }

        // if (gameRound == 100)
        // {
        //     exit(0);
        // }
    }

    // \t -> skip 8 chars
    // printf("\n \t\tRound %d \n", gameRound + 1);
    // printf(" \t===================== \n\n");
    // printf("%c's turn:\n", players[0].name);
    // printf("----------- \n\n");

    return 0;
}