#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --------------------constants--------------------
#define FLOORS 3
#define WIDTH 10
#define LENGTH 25
#define NO_PLAYERS 3

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
    TRIGGERED
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

struct BawanaCell
{
    CellCord cellCoord;
    BawanaCellType type;
    int movementPoints;
};

typedef struct
{
    char name;
    CellCord currentCell; // current location of the player
    CellCord startCell;   // player's starting location in starting area
    Direction startDir;   // player's direction when in starting area
    Direction dir;
    int movementPoints;
    int throwsCount;
    PlayerStatus status;
    int throwsLeftInStatus;
} Player;

typedef struct
{
    char player;
    int steps;
    Direction dir;
    CellCord currentCell;
    int movementPoints;
    char msgBuffer[500];
} Move;

// ----------------------------------------GLOBAL VARIABLES---------------------------------------
// constants
extern const CellCord BawanaEntry;

extern const CellCord specialCells[7];

extern const char *stringDirections[];

extern const char *stringBawanaEffects[];

// arrays
extern struct Cell maze[FLOORS][WIDTH][LENGTH];
extern Player players[NO_PLAYERS];

extern CellCord Flag;

// array pointers
extern struct Stair *stairs;
extern struct Pole *poles;
extern struct Wall *walls;
extern struct BawanaCell *bawanaCells;

// counters
extern int no_Stairs;
extern int no_Poles;
extern int no_Walls;
extern int no_BawanaCells;


extern int gameRound;
#endif