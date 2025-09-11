#ifndef MAZE_H
#define MAZE_H

#include "helpers.h"
#include <stdio.h>

// ----------------------------------------INITIALIZE MAZE FLOORS----------------------------------------
void setUpFloors(struct Cell maze[FLOORS][WIDTH][LENGTH])
{
    // set all cells as empty cells
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
    int cap = 12;
    bawanaCells = (struct BawanaCell *)malloc(cap * sizeof(struct BawanaCell));
    if (!bawanaCells)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    for (int w = 6; w <= 9; w++)
    {
        for (int l = 20; l <= 24; l++)
        {
            if (w == 6 || l == 20)
            {
                maze[0][w][l].cellType = WALL_CELL;
                continue;
            }
            if (bawanaCellCount >= 12)
            {
                cap *= 2;
                bawanaCells = (struct BawanaCell *)realloc(bawanaCells, cap * sizeof(struct BawanaCell));
                if (!bawanaCells)
                {
                    fprintf(stderr, "Error: Memory reallocation failed\n");
                    exit(1);
                }
            }
            maze[0][w][l].cellType = BAWANA_CELL;
            maze[0][w][l].cellTypeId = bawanaCellCount;
            bawanaCells[bawanaCellCount] = (struct BawanaCell){bawanaCellCount};
            bawanaCellCount++;
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

// ----------------------------------------ADD MOVEMENT POINTS TO CELLS----------------------------------------
void addMovementPointsToCells()
{
    int total = 0;
    int capacity = 100;
    CellCord *activeCellList = (CellCord *)malloc(capacity * sizeof(CellCord));

    for (int f = 0; f < FLOORS; f++)
    {
        for (int w = 0; w < WIDTH; w++)
        {
            for (int l = 0; l < LENGTH; l++)
            {
                if (maze[f][w][l].cellType == ACTIVE_CELL ||
                    maze[f][w][l].cellType == STAIR_CELL ||
                    maze[f][w][l].cellType == POLE_CELL ||
                    maze[f][w][l].cellType == STARTING_AREA)
                {
                    if (total >= capacity)
                    {
                        capacity *= 2;
                        CellCord *temp = (CellCord *)realloc(activeCellList, capacity * sizeof(CellCord));
                        if (temp == NULL)
                        {
                            fprintf(stderr, "Error: Memory reallocation failed\n");
                            exit(1);
                        }
                    }

                    activeCellList[total++] = (CellCord){f, w, l};
                }
            }
        }
    }

    if (total == 0)
        return;

    int consumeCells = total * 35 / 100;
    int bonus1Cells = total * 25 / 100; // cells with bonuses of 1 or 2
    int bonus2Cells = total * 10 / 100; // cells with bonuses of 3 to 5
    int multplyCells = total * 5 / 100;

    // shuffle cells to randomly add mp values
    for (int i = total - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        CellCord tmp = activeCellList[i];
        activeCellList[i] = activeCellList[j];
        activeCellList[j] = tmp;
    }

    int current_index = 0;

    for (int k = 0; k < consumeCells && current_index < total; k++, current_index++)
    {
        CellCord cell = activeCellList[current_index];
        maze[cell.floor][cell.width][cell.length].effectType = MP_CONSUME;
        maze[cell.floor][cell.width][cell.length].effectValue = (rand() % 4) + 1;
    }

    for (int k = 0; k < bonus1Cells && current_index < total; k++, current_index++)
    {
        CellCord cell = activeCellList[current_index];
        maze[cell.floor][cell.width][cell.length].effectType = MP_ADD;
        maze[cell.floor][cell.width][cell.length].effectValue = (rand() % 2) + 1;
    }

    for (int k = 0; k < bonus2Cells && current_index < total; k++, current_index++)
    {
        CellCord cell = activeCellList[current_index];
        maze[cell.floor][cell.width][cell.length].effectType = MP_ADD;
        maze[cell.floor][cell.width][cell.length].effectValue = (rand() % 3) + 3;
    }

    for (int k = 0; k < multplyCells && current_index < total; k++, current_index++)
    {
        CellCord cell = activeCellList[current_index];
        maze[cell.floor][cell.width][cell.length].effectType = MP_MULTIPLY;
        maze[cell.floor][cell.width][cell.length].effectValue = (rand() % 2) + 2;
    }
}

// ----------------------------------------SET UP BAWANA----------------------------------------
void bawanaSetUp()
{
    // shuffle bawana cells array
    for (int i = bawanaCellCount - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        struct BawanaCell tmp = bawanaCells[i];
        bawanaCells[i] = bawanaCells[j];
        bawanaCells[j] = tmp;
    }
    for (int i = 0; i < bawanaCellCount; i++)
    {
        if (i < 2)
        {
            bawanaCells[i].type = POISONED_CELL;
            bawanaCells[i].movementPoints = 0;
        }
        else if (i < 4)
        {
            bawanaCells[i].type = DISORIENTED_CELL;
            bawanaCells[i].movementPoints = 50;
        }
        else if (i < 6)
        {
            bawanaCells[i].type = TRIGGERED_CELL;
            bawanaCells[i].movementPoints = 50;
        }
        else if (i < 8)
        {
            bawanaCells[i].type = HAPPY_CELL;
            bawanaCells[i].movementPoints = 200;
        }
        else
        {
            bawanaCells[i].type = RANDOM_CELL;
            bawanaCells[i].movementPoints = rand() % 91 + 10;
        }
    }
}

// ----------------------------------------LOAD FILES----------------------------------------
void loadSeed()
{
    FILE *file = fopen("seed.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Error: opening seed.txt... using default value(1) as seed\n");
        srand(1);
        return;
    }
    int seed;
    fscanf(file, "%d", &seed);
    fclose(file);
    srand(seed);
}

void loadStairs()
{
    FILE *file = fopen("stairs.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Error: opening stairs.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    stairs = (struct Stair *)malloc(capacity * sizeof(struct Stair));
    if (!stairs)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
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
        tempStair.dir = BI_DIR;
        stairs[count++] = tempStair;
        if (count >= capacity)
        {
            capacity *= 2;
            stairs = (struct Stair *)realloc(stairs, capacity * sizeof(struct Stair));
            if (!stairs)
            {
                fprintf(stderr, "Error: Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        fprintf(stderr, "Error: No valid stairs were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    stairsCount = count;
}

void loadPoles()
{
    FILE *file = fopen("poles.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Error: opening poles.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    poles = (struct Pole *)malloc(capacity * sizeof(struct Pole));
    if (!poles)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
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
                fprintf(stderr, "Error: Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        fprintf(stderr, "Error: No valid poles were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    polesCount = count;
}

void loadFlag()
{
    FILE *file = fopen("flag.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Error: opening flag.txt\n");
        exit(1);
    }
    CellCord flagPosition;
    if (fscanf(file, " [%d, %d, %d] ", &flagPosition.floor, &flagPosition.width, &flagPosition.length) != 3)
    {
        fprintf(stderr, "Error: Invalid flag format in flag.txt\n");
        fclose(file);
        exit(1);
    }
    if (!isValidCell(flagPosition))
    {
        printf("Error: Invalid Flag location....flag location set to [2, 0, 8]\n");
        Flag.floor = 2;
        Flag.width = 0;
        Flag.length = 8;
    }
    else
    {
        Flag = flagPosition;
    }
    fclose(file);
}

void loadWalls()
{
    FILE *file = fopen("walls.txt", "r");
    if (!file)
    {
        fprintf(stderr, "Error: opening walls.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    walls = (struct Wall *)malloc(capacity * sizeof(struct Wall));
    if (!walls)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
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
                fprintf(stderr, "Error: Memory reallocation failed \n");
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        fprintf(stderr, "Error: No valid walls were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    wallsCount = count;
}

// ----------------------------------------ADD OBJECTS TO MAZE----------------------------------------
void addStairsToMaze()
{
    for (int i = 0; i < stairsCount; i++)
    {
        struct Stair tempStair = stairs[i];
        maze[tempStair.startFloor][tempStair.startBlockWidth][tempStair.startBlockLength].cellType = STAIR_CELL;
        maze[tempStair.startFloor][tempStair.startBlockWidth][tempStair.startBlockLength].cellTypeId = tempStair.stairId;

        maze[tempStair.endFloor][tempStair.endBlockWidth][tempStair.endBlockLength].cellType = STAIR_CELL;
        maze[tempStair.endFloor][tempStair.endBlockWidth][tempStair.endBlockLength].cellTypeId = tempStair.stairId;
    }
}

void addPolesToMaze()
{
    for (int i = 0; i < polesCount; i++)
    {
        struct Pole tempPole = poles[i];
        if (abs(tempPole.endFloor - tempPole.startFloor) > 1)
        {
            for (int f = 0; f < FLOORS; f++)
            {
                maze[f][tempPole.widthCell][tempPole.lengthCell].cellType = POLE_CELL;
                maze[f][tempPole.widthCell][tempPole.lengthCell].cellTypeId = tempPole.poleId;
            }
        }
        else
        {
            maze[tempPole.startFloor][tempPole.widthCell][tempPole.lengthCell].cellType = POLE_CELL;
            maze[tempPole.startFloor][tempPole.widthCell][tempPole.lengthCell].cellTypeId = tempPole.poleId;

            maze[tempPole.endFloor][tempPole.widthCell][tempPole.lengthCell].cellType = POLE_CELL;
            maze[tempPole.endFloor][tempPole.widthCell][tempPole.lengthCell].cellTypeId = tempPole.poleId;
        }
    }
}

void addWallstoMaze()
{
    for (int i = 0; i < wallsCount; i++)
    {
        struct Wall tempWall = walls[i];

        if (tempWall.startBlockWidth == tempWall.endBlockWidth)
        {
            int step = (tempWall.startBlockLength < tempWall.endBlockLength) ? 1 : -1;
            for (int l = tempWall.startBlockLength; l != tempWall.endBlockLength + step; l += step)
            {
                maze[tempWall.floor][tempWall.startBlockWidth][l].cellType = WALL_CELL;
            }
        }
        else
        {
            int step = (tempWall.startBlockWidth < tempWall.endBlockWidth) ? 1 : -1;
            for (int w = tempWall.startBlockWidth; w != tempWall.endBlockWidth + step; w += step)
            {
                maze[tempWall.floor][w][tempWall.startBlockLength].cellType = WALL_CELL;
            }
        }
    }
}

void addFlagToMaze()
{
    maze[Flag.floor][Flag.width][Flag.length].cellType = FLAG_CELL;
}

// ----------------------------------------CALLING FUNCTIONS----------------------------------------
void loadFiles()
{
    loadWalls();
    loadStairs();
    loadPoles();
    loadFlag();
}

void addObjectsToMaze()
{
    addWallstoMaze();
    addStairsToMaze();
    addPolesToMaze();
    addFlagToMaze();
}

void intializeMaze()
{
    setUpFloors(maze);
    loadFiles();
    addObjectsToMaze();
    addMovementPointsToCells();
    bawanaSetUp();
}

#endif