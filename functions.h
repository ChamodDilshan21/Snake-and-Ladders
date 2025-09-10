#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "helpers.h"
#include <stdio.h>

// ----------------------------------------GLOBAL VARIABLES----------------------------------------
extern int seed;

// ----------------------------------------INITIALIZE MAZE FLOORS----------------------------------------
void initMaze(struct Cell maze[FLOORS][WIDTH][LENGTH])
{
    // initialize all cells as empty cells
    for (int f = 0; f < FLOORS; f++)
    {
        for (int w = 0; w < WIDTH; w++)
        {
            for (int l = 0; l < LENGTH; l++)
            {
                maze[f][w][l].cellType = EMPTY_CELL;
                maze[f][w][l].cellTypeId = -1;
                maze[f][w][l].effectType = MP_NONE;
                maze[f][w][l].effectValue = 0;
            }
        }
    }

    // setup game cells in ground floor
    for (int w = 0; w < 10; w++)
    {
        for (int l = 0; l < 25; l++)
        {
            maze[0][w][l].cellType = ACTIVE_CELL;
        }
    }
    // marking starting-area
    for (int w = 6; w <= 9; w++)
    {
        for (int l = 8; l <= 16; l++)
        {
            maze[0][w][l].cellType = STARTING_AREA_CELL;
        }
    }
    // marking bawana
    int id = 0;
    for (int w = 6; w <= 9; w++)
    {
        for (int l = 20; l <= 24; l++)
        {
            if (w == 6 || l == 20)
            {
                maze[0][w][l].cellType = WALL_CELL;
                continue;
            }
            maze[0][w][l].cellType = BAWANA_CELL;
            maze[0][w][l].cellTypeId = id++;
        }
    }
    // marking bawana entry
    maze[0][9][19].cellType = BAWANA_ENTRY;

    // setup game cells in first floor
    for (int w = 0; w < WIDTH; w++)
    {
        for (int l = 0; l < LENGTH; l++)
        {
            if (w >= 0 && w <= 5)
            {
                if (l >= 8 && l <= 16)
                {
                    continue;
                }
            }
            maze[1][w][l].cellType = ACTIVE_CELL;
        }
    }

    // setup game cells in second floor
    for (int w = 0; w < WIDTH; w++)
    {
        for (int l = 0; l <= 8; l++)
        {
            maze[2][w][l].cellType = ACTIVE_CELL;
        }
    }
}

// ----------------------------------------LOAD FILES----------------------------------------
void loadSeed()
{
    FILE *file = fopen("seed.txt", "r");
    if (!file)
    {
        printf("Error opening seed.txt\n");
        exit(1);
    }
    fscanf(file, "%d", &seed);
    fclose(file);
    srand(seed);
}

void loadStairs()
{
    FILE *file = fopen("stairs.txt", "r");
    if (!file)
    {
        printf("Error opening stairs.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    stairs = (struct Stair *)malloc(capacity * sizeof(struct Stair));
    if (!stairs)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    struct Stair tempStair;
    while (fscanf(file, " [%d, %d, %d, %d, %d, %d] ",
                  &tempStair.startFloor,
                  &tempStair.startBlockWidth,
                  &tempStair.startBlockLength,
                  &tempStair.endFloor,
                  &tempStair.endBlockWidth,
                  &tempStair.endBlockLength) == 6)
    {
        if (!isValidStair(tempStair))
        {
            printf("Error: Invalid coordinates for a stair detected from the stairs.txt...skipped that coordinates \n");
            continue;
        }
        tempStair.stairId = count;
        stairs[count++] = tempStair;
        if (count >= capacity)
        {
            capacity *= 2;
            stairs = (struct Stair *)realloc(stairs, capacity * sizeof(struct Stair));
            if (!stairs)
            {
                printf("Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        printf("Error: No valid stairs were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    stairsCount = count;
    addStairsToMaze();
}

void loadPoles()
{
    FILE *file = fopen("poles.txt", "r");
    if (!file)
    {
        printf("Error opening poles.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    poles = (struct Pole *)malloc(capacity * sizeof(struct Pole));
    if (!poles)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    struct Pole tempPole;
    while (fscanf(file, " [%d, %d, %d, %d] ",
                  &tempPole.startFloor,
                  &tempPole.endFloor,
                  &tempPole.widthCell,
                  &tempPole.lengthCell) == 4)
    {
        if (!isValidPole(tempPole))
        {
            printf("Error: Invalid coordinates for a pole detected from the poles.txt...skipped that coordinates \n");
            continue;
        }
        tempPole.poleId = count;
        poles[count++] = tempPole;
        if (count >= capacity)
        {
            capacity *= 2;
            poles = (struct Pole *)realloc(poles, capacity * sizeof(struct Pole));
            if (!poles)
            {
                printf("Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        printf("No valid poles were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    polesCount = count;
    addPolesToMaze();
}

void loadFlag()
{
    FILE *file = fopen("flag.txt", "r");
    if (!file)
    {
        printf("Error opening flag.txt\n");
        exit(1);
    }
    CellCord flagPosition;
    if (fscanf(file, " [%d, %d, %d] ", &flagPosition.floor, &flagPosition.width, &flagPosition.length) != 3)
    {
        printf("Error: Invalid flag format in flag.txt\n");
        fclose(file);
        exit(1);
    }
    if (!isValidCell(flagPosition))
    {
        printf("Error: Invalid Flag location....flag location set to [0, 0, 0]\n");
        Flag.floor = 0;
        Flag.width = 0;
        Flag.length = 0;
    }
    else
    {
        Flag = flagPosition;
    }
    fclose(file);
    addFlagToMaze();
}

void loadWalls()
{
    FILE *file = fopen("walls.txt", "r");
    if (!file)
    {
        printf("Error opening walls.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    walls = (struct Wall *)malloc(capacity * sizeof(struct Wall));
    if (!walls)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    struct Wall tempWall;
    while (fscanf(file, " [%d, %d, %d, %d, %d] ",
                  &tempWall.floor,
                  &tempWall.startBlockWidth,
                  &tempWall.startBlockLength,
                  &tempWall.endBlockWidth,
                  &tempWall.endBlockLength) == 5)
    {
        if (!isValidWall(tempWall))
        {
            printf("Error: Invalid coordinates for a wall detected from the walls.txt...skipped that coordinates \n");
            continue;
        }
        walls[count++] = tempWall;
        if (count >= capacity)
        {
            capacity *= 2;
            walls = (struct Wall *)realloc(walls, capacity * sizeof(struct Wall));
            if (!walls)
            {
                printf("Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        printf("Error: No valid walls were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    wallsCount = count;
    addWallstoMaze();
}

// ----------------------------------------CALLING FUNCTIONS----------------------------------------
void loadFiles()
{
    loadWalls();
    loadStairs();
    loadPoles();
    loadFlag();
    loadSeed();
}

#endif