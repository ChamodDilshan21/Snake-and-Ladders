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

bool isSpecialCell(CellCord cell)
{
    for (int i = 0; i < sizeof(specialCells) / sizeof(specialCells[0]); i++)
    {
        if (isSameCord(cell, specialCells[i]))
        {
            return true;
        }
    }
    return false;
}

// check if cell is only a game cell and no object in it or is not a special cell(player start, player entry, bawana entry)
bool isVacantCell(CellCord cell)
{
    return isValidCordinates(cell) && maze[cell.floor][cell.width][cell.length].cellType == ACTIVE_CELL;
}

// check if cell is within booundires and vacant
bool isValidCell(CellCord cell)
{
    return isValidCordinates(cell) && !isSpecialCell(cell) &&
           (maze[cell.floor][cell.width][cell.length].cellType == ACTIVE_CELL || maze[cell.floor][cell.width][cell.length].cellType == STARTING_AREA_CELL);
}

// check for walls and boundries
bool isBlockedCell(CellCord cell)
{
    return maze[cell.floor][cell.width][cell.length].cellType == WALL_CELL ||
           maze[cell.floor][cell.width][cell.length].cellType == EMPTY_CELL;
}

bool isStartingAreaCell(CellCord cell) { return maze[cell.floor][cell.width][cell.length].cellType == STARTING_AREA_CELL; }

// check if a stair is valid
bool isValidStair(struct Stair stair, int line, bool logError)
{
    CellCord startCell = {stair.startFloor, stair.startBlockWidth, stair.startBlockLength};
    CellCord endCell = {stair.endFloor, stair.endBlockWidth, stair.endBlockLength};

    if (!isValidCell(startCell))
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of stairs.txt: Invalid start cell [%d,%d,%d].\n",
                    line, stair.startFloor, stair.startBlockWidth, stair.startBlockLength);
            fflush(stderr);
        }
        return false;
    }

    if (!isValidCell(endCell))
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of stairs.txt: Invalid end cell [%d,%d,%d].\n",
                    line, stair.endFloor, stair.endBlockWidth, stair.endBlockLength);
            fflush(stderr);
        }
        return false;
    }

    if (startCell.floor + 1 != endCell.floor)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of stairs.txt: Stair must connect consecutive floors (got %d -> %d).\n",
                    line, startCell.floor, endCell.floor);
            fflush(stderr);
        }
        return false;
    }

    if (startCell.width == endCell.width && startCell.length == endCell.length)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of stairs.txt: Stair cannot be vertical.\n", line);
            fflush(stderr);
        }
        return false;
    }

    return true;
}

// check if pole is valid
bool isValidPole(struct Pole pole, int line, bool logError)
{
    CellCord startCell = {pole.startFloor, pole.widthCell, pole.lengthCell};
    CellCord endCell = {pole.endFloor, pole.widthCell, pole.lengthCell};

    if (!isValidCell(startCell))
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of poles.txt: Invalid start cell [%d,%d,%d].\n",
                    line, pole.startFloor, pole.widthCell, pole.lengthCell);
            fflush(stderr);
        }
        return false;
    }

    if (!isValidCell(endCell))
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of poles.txt: Invalid end cell [%d,%d,%d].\n",
                    line, pole.endFloor, pole.widthCell, pole.lengthCell);
            fflush(stderr);
        }
        return false;
    }

    if (startCell.floor >= endCell.floor)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of poles.txt: Pole must start below and end above (got %d -> %d).\n",
                    line, startCell.floor, endCell.floor);
            fflush(stderr);
        }
        return false;
    }

    return true;
}

// check if wall is valid
bool isValidWall(struct Wall wall, int line, bool logError)
{
    if (!isValidFloor(wall.floor) ||
        !isValidWidth(wall.startBlockWidth) ||
        !isValidWidth(wall.endBlockWidth) ||
        !isValidLength(wall.startBlockLength) ||
        !isValidLength(wall.endBlockLength))
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of walls.txt: has out-of-bound coordinates.\n", line);
            fflush(stderr);
        }
        return false;
    }

    bool isVertical = (wall.startBlockLength == wall.endBlockLength); // vertical wall
    bool isHorizontal = (wall.startBlockWidth == wall.endBlockWidth); // horizontal wall

    if (!isVertical && !isHorizontal)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of walls.txt: Walls can not be diagonal.\n", line);
            fflush(stderr);
        }
        return false;
    }

    // calc wall size(cells)
    int wallWidth = abs(wall.startBlockWidth - wall.endBlockWidth) + 1;
    int wallLength = abs(wall.startBlockLength - wall.endBlockLength) + 1;

    if (isHorizontal)
    {
        int l = wall.startBlockLength < wall.endBlockLength ? wall.startBlockLength : wall.endBlockLength;
        for (int i = 0; i < wallLength; i++)
        {
            CellCord cell = {wall.floor, wall.startBlockWidth, l++};
            if (maze[cell.floor][cell.width][cell.length].cellType != ACTIVE_CELL)
            {
                if (logError)
                {
                    fprintf(stderr, "Line %d of walls.txt: Wall overlap with special cell or object.\n", line);
                    fflush(stderr);
                }
                return false;
            }
        }
    }
    else if (isVertical)
    {
        int w = wall.startBlockWidth < wall.endBlockWidth ? wall.startBlockWidth : wall.endBlockWidth;
        for (int i = 0; i < wallWidth; i++)
        {
            CellCord cell = {wall.floor, w++, wall.startBlockLength};
            if (maze[cell.floor][cell.width][cell.length].cellType != ACTIVE_CELL)
            {
                if (logError)
                {
                    fprintf(stderr, "Line %d of walls.txt: Wall overlap with special cell or object.\n", line);
                    fflush(stderr);
                }
                return false;
            }
        }
    }

    // prevent full barrier walls depending on floor-specific rules
    if (wall.floor == 1)
    {
        if (wall.startBlockWidth < 6 && isHorizontal && wallLength >= 8)
        {
            if (logError)
            {
                fprintf(stderr, "Line %d of walls.txt: Wall is a full horizontal barrier on floor 1.\n", line);
                fflush(stderr);
            }
            return false;
        }
        if (wall.startBlockLength > 7 && wall.startBlockLength < 17 && isVertical && wallWidth >= 4)
        {
            if (logError)
            {
                fprintf(stderr, "Line %d of walls.txt: Wall is a full vertical barrier on floor 1.\n", line);
                fflush(stderr);
            }
            return false;
        }
    }
    else if (wall.floor == 2)
    {
        if (isHorizontal && wallLength >= 9)
        {
            if (logError)
            {
                fprintf(stderr, "Line %d of walls.txt: Wall is a full horizontal barrier on floor 2.\n", line);
                fflush(stderr);
            }
            return false;
        }
    }

    if (isHorizontal && wallLength >= LENGTH)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of walls.txt: Wall is the entire length of the maze.\n", line);
            fflush(stderr);
        }
        return false;
    }
    else if (isVertical && wallWidth >= WIDTH)
    {
        if (logError)
        {
            fprintf(stderr, "Line %d of walls.txt: Wall is the entire width of the maze.\n", line);
            fflush(stderr);
        }
        return false;
    }

    return true;
}

// ---------------------------------------GAME SUPPORT---------------------------------------

// roll movement dice
int rollMovementDice()
{
    return (rand() % 6) + 1;
}

// roll direction dice
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
struct BawanaCell getRandomBawanaCell() { return bawanaCells[rand() % no_BawanaCells]; }

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