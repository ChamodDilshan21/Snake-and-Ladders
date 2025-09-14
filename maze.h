#ifndef MAZE_H
#define MAZE_H

#include "helpers.h"

// ----------------------------------------INITIALIZE MAZE FLOORS----------------------------------------
void setUpFloors(struct Cell maze[FLOORS][WIDTH][LENGTH]) // --> do not create a copy when passing a array to a function because C passes arrays to functions by reference, not by value.  the compiler treats the function parameter as a pointer to the first element of the original array.
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
    int cap = 12;
    int count = 0;
    bawanaCells = (struct BawanaCell *)malloc(cap * sizeof(struct BawanaCell));

    for (int w = 0; w < WIDTH; w++)
    {
        for (int l = 0; l < LENGTH; l++)
        {
            maze[0][w][l].cellType = ACTIVE_CELL;

            if (w >= 6)
            {
                if (l >= 8 && l <= 16)
                {
                    maze[0][w][l].cellType = STARTING_AREA_CELL;
                }
                else if (w > 6 && l > 20)
                {
                    if (count >= cap)
                    {
                        cap *= 2;
                        bawanaCells = (struct BawanaCell *)realloc(bawanaCells, cap * sizeof(struct BawanaCell));
                        if (!bawanaCells)
                        {
                            printf("\nError: Memory reallocation failed.\n");
                            exit(1);
                        }
                    }
                    maze[0][w][l].cellType = BAWANA_CELL;
                    maze[0][w][l].cellTypeId = count;
                    bawanaCells[count++] = (struct BawanaCell){(CellCord){0, w, l}, RANDOM_CELL, 0};
                }
                else if (l >= 20)
                {
                    maze[0][w][l].cellType = WALL_CELL;
                }
            }
        }
    }
    no_BawanaCells = count;
    maze[BawanaEntry.floor][BawanaEntry.width][BawanaEntry.length].cellType = BAWANA_ENTRY; // marking bawana entry

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
        for (int l = 8; l <= 16; l++)
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
    if (activeCellList == NULL)
    {
        printf("\nError: Memory allocation failed.\n");
        exit(1);
    }

    // add all active cells to activeCellList
    for (int f = 0; f < FLOORS; f++)
    {
        for (int w = 0; w < WIDTH; w++)
        {
            for (int l = 0; l < LENGTH; l++)
            {
                if (isVacantCell((CellCord){f, w, l}))
                {
                    if (total >= capacity)
                    {
                        capacity *= 2;
                        CellCord *temp = (CellCord *)realloc(activeCellList, capacity * sizeof(CellCord));
                        if (temp == NULL)
                        {
                            printf("\nError: Memory reallocation failed.\n");
                            exit(1);
                        }
                        activeCellList = temp;
                    }
                    activeCellList[total++] = (CellCord){f, w, l};
                }
            }
        }
    }

    if (total == 0)
    {
        printf("\nError: No active game in maze. Error in maze initialization. Quitting Game....\n");
        exit(1);
    }

    // shuffle elements in activeCellList
    for (int i = total - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        CellCord tmp = activeCellList[i];
        activeCellList[i] = activeCellList[j];
        activeCellList[j] = tmp;
    }

    int consumeCells = total * 35 / 100;
    int bonus1Cells = total * 25 / 100; // cells with bonuses of 1 or 2
    int bonus2Cells = total * 10 / 100; // cells with bonuses of 3 to 5
    int multplyCells = total * 5 / 100;

    int current_index = 0;

    // assigning values to active cells randomly according to the effect type quatation
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
    free(activeCellList);
}

// ----------------------------------------SET UP BAWANA----------------------------------------
void bawanaSetUp()
{
    for (int i = 0; i < no_BawanaCells; i++)
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
    int seed;
    if (!file)
    {
        fprintf(stderr, "Error: opening seed.txt... using default value(1) as seed.\n");
        fflush(stderr);
        seed = 1;
        return;
    }
    else
    {
        fscanf(file, "%d", &seed);
    }
    fclose(file);
    srand(seed);
}

void loadStairs()
{
    FILE *file = fopen("stairs.txt", "r");
    if (!file)
    {
        printf("Error: Could not open stairs.txt\n");
        exit(1);
    }

    struct Stair tempStair;
    int line = 0, validCount = 0;

    // count valid stairs
    while (1)
    {
        int values = fscanf(file, " [%d, %d, %d, %d, %d, %d] ",
                            &tempStair.startFloor,
                            &tempStair.startBlockWidth,
                            &tempStair.startBlockLength,
                            &tempStair.endFloor,
                            &tempStair.endBlockWidth,
                            &tempStair.endBlockLength);

        if (values == EOF)
        {
            break;
        }

        line++;

        if (values != 6)
        {
            fprintf(stderr, "Line %d of stairs.txt: Malformed input (expected 6 integers)\n", line);
            fflush(stderr);
            continue;
        }

        if (isValidStair(tempStair, line, true))
        {
            validCount++;
        }
    }

    if (validCount == 0)
    {
        printf("Error: No valid stairs found in stairs.txt. Quitting Game....\n");
        fclose(file);
        exit(1);
    }

    // Allocate exact memory
    stairs = malloc(validCount * sizeof(struct Stair));
    if (!stairs)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        exit(1);
    }

    // load valid stairs
    rewind(file);
    int count = 0;
    line = 0;

    while (1)
    {
        int values = fscanf(file, " [%d, %d, %d, %d, %d, %d] ",
                            &tempStair.startFloor,
                            &tempStair.startBlockWidth,
                            &tempStair.startBlockLength,
                            &tempStair.endFloor,
                            &tempStair.endBlockWidth,
                            &tempStair.endBlockLength);

        if (values == EOF)
        {
            break;
        }
        line++;

        if (values != 6)
        {
            continue;
        }

        if (!isValidStair(tempStair, line, false))
        {
            continue;
        }

        tempStair.stairId = count;
        tempStair.dir = BI_DIR;
        stairs[count++] = tempStair;
    }

    fclose(file);
    no_Stairs = count;
}

