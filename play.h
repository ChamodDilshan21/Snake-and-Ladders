#ifndef PLAY_H
#define PLAY_H

#include "types.h"
#include "helpers.h"

// ----------------------------------------INITIALIZE PLAYERS---------------------------------------
void initPlayers()
{

    players[0] = (Player){
        'A',
        (CellCord){0, 6, 12},
        (CellCord){0, 6, 12},
        NORTH,
        NORTH,
        100,
        0,
        STARTING_AREA,
        0};

    players[1] = (Player){
        'B',
        (CellCord){0, 9, 8},
        (CellCord){0, 9, 8},
        WEST,
        WEST,
        100,
        0,
        STARTING_AREA,
        0};

    players[2] = (Player){
        'C',
        (CellCord){0, 9, 16},
        (CellCord){0, 9, 16},
        EAST,
        EAST,
        100,
        0,
        STARTING_AREA,
        0};
}

// ----------------------------------------MOVEMENT HELP---------------------------------------

bool takeStair(CellCord *c, int index)
{
    CellCord stairStart = {stairs[index].startFloor, stairs[index].startBlockWidth, stairs[index].startBlockLength};
    CellCord stairEnd = {stairs[index].endFloor, stairs[index].endBlockWidth, stairs[index].endBlockLength};
    bool atStairStart = isSameCord(*c, stairStart);
    bool atStairEnd = isSameCord(*c, stairEnd);

    StairDirection dir = stairs[index].dir;

    if ((dir == UP && atStairEnd) || (dir == DOWN && atStairStart))
    {
        return false;
    }
    else if ((dir == UP && atStairStart) || dir == BI_DIR && atStairStart)
    {
        *c = stairEnd;
    }
    else if ((dir == DOWN && atStairEnd) || (dir == BI_DIR && atStairEnd))
    {
        *c = stairStart;
    }
    return true;
}

void takePole(CellCord *c, int index)
{
    CellCord poleStart = {poles[index].startFloor, poles[index].widthCell, poles[index].lengthCell};
    CellCord poleEnd = {poles[index].endFloor, poles[index].widthCell, poles[index].lengthCell};

    if (c->floor <= poleEnd.floor)
    {
        *c = poleStart;
    }
}

// calc movement ponts for a single step
int calcMovementPoints(CellCord cell, int mp)
{
    switch (maze[cell.floor][cell.width][cell.length].effectType)
    {
    case MP_CONSUME:
        return mp - maze[cell.floor][cell.width][cell.length].effectValue;
        break;

    case MP_ADD:
        return mp + maze[cell.floor][cell.width][cell.length].effectValue;
        break;

    case MP_MULTIPLY:
        return mp * maze[cell.floor][cell.width][cell.length].effectValue;

    default:
        return mp;
    }
}

