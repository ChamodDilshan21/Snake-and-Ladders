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

// ---------------------------------------CHECK IS FLAG REACHERABLE---------------------------------------
bool isFlagReachable(CellCord start)
{
    // BFS queue: store CellCord
    CellCord queue[1000]; // Fixed size for simplicity; adjust if needed
    int front = 0, rear = 0;
    bool visited[FLOORS][WIDTH][LENGTH] = {false};

    queue[rear++] = start;
    visited[start.floor][start.width][start.length] = true;

    while (front < rear)
    {
        CellCord curr = queue[front++];

        if (isSameCord(curr, Flag))
        {
            return true; // Flag reached
        }

        // Check neighbors: N, E, S, W
        Direction dirs[] = {NORTH, EAST, SOUTH, WEST};
        for (int d = 0; d < 4; d++)
        {
            CellCord next = getNextCellCoord(curr, dirs[d]);
            if (isValidCordinates(next) && !isBlockedCell(next) && !visited[next.floor][next.width][next.length])
            {
                visited[next.floor][next.width][next.length] = true;
                queue[rear++] = next;
            }
        }

        // Handle stairs if on a stair cell
        struct Cell cell = maze[curr.floor][curr.width][curr.length];
        if (cell.cellType == STAIR_CELL)
        {
            CellCord stairEnd = curr; // Temp
            if (takeStair(&stairEnd, cell.cellTypeId))
            { // Reuse takeStair to get end
                if (!visited[stairEnd.floor][stairEnd.width][stairEnd.length])
                {
                    visited[stairEnd.floor][stairEnd.width][stairEnd.length] = true;
                    queue[rear++] = stairEnd;
                }
            }
        }

        // Handle poles if on a pole cell (down only, as per game logic)
        if (cell.cellType == POLE_CELL)
        {
            CellCord poleStart = curr; // Temp
            if (takePole(&poleStart, cell.cellTypeId))
            {
                if (!visited[poleStart.floor][poleStart.width][poleStart.length])
                {
                    visited[poleStart.floor][poleStart.width][poleStart.length] = true;
                    queue[rear++] = poleStart;
                }
            }
        }
    }
    return false; // Flag not reachable
}

// ---------------------------------------CHECK IS FLAG REACHERABLE---------------------------------------
int main()
{
    // write errors into log.txt file
    freopen("log.txt", "a", stderr);
    fprintf(stderr, "\n---------------------------------------------------------------------------------------------------------------------\n\n");
    fflush(stderr);

    // game start here
    printf("\n\t\t   ___   _   __  __ ___   ___ ___ ___ ___ _  _ ___ _ \r\n\t\t  / __| /_\\ |  \\/  | __| | _ ) __/ __|_ _| \\| / __| |\r\n\t\t | (_ |/ _ \\| |\\/| | _|  | _ \\ _| (_ || || .` \\__ \\_|\r\n\t\t  \\___/_/ \\_\\_|  |_|___| |___/___\\___|___|_|\\_|___(_)\r\n                                                     \n");

    loadSeed();
    intializeMaze();
    for (int i = 0; i < NO_PLAYERS; i++)
    {
        if (!isFlagReachable(players[i].startCell))
        { // Check from starting cells
            printf("\nError: Flag is unreachable from player %c's starting position. Quitting Game....\n", players[i].name);
            exit(1);
        }
    }
    initPlayers();

    while (true)
    {
        printf("\n \tRound %d \n", gameRound + 1);
        printf(" ===================== \n");
        for (int i = 0; i < NO_PLAYERS; i++)
        {
            printf("\n----%c's turn:----\n", players[i].name);
            playerTurn(&players[i]);
        }

        gameRound++;
        if (gameRound % 5 == 0)
        {
            printf("\n \\\\---Five rounds has passed. The direction of the stairs change randomly.---\\\\ \n");
            changeStairDirection();
        }
    }

    return 0;
}