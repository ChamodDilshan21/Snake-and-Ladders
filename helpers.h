#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"

// ----------------------------------------VALIDATION SUPPORT---------------------------------------

bool isValidFloor(int floor) { return floor >= 0 && floor < FLOORS; }

bool isValidWidth(int width) { return width >= 0 && width < WIDTH; }

bool isValidLength(int length) { return length >= 0 && length < LENGTH; }

bool isValidCordinates(CellCord cell)
{
    return isValidFloor(cell.floor) && isValidWidth(cell.width) && isValidLength(cell.length);
}

// check two location are the same
bool isSameCord(CellCord c1, CellCord c2)
{
    return c1.floor == c2.floor && c1.width == c2.width && c1.length == c2.length;
}

// check if cell doesn't have any object currently
bool isVacantCell(CellCord cell)
{
    if (maze[cell.floor][cell.width][cell.length].cellType == STARTING_AREA_CELL)
    {
        for (int i = 0; i < sizeof(specialCells) / sizeof(specialCells[0]); i++)
        {
            if (isSameCord(cell, specialCells[i]))
            {
                return false;
            }
        }
        return true;
    }
    return maze[cell.floor][cell.width][cell.length].cellType == ACTIVE_CELL;
}

// check if cell is within booundires and vacant
bool isValidCell(CellCord cell) { return isValidCordinates(cell) && isVacantCell(cell); }

// check for walls and boundries
bool isBlockedCell(CellCord cell)
{
    return maze[cell.floor][cell.width][cell.length].cellType == WALL_CELL ||
           maze[cell.floor][cell.width][cell.length].cellType == EMPTY_CELL;
}

bool isStartingAreaCell(CellCord cell) { return maze[cell.floor][cell.width][cell.length].cellType == STARTING_AREA_CELL; }

// check if stair is valid
bool isValidStair(struct Stair stair)
{
    CellCord startCell = {stair.startFloor, stair.startBlockWidth, stair.startBlockLength};
    CellCord endCell = {stair.endFloor, stair.endBlockWidth, stair.endBlockLength};

    return isValidCell(startCell) &&
           isValidCell(endCell) &&
           (startCell.floor + 1 == endCell.floor) && // stair should be from a lower floor to a upper floor and cannot go through floors
           !(startCell.width == endCell.width && startCell.length == endCell.length); // stairs cannot be vertical
}

// check if pole is valid
bool isValidPole(struct Pole pole)
{
    CellCord startCell = {pole.startFloor, pole.widthCell, pole.lengthCell};
    CellCord endCell = {pole.endFloor, pole.widthCell, pole.lengthCell};

    return isValidCell(startCell) && isValidCell(endCell) && (startCell.floor < endCell.floor);
}

// check if stair is valid
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

    bool isVertical = wall.startBlockLength == wall.endBlockLength; // vertical wall
    bool isHorizontal = wall.startBlockWidth == wall.endBlockWidth; // horizontal wall

    // walls cannot diagonal
    if (!isVertical && !isHorizontal)
    {
        return false;
    }

    // calc wall size(cells)
    int wallWidth = abs(wall.startBlockWidth - wall.endBlockWidth) + 1;
    int wallLength = abs(wall.startBlockLength - wall.endBlockLength) + 1;

    if (isHorizontal)
    {
        int l = wall.startBlockLength < wall.endBlockLength ? wall.startBlockLength : wall.endBlockLength;

        //check wall is contiguous and do not have objects(stairs/poles/flag) in middle
        for (int i = 0; i <= wallLength; i++)
        {
            CellCord cell = {wall.floor, wall.startBlockWidth, l++};
            if (maze[cell.floor][cell.width][cell.length].cellType != ACTIVE_CELL)
            {
                return false;
            }
        }
    }
    else if (isVertical)
    {
        int w = wall.startBlockWidth < wall.endBlockWidth ? wall.startBlockWidth : wall.endBlockWidth;

        //check wall is contiguous and do not have objects(stairs/poles/flag) in middle
        for (int i = 0; i <= wallWidth; i++)
        {
            CellCord cell = {wall.floor, w++, wall.startBlockLength};
            if (maze[cell.floor][cell.width][cell.length].cellType != ACTIVE_CELL)
            {
                return false;
            }
        }
    }

    // check if wall is a entire barrier (from one end to other)
    if (wall.floor == 1)
    {
        if (wall.startBlockWidth < 6)
        {
            if (isHorizontal)
            {
                return wallLength < 8;
            }
        }
        else if (wall.startBlockLength > 7 && wall.startBlockLength < 17)
        {
            if (isVertical)
            {
                return wallWidth < 4;
            }
        }
    }
    else if (wall.floor == 2)
    {
        if (isHorizontal)
        {
            return wallLength < 9;
        }
    }

    if (isHorizontal)
    {
        return wallLength < LENGTH;
    }
    else if (isVertical)
    {
        return wallWidth < WIDTH;
    }

    return true;
}

// ---------------------------------------GAME SUPPORT---------------------------------------

//roll movement dice
int rollMovementDice()
{
    return (rand() % 6) + 1;
}

//roll direction dice
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

// change stair direction randomly
void changeStairDirection()
{
    for (int i = 0; i < no_Stairs; i++)
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

// format cell coordinate to string -> format - [0, 0, ,0]
const char *cordToString(CellCord c)
{
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "[%d, %d, %d]", c.floor, c.width, c.length);
    return buffer;
}

// ----------------------------------------PLAYER MOVE SUPPORT---------------------------------------

// find the coordinates of the next cell
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

// find the next cell
struct Cell getNextCell(CellCord nextCell) { return maze[nextCell.floor][nextCell.width][nextCell.length]; }

// get a random bawana cell
struct BawanaCell getRandomBawanaCell() { return bawanaCells[rand() % 12]; }

// check if player has captured a another
void hasCapturedPlayer(char capturedBy, CellCord cell)
{
    for (int i = 0; i < NO_PLAYERS; i++)
    {
        if (players[i].name != capturedBy && isSameCord(players[i].currentCell, cell))
        {
            players[i].currentCell = players[i].startCell;
            printf("%c has been captured by %c at %s. Player %c has send to his starting location in starting area.\n",
                   players[i].name, capturedBy, cordToString(cell), players[i].name);
            return;
        }
    }
}

// check if player has returned to starting area
bool backToStartingArea(Player *p)
{
    if (getNextCell(p->currentCell).cellType == STARTING_AREA_CELL)
    {
        p->currentCell = p->startCell;
        p->dir = p->startDir;
        p->status = STARTING_AREA;
        return true;
    }
    return false;
}

#endif