void loadPoles()
{
    FILE *file = fopen("poles.txt", "r");
    if (!file)
    {
        printf("Error: Could not open poles.txt\n");
        exit(1);
    }

    struct Pole tempPole;
    int line = 0, validCount = 0;

    // count valid poles
    while (1)
    {
        int values = fscanf(file, " [%d, %d, %d, %d] ",
                            &tempPole.startFloor,
                            &tempPole.endFloor,
                            &tempPole.widthCell,
                            &tempPole.lengthCell);

        if (values == EOF)
        {
            break;
        }

        line++;

        if (values != 4)
        {
            continue;
        }

        if (isValidPole(tempPole, line, false))
        {
            validCount++;
        }
    }

    if (validCount == 0)
    {
        printf("Error: No valid poles found in poles.txt. Quitting Game....\n");
        fclose(file);
        exit(1);
    }

    // Allocate exact memory
    poles = malloc(validCount * sizeof(struct Pole));
    if (!poles)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        exit(1);
    }

    // load valid poles
    rewind(file);
    int count = 0;
    line = 0;

    while (1)
    {
        int values = fscanf(file, " [%d, %d, %d, %d] ",
                            &tempPole.startFloor,
                            &tempPole.endFloor,
                            &tempPole.widthCell,
                            &tempPole.lengthCell);

        if (values == EOF)
        {
            break;
        }

        line++;

        if (values != 4)
        {
            fprintf(stderr, "Line %d of poles.txt: Malformed input (expected 4 integers)\n", line);
            fflush(stderr);
            continue;
        }

        if (!isValidPole(tempPole, line, true))
        {
            continue;
        }

        tempPole.poleId = count;
        poles[count++] = tempPole;
    }

    fclose(file);
    no_Poles = count;
}

void loadWalls()
{
    FILE *file = fopen("walls.txt", "r");
    if (!file)
    {
        printf("\nError: opening walls.txt\n");
        exit(1);
    }

    int capacity = 10;
    int count = 0;
    walls = (struct Wall *)malloc(capacity * sizeof(struct Wall));
    if (!walls)
    {
        printf("\nError: Memory allocation failed.\n");
        fclose(file);
        exit(1);
    }

    struct Wall tempWall;
    int line = 0;
    while (fscanf(file, " [%d, %d, %d, %d, %d] ",
                  &tempWall.floor,
                  &tempWall.startBlockWidth,
                  &tempWall.startBlockLength,
                  &tempWall.endBlockWidth,
                  &tempWall.endBlockLength) == 5)
    {
        line++;
        if (!isValidWall(tempWall, line, true))
        {
            continue;
        }
        walls[count++] = tempWall;
        if (count >= capacity)
        {
            capacity *= 2;
            walls = (struct Wall *)realloc(walls, capacity * sizeof(struct Wall));
            if (!walls)
            {
                printf("\nError: Memory reallocation failed.\n");
                fclose(file);
                exit(1);
            }
        }
    }
    fclose(file);
    if (count == 0)
    {
        printf("\nError: No valid walls were loaded from the file. Quitting Game....\n");
        exit(1);
    }
    no_Walls = count;
}

void loadFlag()
{
    FILE *file = fopen("flag.txt", "r");
    if (!file)
    {
        printf("\nError: opening flag.txt\n");
        exit(1);
    }

    CellCord flagPosition;
    if (fscanf(file, " [%d, %d, %d] ", &flagPosition.floor, &flagPosition.width, &flagPosition.length) != 3)
    {
        printf("\nError: Invalid flag format in flag.txt\n");
        fclose(file);
        exit(1);
    }

    if (!isVacantCell(flagPosition))
    {
        printf("\nError: Invalid flag location. Quitting Game....\n");
        fclose(file);
        exit(1);
    }

    Flag = flagPosition;
    fclose(file);
}

// ----------------------------------------ADD OBJECTS TO MAZE----------------------------------------
void addStairsToMaze()
{
    for (int i = 0; i < no_Stairs; i++)
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
    for (int i = 0; i < no_Poles; i++)
    {
        struct Pole tempPole = poles[i];
        for (int f = tempPole.startFloor; f <= tempPole.endFloor; f++)
        {
            maze[f][tempPole.widthCell][tempPole.lengthCell].cellType = POLE_CELL;
            maze[f][tempPole.widthCell][tempPole.lengthCell].cellTypeId = tempPole.poleId;
        }
    }
}

void addWallstoMaze()
{
    for (int i = 0; i < no_Walls; i++)
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

void intializeMaze()
{
    setUpFloors(maze);

    loadFlag();
    addFlagToMaze();

    loadWalls();
    addWallstoMaze();

    loadStairs();
    addStairsToMaze();

    loadPoles();
    addPolesToMaze();

    addMovementPointsToCells();

    bawanaSetUp();
}

#endif