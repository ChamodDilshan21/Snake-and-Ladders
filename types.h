#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// --------------------constants--------------------
#define FLOORS 3
#define WIDTH 10
#define LENGTH 25

// --------------------enums--------------------
typedef enum
{
    EMPTY_CELL,
    ACTIVE_CELL,
    STARTING_AREA_CELL,
    BAWANA_CELL,
    STAIR_CELL,
    POLE_CELL,
    WALL_CELL,
    FLAG_CELL,
    BAWANA_ENTRY
} CellType;

typedef enum
{
    MP_NONE,
    MP_CONSUME,
    MP_ADD,
    MP_MULTIPLY
} MPEffectType;

// --------------------structs--------------------
typedef struct
{
    int floor;
    int width;
    int length;
} CellCord;

struct Cell
{
    CellType cellType;
    int cellTypeId;
    MPEffectType effectType;
    int effectValue;
};

struct Stair
{
    int stairId;
    int startFloor;
    int startBlockWidth;
    int startBlockLength;
    int endFloor;
    int endBlockWidth;
    int endBlockLength;
};

struct Pole
{
    int poleId;
    int startFloor;
    int endFloor;
    int widthCell;
    int lengthCell;
};

struct Wall
{
    int floor;
    int startBlockWidth;
    int startBlockLength;
    int endBlockWidth;
    int endBlockLength;
};

#endif