#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"
#include <stdlib.h>

// ----------------------------------------VALIDATE DATA---------------------------------------
bool isValidFloor(int floor) { return floor >= 0 && floor < FLOORS; }

bool isValidWidth(int width) { return width >= 0 && width < WIDTH; }

bool isValidLength(int length) { return length >= 0 && length < LENGTH; }

bool isValidCordinates(CellCord cell)
{
    return isValidFloor(cell.floor) && isValidWidth(cell.width) && isValidLength(cell.length);
}

bool isGameCell(CellCord cell) { return maze[cell.floor][cell.width][cell.length].cellType == ACTIVE_CELL; }

bool isValidCell(CellCord cell) { return isValidCordinates(cell) && isGameCell(cell); }

bool isBlockedCell(struct Cell cell) { return cell.cellType == WALL_CELL || cell.cellType == EMPTY_CELL; }

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

// ----------------------------------------STAIR DIRECTION CHANGE---------------------------------------

void changeStairDirection()
{
    for (int i = 0; i < stairsCount; i++)
    {
        switch (rand() % 3)
        {
        case 1:
            stairs[i].dir = UP;
            break;
        case 2:
            stairs[i].dir = DOWN;
            break;

        default:
            stairs[i].dir = BI_DIR;
            break;
        }
    }
}

// ----------------------------------------DICE ROLLERS---------------------------------------
int rollMovementDice()
{
    return (rand() % 6) + 1;
}

Direction rollDirectionDice()
{
    int face = rand() % 6 + 1;
    switch (face)
    {
    case 2:
        return NORTH;
    case 3:
        return EAST;
    case 4:
        return SOUTH;
    case 5:
        return WEST;
    default:
        return NO_CHANGE;
    }
}

// ----------------------------------------HELPERS---------------------------------------
CellCord getNextCellCoord(CellCord current, Direction dir)
{
    CellCord new = current;
    switch (dir)
    {
    case NORTH:
        new.width -= 1;
        break;
    case SOUTH:
        new.width += 1;
        break;
    case EAST:
        new.length += 1;
        break;
    case WEST:
        new.length -= 1;
        break;
    default:
        break;
    }
    return new;
}

struct Cell getNextCell(CellCord nextCell)
{
    return maze[nextCell.floor][nextCell.width][nextCell.length];
}

bool isMovePossible(CellCord current, Direction dir, int steps)
{
    for (int i = 0; i < steps; i++)
    {
        CellCord nextCoord = getNextCellCoord(current, dir);
        if (!isValidCordinates(nextCoord))
        {
            return false;
        }
        struct Cell nextCell = getNextCell(nextCoord);
        if (isBlockedCell(nextCell))
        {
            return false;
        }
    }
    return true;
}

struct BawanaCell getRandomBawanaCell()
{
    return bawanaCells[rand() % 12];
}

#endif