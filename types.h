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

typedef enum
{
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NO_CHANGE
} Direction;

typedef enum
{
    STARTING_AREA,
    IN_MAZE,
    POISONED,
    DISORIENTED,
    TRIGGERED,
    HAPPY
} PlayerStatus;

typedef enum
{
    UP,
    DOWN,
    BI_DIR
} StairDirection;

typedef enum
{
    POISONED_CELL,
    DISORIENTED_CELL,
    TRIGGERED_CELL,
    HAPPY_CELL,
    RANDOM_CELL
} BawanaCellType;

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
    StairDirection dir;
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

typedef struct
{
    char name;
    CellCord currentPos;
    CellCord entryPos;
    Direction dir;
    int movementPoints;
    int throwsCount;
    PlayerStatus status;
    int statusDuration;
} Player;

struct BawanaCell
{
    int BawanaCellId;
    BawanaCellType type;
    int movementPoints;
};

// ----------------------------------------GLOBAL VARIABLES---------------------------------------
extern struct Cell maze[FLOORS][WIDTH][LENGTH];

extern CellCord Flag;

extern struct Stair *stairs;
extern struct Pole *poles;
extern struct Wall *walls;
extern struct BawanaCell *bawanaCells;

extern int stairsCount;
extern int polesCount;
extern int wallsCount;
extern int bawanaCellCount;


#endif