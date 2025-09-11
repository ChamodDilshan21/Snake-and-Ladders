#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FLOORS 3
#define WIDTH 10
#define LENGTH 25

// -------------------- enums & types --------------------
typedef enum {
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

typedef enum {
    MP_NONE,
    MP_CONSUME,   // deduct N
    MP_ADD,       // add N
    MP_MULTIPLY   // multiply by N
} MPEffectType;

typedef struct {
    int floor, width, length;
} CellCord;

struct Stair {
    int stairId;
    int startFloor;
    int startBlockWidth;
    int startBlockLength;
    int endFloor;
    int endBlockWidth;
    int endBlockLength;
    int reversed; // 0 = normal, 1 = reversed (used when direction flips)
};

struct Pole {
    int poleId;
    int startFloor;
    int endFloor;
    int widthCell;
    int lengthCell;
};

struct Wall {
    int floor;
    int startBlockWidth;
    int startBlockLength;
    int endBlockWidth;
    int endBlockLength;
};

struct Cell {
    CellType cellType;
    int cellTypeId; // for stair/pole/wall/bawana id
    MPEffectType effectType;
    int effectValue; // either consume/add or multiply factor
};

typedef enum {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIR_NONE
} Direction;

typedef enum {
    STATUS_STARTING_AREA,
    STATUS_IN_MAZE,
    STATUS_POISONED,
    STATUS_DISORIENTED,
    STATUS_TRIGGERED,
    STATUS_HAPPY
} PlayerStatus;

typedef struct {
    char name;           // 'A','B','C'
    CellCord pos;
    Direction dir;
    int movementPoints;
    int throwsCount;     // to know when to roll direction die
    PlayerStatus status;
    int poisonedTurnsLeft;
    int disorientedTurnsLeft;
    int triggeredTurnsLeft;
} Player;

// -------------------- globals --------------------
struct Cell maze[FLOORS][WIDTH][LENGTH];
CellCord Flag;

struct Stair *stairs = NULL;
struct Pole *poles = NULL;
struct Wall *walls = NULL;

int stairsCount = 0;
int polesCount = 0;
int wallsCount = 0;

int seedVal = 0;
int roundsElapsed = 0; // every three players finishing increments rounds

// bawana coordinates known from spec: ground floor [0], bounds w=6..9, l=20..24 excluding walls
// entrance = [0,9,19]
CellCord bawanaCells[12];
int bawanaCount = 0;
CellCord bawanaEntrance = {0,9,19};

// -------------------- helpers prototypes --------------------
int load_files(void);
void initMaze(void);
void assignCellEffects(void);
int rollMovementDice(void);
Direction rollDirectionDice(void);
CellCord getNextCell(CellCord cur, Direction d);
int isValidCord(CellCord c);
int isGameCellCord(CellCord c);
void printFloors(void);
void loadSeed(const char *fn);

// game logic
void initPlayers(Player players[3]);
void printPlayerPos(Player *p);
int findStairIndexById(int id);
int findPoleByCell(CellCord pos); // returns poleId or -1
void flipStairsRandomly(void);
int detect_and_break_loop(CellCord visited[], int visitedCount); // returns 1 if loop detected

void sendToStartingArea(Player *p);
CellCord randomBawanaCell(void);
void applyBawanaEffect(Player *p, int bawanaIndex);
void movePlayer(Player *p, Player players[], int playerCount);
void applyCellMovementPointEffect(Player *p, struct Cell c);
void captureIfPresent(Player *p, Player players[], int playerCount);

// -------------------- implementations --------------------

int isValidCord(CellCord c) {
    return c.floor >= 0 && c.floor < FLOORS &&
           c.width >= 0 && c.width < WIDTH &&
           c.length >= 0 && c.length < LENGTH;
}

int isGameCellCord(CellCord c) {
    if (!isValidCord(c)) return 0;
    struct Cell cc = maze[c.floor][c.width][c.length];
    return (cc.cellType == ACTIVE_CELL ||
            cc.cellType == STARTING_AREA_CELL ||
            cc.cellType == BAWANA_CELL ||
            cc.cellType == BAWANA_ENTRY);
}

void initMaze(void) {
    // default empty
    for (int f=0; f<FLOORS; f++)
        for (int w=0; w<WIDTH; w++)
            for (int l=0; l<LENGTH; l++) {
                maze[f][w][l].cellType = EMPTY_CELL;
                maze[f][w][l].cellTypeId = -1;
                maze[f][w][l].effectType = MP_NONE;
                maze[f][w][l].effectValue = 0;
            }

    // ground floor active
    for (int w=0; w<WIDTH; w++)
        for (int l=0; l<LENGTH; l++)
            maze[0][w][l].cellType = ACTIVE_CELL;

    // starting area (w 6..9, l 8..16) - mark starting area cells
    for (int w=6; w<=9; w++)
        for (int l=8; l<=16; l++)
            maze[0][w][l].cellType = STARTING_AREA_CELL;

    // bawana region (w 6..9, l 20..24) with walls at w==6 or l==20
    int id = 0;
    for (int w=6; w<=9; w++) {
        for (int l=20; l<=24; l++) {
            if (w==6 || l==20) {
                maze[0][w][l].cellType = WALL_CELL;
            } else {
                maze[0][w][l].cellType = BAWANA_CELL;
                maze[0][w][l].cellTypeId = id;
                bawanaCells[bawanaCount++] = (CellCord){0,w,l};
                id++;
            }
        }
    }
    maze[0][9][19].cellType = BAWANA_ENTRY;
    maze[0][9][19].cellTypeId = -1;

    // floor 1 active except the central blocked area per spec
    for (int w=0; w<WIDTH; w++) {
        for (int l=0; l<LENGTH; l++) {
            if (w>=0 && w<=5 && l>=8 && l<=16) continue;
            maze[1][w][l].cellType = ACTIVE_CELL;
        }
    }
    // floor 2 active l 0..8
    for (int w=0; w<WIDTH; w++)
        for (int l=0; l<=8; l++)
            maze[2][w][l].cellType = ACTIVE_CELL;
}

// random distribution for cell effects per Rule 10
void assignCellEffects(void) {
    // Build list of all eligible cells (ACTIVE_CELL, BAWANA_ENTRY excluded for effect assignment maybe)
    int total = 0;
    CellCord list[FLOORS*WIDTH*LENGTH];
    for (int f=0; f<FLOORS; f++)
        for (int w=0; w<WIDTH; w++)
            for (int l=0; l<LENGTH; l++) {
                struct Cell c = maze[f][w][l];
                if (c.cellType==ACTIVE_CELL) {
                    list[total++] = (CellCord){f,w,l};
                }
            }
    if (total==0) return;
    // Percentages:
    // 25% consumable zero
    // 35% consumable 1..4
    // 25% bonuses 1..2 (MP_ADD)
    // 10% bonuses 3..5 (MP_ADD)
    // 5% multiply factors 2 or 3 (MP_MULTIPLY)
    int n_zero = total * 25 / 100;
    int n_consume = total * 35 / 100;
    int n_bonus1 = total * 25 / 100;
    int n_bonus2 = total * 10 / 100;
    int n_mult = total - (n_zero + n_consume + n_bonus1 + n_bonus2);
    // shuffle
    for (int i=total-1;i>0;i--) {
        int j = rand()%(i+1);
        CellCord tmp = list[i]; list[i]=list[j]; list[j]=tmp;
    }
    int idx=0;
    // zero consumable
    for (int k=0;k<n_zero && idx<total;k++,idx++) {
        CellCord cc = list[idx];
        maze[cc.floor][cc.width][cc.length].effectType = MP_CONSUME;
        maze[cc.floor][cc.width][cc.length].effectValue = 0;
    }
    // consume 1..4
    for (int k=0;k<n_consume && idx<total;k++,idx++) {
        CellCord cc = list[idx];
        maze[cc.floor][cc.width][cc.length].effectType = MP_CONSUME;
        maze[cc.floor][cc.width][cc.length].effectValue = (rand()%4)+1;
    }
    // bonus 1..2
    for (int k=0;k<n_bonus1 && idx<total;k++,idx++) {
        CellCord cc = list[idx];
        maze[cc.floor][cc.width][cc.length].effectType = MP_ADD;
        maze[cc.floor][cc.width][cc.length].effectValue = (rand()%2)+1; // 1..2
    }
    // bonus 3..5
    for (int k=0;k<n_bonus2 && idx<total;k++,idx++) {
        CellCord cc = list[idx];
        maze[cc.floor][cc.width][cc.length].effectType = MP_ADD;
        maze[cc.floor][cc.width][cc.length].effectValue = (rand()%3)+3; //3..5
    }
    // multipliers 2 or 3
    for (int k=0;k<n_mult && idx<total;k++,idx++) {
        CellCord cc = list[idx];
        maze[cc.floor][cc.width][cc.length].effectType = MP_MULTIPLY;
        maze[cc.floor][cc.width][cc.length].effectValue = (rand()%2)+2; //2..3
    }
}

void printFloors(void) {
    for (int f=0; f<FLOORS; f++) {
        printf("\n\n========== Floor %d ==========\n", f);
        for (int w=0; w<WIDTH; w++) {
            for (int l=0; l<LENGTH; l++) {
                char s=' ';
                switch (maze[f][w][l].cellType) {
                    case ACTIVE_CELL: s='A'; break;
                    case STARTING_AREA_CELL: s='*'; break;
                    case BAWANA_CELL: s='B'; break;
                    case BAWANA_ENTRY: s='E'; break;
                    case STAIR_CELL: s='S'; break;
                    case POLE_CELL: s='P'; break;
                    case WALL_CELL: s='W'; break;
                    case FLAG_CELL: s='F'; break;
                    default: s='x'; break;
                }
                printf("%c ", s);
            }
            printf("\n");
        }
    }
}

// -------------------- file parsing --------------------
static void *mustAlloc(size_t sz) {
    void *p = malloc(sz);
    if (!p) { perror("malloc"); exit(1); }
    return p;
}

void loadSeed(const char *fn) {
    FILE *f = fopen(fn,"r");
    if (!f) { perror(fn); exit(1); }
    if (fscanf(f,"%d",&seedVal) != 1) {
        fprintf(stderr,"Invalid seed in %s\n",fn); exit(1);
    }
    fclose(f);
    srand(seedVal);
}

int load_files(void) {
    // loads stairs.txt, poles.txt, walls.txt, flag.txt
    // load walls
    FILE *f;
    f = fopen("walls.txt","r");
    if (!f) { perror("walls.txt"); return -1; }
    int cap = 8;
    walls = mustAlloc(cap * sizeof *walls);
    wallsCount = 0;
    char line[256];
    while (fgets(line,sizeof line,f)) {
        int fl, sw, sl, ew, el;
        if (sscanf(line,"[%d,%d,%d,%d,%d]", &fl,&sw,&sl,&ew,&el) == 5 ||
            sscanf(line," %d %d %d %d %d",&fl,&sw,&sl,&ew,&el)==5 ||
            sscanf(line,"[%d, %d, %d, %d, %d]", &fl,&sw,&sl,&ew,&el) == 5) {
            struct Wall w = {fl, sw, sl, ew, el};
            if (wallsCount>=cap) { cap*=2; walls = realloc(walls, cap*sizeof *walls); if (!walls){perror("realloc");exit(1);} }
            walls[wallsCount++] = w;
        }
    }
    fclose(f);

    // load stairs
    f = fopen("stairs.txt","r");
    if (!f) { perror("stairs.txt"); return -1; }
    cap = 8; stairs = mustAlloc(cap * sizeof *stairs); stairsCount = 0;
    while (fgets(line,sizeof line,f)) {
        int sf, sw, sl, ef, ew, el;
        if (sscanf(line,"[%d,%d,%d,%d,%d,%d]", &sf,&sw,&sl,&ef,&ew,&el) == 6 ||
            sscanf(line," %d %d %d %d %d %d",&sf,&sw,&sl,&ef,&ew,&el) == 6 ||
            sscanf(line,"[%d, %d, %d, %d, %d, %d]", &sf,&sw,&sl,&ef,&ew,&el) == 6) {
            struct Stair s; s.stairId = stairsCount; s.startFloor = sf; s.startBlockWidth = sw; s.startBlockLength = sl;
            s.endFloor = ef; s.endBlockWidth = ew; s.endBlockLength = el; s.reversed = 0;
            if (stairsCount>=cap) { cap*=2; stairs = realloc(stairs, cap*sizeof *stairs); if (!stairs){perror("realloc");exit(1);} }
            stairs[stairsCount++] = s;
        }
    }
    fclose(f);

    // load poles
    f = fopen("poles.txt","r");
    if (!f) { perror("poles.txt"); return -1; }
    cap = 8; poles = mustAlloc(cap * sizeof *poles); polesCount = 0;
    while (fgets(line,sizeof line,f)) {
        int sf, ef, w, l;
        if (sscanf(line,"[%d,%d,%d,%d]",&sf,&ef,&w,&l) == 4 ||
            sscanf(line," %d %d %d %d",&sf,&ef,&w,&l) == 4 ||
            sscanf(line,"[%d, %d, %d, %d]",&sf,&ef,&w,&l) == 4) {
            struct Pole p = {polesCount, sf, ef, w, l};
            if (polesCount>=cap) { cap*=2; poles = realloc(poles, cap*sizeof *poles); if (!poles){perror("realloc");exit(1);} }
            poles[polesCount++] = p;
        }
    }
    fclose(f);

    // load flag
    f = fopen("flag.txt","r");
    if (!f) { perror("flag.txt"); return -1; }
    int ff, fw, flg;
    if (fscanf(f,"[%d,%d,%d]", &ff,&fw,&flg) == 3 ||
        fscanf(f,"%d %d %d", &ff,&fw,&flg) == 3 ||
        fscanf(f,"[%d, %d, %d]", &ff,&fw,&flg) == 3) {
        Flag.floor = ff; Flag.width = fw; Flag.length = flg;
        if (isValidCord(Flag) && isGameCellCord(Flag)) {
            maze[Flag.floor][Flag.width][Flag.length].cellType = FLAG_CELL;
        } else {
            // If invalid, default to 0,0,0
            Flag.floor = 0; Flag.width = 0; Flag.length = 0;
            maze[0][0][0].cellType = FLAG_CELL;
        }
    } else {
        printf("Invalid flag.txt format\n");
        fclose(f);
        return -1;
    }
    fclose(f);

    // place walls, stairs, poles into maze
    for (int i=0;i<wallsCount;i++) {
        struct Wall w = walls[i];
        if (w.startBlockWidth == w.endBlockWidth) {
            int step = (w.startBlockLength < w.endBlockLength) ? 1 : -1;
            for (int l = w.startBlockLength; l != w.endBlockLength + step; l += step) {
                if (w.floor>=0 && w.floor<FLOORS && w.startBlockWidth>=0 && w.startBlockWidth<WIDTH && l>=0 && l<LENGTH)
                    maze[w.floor][w.startBlockWidth][l].cellType = WALL_CELL;
            }
        } else {
            int step = (w.startBlockWidth < w.endBlockWidth) ? 1 : -1;
            for (int ww = w.startBlockWidth; ww != w.endBlockWidth + step; ww += step) {
                if (w.floor>=0 && ww>=0 && ww<WIDTH && w.startBlockLength>=0 && w.startBlockLength<LENGTH)
                    maze[w.floor][ww][w.startBlockLength].cellType = WALL_CELL;
            }
        }
    }

    for (int i=0;i<stairsCount;i++) {
        struct Stair s = stairs[i];
        if (isValidCord((CellCord){s.startFloor,s.startBlockWidth,s.startBlockLength}))
            maze[s.startFloor][s.startBlockWidth][s.startBlockLength].cellType = STAIR_CELL, maze[s.startFloor][s.startBlockWidth][s.startBlockLength].cellTypeId = s.stairId;
        if (isValidCord((CellCord){s.endFloor,s.endBlockWidth,s.endBlockLength}))
            maze[s.endFloor][s.endBlockWidth][s.endBlockLength].cellType = STAIR_CELL, maze[s.endFloor][s.endBlockWidth][s.endBlockLength].cellTypeId = s.stairId;
    }
    for (int i=0;i<polesCount;i++) {
        struct Pole p = poles[i];
        // mark pole cell on all floors between start and end (inclusive)
        int low = p.startFloor < p.endFloor ? p.startFloor : p.endFloor;
        int high = p.startFloor < p.endFloor ? p.endFloor : p.startFloor;
        for (int f=0; f<FLOORS; f++) {
            // spec: if pole runs through floors, all floors have pole cell
            if (isValidCord((CellCord){f,p.widthCell,p.lengthCell}))
                maze[f][p.widthCell][p.lengthCell].cellType = POLE_CELL, maze[f][p.widthCell][p.lengthCell].cellTypeId = p.poleId;
        }
    }

    return 0;
}

// ---------- dice and simple helpers ----------
int rollMovementDice(void) { return (rand()%6) + 1; }
Direction rollDirectionDice(void) {
    int face = rand()%6; // 0..5
    switch(face) {
        case 1: return DIR_NORTH;
        case 2: return DIR_EAST;
        case 3: return DIR_SOUTH;
        case 4: return DIR_WEST;
        default: return DIR_NONE; // empty faces
    }
}

CellCord getNextCell(CellCord cur, Direction d) {
    CellCord n = cur;
    switch(d) {
        case DIR_NORTH: n.width -= 1; break;
        case DIR_SOUTH: n.width += 1; break;
        case DIR_EAST:  n.length += 1; break;
        case DIR_WEST:  n.length -= 1; break;
        default: break;
    }
    return n;
}

void printPlayerPos(Player *p) {
    printf("%c is at [%d,%d,%d] and has %d movement points and is %s\n",
        p->name, p->pos.floor, p->pos.width, p->pos.length,
        p->movementPoints,
        p->status==STATUS_STARTING_AREA?"at starting area":
        p->status==STATUS_IN_MAZE?"in maze":
        p->status==STATUS_POISONED?"poisoned":
        p->status==STATUS_DISORIENTED?"disoriented":
        p->status==STATUS_TRIGGERED?"triggered":"happy");
}

int findStairIndexById(int id) {
    for (int i=0;i<stairsCount;i++) if (stairs[i].stairId==id) return i;
    return -1;
}
int findPoleByCell(CellCord pos) {
    for (int i=0;i<polesCount;i++) {
        if (poles[i].widthCell==pos.width && poles[i].lengthCell==pos.length) return i;
    }
    return -1;
}

// every 5 rounds flip some stairs direction randomly
void flipStairsRandomly(void) {
    for (int i=0;i<stairsCount;i++) {
        if ((rand()%2)==0) { // 50% chance flip
            // swap start and end
            struct Stair *s = &stairs[i];
            s->reversed = !s->reversed;
            // swap coordinates
            int sf = s->startFloor, sw = s->startBlockWidth, sl = s->startBlockLength;
            s->startFloor = s->endFloor; s->startBlockWidth = s->endBlockWidth; s->startBlockLength = s->endBlockLength;
            s->endFloor = sf; s->endBlockWidth = sw; s->endBlockLength = sl;
            // update maze cellTypeId for the two endpoints (ensure still S)
            if (isValidCord((CellCord){s->startFloor,s->startBlockWidth,s->startBlockLength}))
                maze[s->startFloor][s->startBlockWidth][s->startBlockLength].cellType = STAIR_CELL, maze[s->startFloor][s->startBlockWidth][s->startBlockLength].cellTypeId = s->stairId;
            if (isValidCord((CellCord){s->endFloor,s->endBlockWidth,s->endBlockLength}))
                maze[s->endFloor][s->endBlockWidth][s->endBlockLength].cellType = STAIR_CELL, maze[s->endFloor][s->endBlockWidth][s->endBlockLength].cellTypeId = s->stairId;
        }
    }
}

// if a player loops between stairs/poles infinitely, detect visited sequence; we detect repeated cell visited twice in same sequence
int detect_and_break_loop(CellCord visited[], int visitedCount) {
    for (int i=0;i<visitedCount;i++) {
        for (int j=i+1;j<visitedCount;j++) {
            if (visited[i].floor==visited[j].floor && visited[i].width==visited[j].width && visited[i].length==visited[j].length) return 1;
        }
    }
    return 0;
}

void sendToStartingArea(Player *p) {
    // place to starting-area default pos per spec:
    if (p->name=='A') p->pos = (CellCord){0,6,12};
    else if (p->name=='B') p->pos = (CellCord){0,9,8};
    else if (p->name=='C') p->pos = (CellCord){0,9,16};
    p->status = STATUS_STARTING_AREA;
    p->movementPoints = 100; // spec: retains movement points? but starting area initial; choose 100
    p->throwsCount = 0;
    p->poisonedTurnsLeft = p->disorientedTurnsLeft = p->triggeredTurnsLeft = 0;
    printf("Player %c moved back to starting area at [%d,%d,%d]\n", p->name, p->pos.floor, p->pos.width, p->pos.length);
}

CellCord randomBawanaCell(void) {
    if (bawanaCount==0) return bawanaEntrance;
    return bawanaCells[rand()%bawanaCount];
}

void applyBawanaEffect(Player *p, int bawanaIndex) {
    // bawanaIndex is index into bawanaCells, or -1 if entrance
    // When player lands in Bawana region we choose random effect per distribution:
    // 2 of each of Food Poisoning, Disoriented, Triggered, Happy -> total 8 cells; remaining 4 give 10..100 movement points
    int effectTypeRoll = rand()%12; // 0..11
    if (effectTypeRoll < 2) {
        // Food poisoning
        p->status = STATUS_POISONED;
        p->poisonedTurnsLeft = 3;
        p->movementPoints = 0;
        printf("Player %c eats from Bawana and has food poisoning. Will miss next 3 throws.\n", p->name);
    } else if (effectTypeRoll < 4) {
        // Disoriented
        p->status = STATUS_DISORIENTED;
        p->movementPoints = 50;
        p->disorientedTurnsLeft = 4;
        // placed in entrance
        p->pos = bawanaEntrance;
        p->dir = DIR_NORTH;
        printf("Player %c is disoriented: placed at entrance with 50 MP, will move randomly for next 4 throws.\n", p->name);
    } else if (effectTypeRoll < 6) {
        // Triggered
        p->status = STATUS_TRIGGERED;
        p->movementPoints = 50;
        p->triggeredTurnsLeft = 4;
        p->pos = bawanaEntrance;
        p->dir = DIR_NORTH;
        printf("Player %c is triggered: placed at entrance with 50 MP and will move twice as fast for next 4 throws.\n", p->name);
    } else if (effectTypeRoll < 8) {
        // Happy
        p->status = STATUS_HAPPY;
        p->movementPoints = 200;
        p->pos = bawanaEntrance;
        p->dir = DIR_NORTH;
        printf("Player %c is happy: placed at entrance with 200 movement points.\n", p->name);
    } else {
        // random MP 10..100
        int mp = (rand()%91) + 10;
        p->status = STATUS_IN_MAZE;
        p->movementPoints = mp;
        p->pos = (bawanaIndex>=0 ? bawanaCells[bawanaIndex] : bawanaEntrance);
        printf("Player %c eats from Bawana and earns %d movement points and is placed at [%d,%d,%d]\n",
               p->name, mp, p->pos.floor, p->pos.width, p->pos.length);
    }
}

// apply cell effect (consumable or bonus/multiplier) when a player *passes through or lands on* a cell
void applyCellMovementPointEffect(Player *p, struct Cell c) {
    if (c.effectType == MP_NONE) return;
    if (c.effectType == MP_CONSUME) {
        p->movementPoints -= c.effectValue;
        printf("%c stepped on consumable %d: MP now %d\n", p->name, c.effectValue, p->movementPoints);
    } else if (c.effectType == MP_ADD) {
        p->movementPoints += c.effectValue;
        printf("%c stepped on bonus +%d: MP now %d\n", p->name, c.effectValue, p->movementPoints);
    } else if (c.effectType == MP_MULTIPLY) {
        p->movementPoints *= c.effectValue;
        printf("%c stepped on multiplier x%d: MP now %d\n", p->name, c.effectValue, p->movementPoints);
    }
}

// capture another player if present on same cell
void captureIfPresent(Player *p, Player players[], int playerCount) {
    for (int i=0;i<playerCount;i++) {
        if (players[i].name == p->name) continue;
        if (players[i].pos.floor==p->pos.floor && players[i].pos.width==p->pos.width && players[i].pos.length==p->pos.length) {
            // capture
            printf("Player %c lands on player %c -> %c is captured and moved to starting cell.\n", p->name, players[i].name, players[i].name);
            sendToStartingArea(&players[i]);
        }
    }
}

// main movement implementation for a single player's turn
void movePlayer(Player *p, Player players[], int playerCount) {
    if (p->status == STATUS_POISONED) {
        if (p->poisonedTurnsLeft > 0) {
            printf("%c is still food poisoned and misses the turn (turns left: %d)\n", p->name, p->poisonedTurnsLeft);
            p->poisonedTurnsLeft--;
            if (p->poisonedTurnsLeft == 0) {
                // after recovery, placed on a random bawana cell and effect applies (spec)
                CellCord rc = randomBawanaCell();
                p->pos = rc;
                printf("%c has recovered from food poisoning and is placed at Bawana cell [%d,%d,%d]\n", p->name, rc.floor, rc.width, rc.length);
                // apply effect again (could chain) -> call applyBawanaEffect
                applyBawanaEffect(p, 0);
            }
        }
        return;
    }

    // roll movement die always
    int move = rollMovementDice();
    p->throwsCount++;
    int directionDiceRollThisTurn = 0;
    Direction newDir = DIR_NONE;
    // every 4th throw (counting throws for that player) roll direction die
    if (p->throwsCount % 4 == 0) {
        newDir = rollDirectionDice();
        directionDiceRollThisTurn = 1;
        if (newDir != DIR_NONE) {
            p->dir = newDir;
            printf("Player %c rolls direction dice and changes direction to %d\n", p->name, p->dir);
        } else {
            printf("Player %c rolls direction dice and keeps direction %d\n", p->name, p->dir);
        }
    }

    // if player in starting area they must roll 6 to enter
    if (p->status == STATUS_STARTING_AREA) {
        printf("%c is at the starting area and rolls %d\n", p->name, move);
        if (move == 6) {
            // enter maze at player's first cell
            if (p->name=='A') p->pos = (CellCord){0,5,12};
            else if (p->name=='B') p->pos = (CellCord){0,9,7};
            else if (p->name=='C') p->pos = (CellCord){0,9,17};
            p->status = STATUS_IN_MAZE;
            printf("%c is at the starting area and rolls 6 on the movement dice and is placed on [%d,%d,%d] of the maze.\n",
                   p->name, p->pos.floor, p->pos.width, p->pos.length);
            // apply landing cell effect
            applyCellMovementPointEffect(p, maze[p->pos.floor][p->pos.width][p->pos.length]);
            captureIfPresent(p, players, playerCount);
        } else {
            printf("%c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", p->name, move);
        }
        return;
    }

    // if disoriented, direction is random every move for disorientedTurnsLeft (overrides direction dice)
    int forcedRandomDirection = 0;
    if (p->status == STATUS_DISORIENTED && p->disorientedTurnsLeft > 0) {
        forcedRandomDirection = 1;
        p->disorientedTurnsLeft--;
        int face = rand()%4;
        p->dir = (face==0?DIR_NORTH:face==1?DIR_EAST:face==2?DIR_SOUTH:DIR_WEST);
        printf("%c is disoriented and moves in random direction (%d) this turn\n", p->name, p->dir);
        if (p->disorientedTurnsLeft == 0) {
            p->status = STATUS_IN_MAZE;
            printf("%c has recovered from disorientation.\n", p->name);
        }
    }

    // if triggered, movement is doubled
    int movementMultiplier = (p->status==STATUS_TRIGGERED && p->triggeredTurnsLeft>0) ? 2 : 1;
    if (p->status==STATUS_TRIGGERED && p->triggeredTurnsLeft>0) {
        p->triggeredTurnsLeft--;
        if (p->triggeredTurnsLeft==0) { p->status = STATUS_IN_MAZE; printf("%c triggered effect ended.\n", p->name); }
    }

    printf("%c rolls and %d on the movement dice and moves %s by %d cells\n",
           p->name, move,
           p->dir==DIR_NORTH?"North":p->dir==DIR_EAST?"East":p->dir==DIR_SOUTH?"South":"West",
           move * movementMultiplier);

    int stepsToMove = move * movementMultiplier;
    CellCord visited[64];
    int visitedCount = 0;

    for (int step=0; step<stepsToMove; step++) {
        // get next cell
        CellCord next = getNextCell(p->pos, p->dir);

        // boundary or blocked check => stop; entire movement can't partially proceed
        if (!isValidCord(next)) {
            printf("%c cannot move: edge of maze reached at [%d,%d,%d]. Remains at [%d,%d,%d].\n",
                   p->name, next.floor, next.width, next.length, p->pos.floor, p->pos.width, p->pos.length);
            break;
        }
        if (maze[next.floor][next.width][next.length].cellType == WALL_CELL) {
            printf("%c cannot move: blocked by wall at [%d,%d,%d]. Remains at [%d,%d,%d].\n",
                   p->name, next.floor, next.width, next.length, p->pos.floor, p->pos.width, p->pos.length);
            break;
        }

        // advance to next cell
        p->pos = next;
        // record visited for loop detection
        if (visitedCount < (int)(sizeof visited / sizeof visited[0])) visited[visitedCount++] = p->pos;
        // consume/apply cell effect
        applyCellMovementPointEffect(p, maze[p->pos.floor][p->pos.width][p->pos.length]);

        // if stepping into bawana entry or bawana cells
        if (maze[p->pos.floor][p->pos.width][p->pos.length].cellType == BAWANA_CELL) {
            printf("%c has entered Bawana at [%d,%d,%d]\n", p->name, p->pos.floor, p->pos.width, p->pos.length);
            // find bawana index
            int idx = -1;
            for (int bi=0; bi<bawanaCount; bi++) {
                if (bawanaCells[bi].floor==p->pos.floor && bawanaCells[bi].width==p->pos.width && bawanaCells[bi].length==p->pos.length) { idx=bi; break;}
            }
            applyBawanaEffect(p, idx);
            if (p->status != STATUS_IN_MAZE) return; // e.g. poisoned pausing next turns
        } else if (maze[p->pos.floor][p->pos.width][p->pos.length].cellType == BAWANA_ENTRY) {
            // if placed in entry after visiting Bawana they have North direction per spec
            p->dir = DIR_NORTH;
        }

        // stair landing: immediate teleport to other end and continue remaining movement from there
        if (maze[p->pos.floor][p->pos.width][p->pos.length].cellType == STAIR_CELL) {
            int sid = maze[p->pos.floor][p->pos.width][p->pos.length].cellTypeId;
            int sidx = findStairIndexById(sid);
            if (sidx >= 0) {
                struct Stair s = stairs[sidx];
                // determine whether we are on start or end coordinates
                int onStart = (p->pos.floor==s.startFloor && p->pos.width==s.startBlockWidth && p->pos.length==s.startBlockLength);
                int onEnd = (p->pos.floor==s.endFloor && p->pos.width==s.endBlockWidth && p->pos.length==s.endBlockLength);
                if (onStart) {
                    // move to end
                    p->pos = (CellCord){s.endFloor, s.endBlockWidth, s.endBlockLength};
                    printf("%c lands on stair %d and takes it to [%d,%d,%d]\n", p->name, s.stairId, p->pos.floor, p->pos.width, p->pos.length);
                } else if (onEnd) {
                    // per spec landing either end moves to other end
                    p->pos = (CellCord){s.startFloor, s.startBlockWidth, s.startBlockLength};
                    printf("%c lands on stair %d and takes it to [%d,%d,%d]\n", p->name, s.stairId, p->pos.floor, p->pos.width, p->pos.length);
                }
                // after teleport, continue. check loops
                if (detect_and_break_loop(visited, visitedCount)) {
                    printf("Infinite stairs/pole loop detected for %c. Moving %c to starting area and keeping MP.\n", p->name, p->name);
                    sendToStartingArea(p);
                    return;
                }
            }
        }

        // pole landing: slide down to lowest floor of the pole (target floor)
        if (maze[p->pos.floor][p->pos.width][p->pos.length].cellType == POLE_CELL) {
            int pid = maze[p->pos.floor][p->pos.width][p->pos.length].cellTypeId;
            if (pid >= 0 && pid < polesCount) {
                struct Pole pol = poles[pid];
                // target = min(startFloor,endFloor)
                int target = pol.startFloor < pol.endFloor ? pol.startFloor : pol.endFloor;
                p->pos.floor = target;
                printf("%c lands on pole %d and slides to floor %d at [%d,%d,%d]\n", p->name, pid, target, p->pos.floor, p->pos.width, p->pos.length);
                if (detect_and_break_loop(visited, visitedCount)) {
                    printf("Infinite stairs/pole loop detected for %c. Moving to starting area.\n", p->name);
                    sendToStartingArea(p);
                    return;
                }
            }
        }

        // if landed on flag
        if (maze[p->pos.floor][p->pos.width][p->pos.length].cellType == FLAG_CELL) {
            printf("Player %c landed on FLAG at [%d,%d,%d] — game over! %c wins!\n", p->name, p->pos.floor, p->pos.width, p->pos.length, p->name);
            exit(0);
        }

        // if movement points depleted mid-move -> sent to Bawana
        if (p->movementPoints <= 0) {
            printf("%c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->name);
            // place to entrance and apply Bawana
            p->pos = bawanaEntrance;
            applyBawanaEffect(p, -1);
            return;
        }

        // capture other players if present (Rule 5)
        captureIfPresent(p, players, playerCount);
    } // end steps

    // end of turn messages
    printf("%c moved and ended at [%d,%d,%d]; MP=%d; Direction=%s\n",
           p->name, p->pos.floor, p->pos.width, p->pos.length, p->movementPoints,
           p->dir==DIR_NORTH?"North":p->dir==DIR_EAST?"East":p->dir==DIR_SOUTH?"South":"West");
}

// -------------------- initialize players --------------------
void initPlayers(Player players[3]) {
    // A
    players[0].name = 'A';
    players[0].pos = (CellCord){0,6,12}; // starting area
    players[0].dir = DIR_NORTH;
    players[0].movementPoints = 100;
    players[0].throwsCount = 0;
    players[0].status = STATUS_STARTING_AREA;
    players[0].poisonedTurnsLeft = players[0].disorientedTurnsLeft = players[0].triggeredTurnsLeft = 0;
    // B
    players[1].name = 'B';
    players[1].pos = (CellCord){0,9,8};
    players[1].dir = DIR_WEST;
    players[1].movementPoints = 100;
    players[1].throwsCount = 0;
    players[1].status = STATUS_STARTING_AREA;
    players[1].poisonedTurnsLeft = players[1].disorientedTurnsLeft = players[1].triggeredTurnsLeft = 0;
    // C
    players[2].name = 'C';
    players[2].pos = (CellCord){0,9,16};
    players[2].dir = DIR_EAST;
    players[2].movementPoints = 100;
    players[2].throwsCount = 0;
    players[2].status = STATUS_STARTING_AREA;
    players[2].poisonedTurnsLeft = players[2].disorientedTurnsLeft = players[2].triggeredTurnsLeft = 0;
}

// -------------------- main --------------------
int main(void) {
    // load seed first if exists
    FILE *s = fopen("seed.txt","r");
    if (s) {
        fclose(s);
        loadSeed("seed.txt");
    } else {
        seedVal = (int)time(NULL);
        srand(seedVal);
        printf("No seed.txt found — using time seed %d\n", seedVal);
    }

    initMaze();

    if (load_files() != 0) {
        fprintf(stderr,"Error loading files. Ensure walls.txt, stairs.txt, poles.txt, flag.txt exist.\n");
        return 1;
    }

    assignCellEffects();

    printFloors();

    // show some seeded info
    printf("\nAssigned effects to active cells. Seed = %d\n", seedVal);

    Player players[3];
    initPlayers(players);

    // main game loop: players take turns until flag captured (game exits)
    int turnIndex = 0;
    int playerCount = 3;
    while (1) {
        Player *p = &players[turnIndex];
        printf("\n--- Player %c's turn (round %d) ---\n", p->name, roundsElapsed+1);
        movePlayer(p, players, playerCount);

        // increment round when all three played once
        turnIndex++;
        if (turnIndex >= playerCount) {
            turnIndex = 0;
            roundsElapsed++;
            // every 5 rounds flip stairs randomly per spec
            if (roundsElapsed % 5 == 0) {
                printf("Five rounds completed — randomly changing some stairs direction now.\n");
                flipStairsRandomly();
            }
        }
    }

    return 0;
}