// apply effect to player according to the bawana cell they land on
void applyBawanaEffect(Player *p, struct BawanaCell *bawanaCell)
{
    switch (bawanaCell->type)
    {
    case POISONED_CELL:
        p->currentCell = bawanaCell->cellCoord;
        p->movementPoints = bawanaCell->movementPoints;
        p->status = POISONED;
        p->throwsLeftInStatus = 3;
        printf("%c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->name);
        break;

    case DISORIENTED_CELL:
        p->currentCell = BawanaEntry;
        p->movementPoints = bawanaCell->movementPoints;
        p->status = DISORIENTED;
        p->throwsLeftInStatus = 4;
        printf("%c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->name);
        break;

    case TRIGGERED_CELL:
        p->currentCell = BawanaEntry;
        p->movementPoints = bawanaCell->movementPoints;
        p->status = TRIGGERED;
        p->throwsLeftInStatus = 4;
        printf("%c eats from Bawana and is triggered due to bad quality of food. %c is placed at the entrance of Bawana with 50 movement points.\n", p->name, p->name);
        break;

    case HAPPY_CELL:
        p->currentCell = BawanaEntry;
        p->movementPoints = bawanaCell->movementPoints;
        p->status = IN_MAZE;
        p->throwsLeftInStatus = 0;
        printf("%c eats from Bawana and is happy. %c is placed at the entrance of Bawana with 200 movement points.\n", p->name, p->name);
        break;

    case RANDOM_CELL:
        p->currentCell = BawanaEntry;
        p->movementPoints = bawanaCell->movementPoints;
        p->status = IN_MAZE;
        p->throwsLeftInStatus = 0;
        printf("%c eats from Bawana and earns %d movement points and is placed at the entrance of Bawana.\n", p->name, p->movementPoints);
        break;
    }
}

// ----------------------------------------PLAYER MOVEMENT IMPLIMETATION---------------------------------------

// check if player can move to next cell or blocked
bool isNextStepPossible(CellCord current, Direction dir)
{
    CellCord nextCoord = getNextCellCoord(current, dir);
    return isValidCordinates(nextCoord) && !isBlockedCell(nextCoord);
}

// move player to next cell
void movePlayer(Move *move, int *offset)
{
    int bytes_written; // update msgBuffer and offset

    CellCord nextCellCord = getNextCellCoord(move->currentCell, move->dir);
    struct Cell nextCell = getNextCell(nextCellCord);
    move->movementPoints = calcMovementPoints(nextCellCord, move->movementPoints);

    if (nextCell.cellType == STAIR_CELL) // check if player have to take a stair
    {
        if (takeStair(&nextCellCord, nextCell.cellTypeId))
        {
            move->movementPoints = calcMovementPoints(nextCellCord, move->movementPoints);

            bytes_written = snprintf(move->msgBuffer + *offset, sizeof(move->msgBuffer) - *offset, "%c lands on %s which is a stair cell. %c takes the stairs and now placed at %s.\n",
                                     move->player, cordToString(move->currentCell), move->player, cordToString(nextCellCord)); // add msg to msgBuffer

            *offset += bytes_written;
        }
    }
    else if (nextCell.cellType == POLE_CELL) // check if player have to take a pole
    {
        takePole(&nextCellCord, nextCell.cellTypeId);
        move->movementPoints = calcMovementPoints(nextCellCord, move->movementPoints);

        bytes_written = snprintf(move->msgBuffer + *offset, sizeof(move->msgBuffer) - *offset, "%c lands on %s which is a pole cell. %c slides down and now placed at %s.\n",
                                 move->player, cordToString(move->currentCell), move->player, cordToString(nextCellCord)); // add msg to msgBuffer

        *offset += bytes_written;
    }
    else if (nextCell.cellType == FLAG_CELL) // check if player has reached the flag
    {
        printf("\n\n------------------------------------- Game Over -------------------------------------\n\n");
        printf("%c has capture the flag at %s. The winner is %c.\n\n", move->player, cordToString(nextCellCord), move->player);
        exit(0); // successfully complete the game.
    }
    move->currentCell = nextCellCord; // move player to next cell
}

// check and do the players move
bool isPlayerMoved(Move *move)
{
    int bufferOffset = 0; // keep track of msgBuffer in move
    for (int i = 0; i < move->steps; i++)
    {
        if (isNextStepPossible(move->currentCell, move->dir))
        {
            movePlayer(move, &bufferOffset);
        }
        else
        {
            return false;
        }
    }
    return true;
}

// ----------------------------------------IMPLIMETATION OF A SINGLE TURN OF A PLAYER---------------------------------------
void playerTurn(Player *player)
{
    // check if poisoned
    if (player->status == POISONED)
    {
        // check if food poisoning still affect
        if (player->throwsLeftInStatus > 0)
        {
            printf("%c is still food poisoned and misses the turn.\n", player->name);
            player->throwsLeftInStatus--;
        }
        else
        {
            struct BawanaCell bawana = getRandomBawanaCell();
            printf("%c is now fit to proceed from the food poisoning episode and now placed on a %s and the effects take place.\n",
                   player->name, stringBawanaEffects[bawana.type]);
            applyBawanaEffect(player, &bawana);
        }
        player->throwsCount++;
        return;
    }

    Move move;

    // roll movement dice
    int movementDice = rollMovementDice();

    // increase throw count and deduct mp
    player->movementPoints -= 2;
    player->throwsCount++;

    // roll direction dice if possible
    Direction dir = player->dir;
    bool isDirectionDiceRoll = false;
    if (player->status != STARTING_AREA && player->throwsCount % 4 == 0)
    {
        dir = rollDirectionDice();
        player->dir = dir == NO_CHANGE ? player->dir : dir;
        isDirectionDiceRoll = true;
    }

    // applying necessary effects to the move according to player's status
    if (player->status == DISORIENTED)
    {
        if (player->throwsLeftInStatus > 0)
        {
            dir = rollDirectionDice();
            printf("%c rolls and %d on the movement dice and is disoriented and move in the %s.\n", player->name, movementDice, stringDirections[dir]);
        }
        printf("%c has recovered from disorientation.\n", player->name);
        move = (Move){player->name, movementDice, dir, player->currentCell, 0, ""};
    }
    else if (player->status == TRIGGERED)
    {
        if (player->throwsLeftInStatus > 0)
        {
            printf("%c is triggered and rolls and %d on the movement dice and move in the %s and moving %d cells.\n", player->name, movementDice, stringDirections[dir], movementDice * 2);
            movementDice *= 2;
        }
        printf("%c has recovered from triggered status.\n", player->name);
        move = (Move){player->name, movementDice, dir, player->currentCell, 0, ""};
    }
    else if (player->status == STARTING_AREA)
    {
        if (movementDice == 6)
        {
            player->status = IN_MAZE;
            printf("%c is at the starting area and rolls 6 on the movement dice and is placed on his entry cell of the maze.\n", player->name);
            move = (Move){player->name, 1, dir, player->currentCell, 0, ""};
        }
        else
        {
            printf("%c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", player->name, movementDice);
            return;
        }
    }
    else
    {
        move = (Move){player->name, movementDice, dir, player->currentCell, 0, ""};
    }

    // check player status
    // poisioned -> how many turns left? if timesup place randomly in bawana ----------> DONE
    // triggered -> roll dice but multiply steps by 2----------> DONE
    // happy -> nothing special----------> DONE
    // disoriented -> how many turns left? if timesup set status-in_maze else: random movement (steps+direction)----------> DONE
    // in_maze -> normal movement----------> DONE
    // starting_area -> should land 6 to be in_maze----------> DONE

    // movement dice roll everytime----------> DONE
    // direction dice - every 4 time----------> DONE

    // check movement possible? ----------> DONE
    // if possible take move ----------> DONE
    // take stair, poles ----------> DONE
    // after taking move check whether mp == 0 ? send to bawana ----------> DONE
    //  end of move check any caputres happen ----------> DONE
    //  if yes send captured player to starting area ----------> DONE
    // check player is in starting area again ----------> DONE
    // deduct 2 mp for blocked move ----------> DONE
    // increase throwCount of player by 1 ----------> DONE
    // print final message

    // move player if possible
    if (isPlayerMoved(&move))
    {
        // update player's movement points and current location
        player->currentCell = move.currentCell;
        player->movementPoints += move.movementPoints;

        printf("%s", move.msgBuffer);

        // check for captures
        hasCapturedPlayer(player->name, player->currentCell);

        if (isDirectionDiceRoll)
        {
            printf("%c rolls and %d on the movement dice and %s on the direction dice, changes direction to %s and moves %d cells and is now at %s.\n",
                   player->name, movementDice, stringDirections[dir], stringDirections[dir], move.steps, cordToString(player->currentCell));
        }
        else
        {
            printf("%c rolls and %d on the movement dice and moves %s by %d cells and is now at %s.\n",
                   player->name, movementDice, stringDirections[dir], move.steps, cordToString(player->currentCell));
        }

        // check player's mp and if  mp <= 0 teleport to bawana
        if (player->movementPoints <= 0)
        {
            struct BawanaCell bawana = getRandomBawanaCell();
            printf("%c movement points are depleted and requires replenishment. Transporting to Bawana.\n", player->name);
            printf("%c is place on a %s cell and effects take place.\n", player->name, stringBawanaEffects[bawana.type]);
            applyBawanaEffect(player, &bawana);
        }

        // check if player is back to starting area
        if (backToStartingArea(player))
        {
            printf("%c is back to starting area. %c send to his starting location - %s and direction change to starting direction - %s. Movement points are not reset.\n",
                   player->name, player->name, cordToString(player->startCell), stringDirections[player->startDir]);
        }

        // print movement point and direction at the end of the move
        printf("%c moved %d cells that cost %d movement points and is left with %d and is moving in the %s.\n",
               player->name, move.steps, move.movementPoints, player->movementPoints, stringDirections[player->dir]);
    }
    else
    {
        if (isDirectionDiceRoll)
        {
            printf("%c rolls and %d on the movement dice and %s on the direction dice, direction changed to %s but cannot move. Player remains at %s.\n",
                   player->name, movementDice, stringDirections[dir], stringDirections[dir], cordToString(player->currentCell));
        }
        else
        {
            printf("%c rolls and %d on the movement dice and cannot move in the %s. Player remains at %s.\n",
                   player->name, movementDice, stringDirections[dir], cordToString(player->currentCell));
        }
    }
}

#endif