#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"
#include <stdlib.h>

// ----------------------------------------GLOBAL VARIABLES---------------------------------------
extern struct Cell maze[FLOORS][WIDTH][LENGTH];

extern CellCord Flag;

extern struct Stair *stairs;
extern struct Pole *poles;
extern struct Wall *walls;

extern int stairsCount;
extern int polesCount;
extern int wallsCount;

// ----------------------------------------VALIDATE DATA---------------------------------------
bool isValidFloor(int floor) { return floor >= 0 && floor <= 2; }

bool isValidWidth(int width) { return width >= 0 && width <= 9; }

bool isValidLength(int length) { return length >= 0 && length <= 24; }

bool isValidCordinates(CellCord cell)
{
    return isValidFloor(cell.floor) && isValidWidth(cell.width) && isValidLength(cell.length);
}

bool isGameCell(CellCord cell) { return maze[cell.floor][cell.width][cell.length].cellType == ACTIVE_CELL; }

bool isValidCell(CellCord cell) { return isValidCordinates(cell) && isGameCell(cell); }

bool isValidStair(struct Stair stair)
{
    CellCord startCell = {stair.startFloor, stair.startBlockWidth, stair.startBlockLength};
    CellCord endCell = {stair.endFloor, stair.endBlockWidth, stair.endBlockLength};

    return isValidCell(startCell) &&
           isValidCell(endCell) &&
           (startCell.floor + 1 == endCell.floor) &&
           !(startCell.width == endCell.width && startCell.length == endCell.length);
}

bool isValidPole(struct Pole pole)
{
    CellCord startCell = {pole.startFloor, pole.widthCell, pole.lengthCell};
    CellCord endCell = {pole.endFloor, pole.widthCell, pole.lengthCell};

    return isValidCell(startCell) && isValidCell(endCell) && (startCell.floor != endCell.floor);
}

bool isValidWall(struct Wall wall)
{
    if (!isValidFloor(wall.floor) ||
        !isValidWidth(wall.startBlockWidth) ||
        !isValidWidth(wall.endBlockWidth) ||
        !isValidLength(wall.startBlockLength) ||
        !isValidLength(wall.endBlockLength))
    {
        return false;
    }

    bool isVertical = wall.startBlockLength == wall.endBlockLength;
    bool isHorizontal = wall.startBlockWidth == wall.endBlockWidth;

    if (!isVertical && !isHorizontal)
    {
        return false;
    }

    int wallSize = isHorizontal ? abs(wall.startBlockLength - wall.endBlockLength) : abs(wall.startBlockWidth - wall.endBlockWidth);
    if (isHorizontal)
    {
        int l = wall.startBlockLength < wall.endBlockLength ? wall.startBlockLength : wall.endBlockLength;
        for (int i = 0; i <= wallSize; i++)
        {
            CellCord cell = {wall.floor, wall.startBlockWidth, l++};
            if (!isGameCell(cell))
            {
                return false;
            }
        }
    }
    else
    {
        int w = wall.startBlockWidth < wall.endBlockWidth ? wall.startBlockWidth : wall.endBlockWidth;
        for (int i = 0; i <= wallSize; i++)
        {
            CellCord cell = {wall.floor, w++, wall.startBlockLength};
            if (!isGameCell(cell))
            {
                return false;
            }
        }
    }
    return true;
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

#endif