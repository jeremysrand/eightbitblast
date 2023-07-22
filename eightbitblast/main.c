/*
 * main.c
 * eightbitblast
 *
 * Created by Jeremy Rand on 2023-07-21.
 * Copyright (c) 2023 ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_ENEMIES 50
#define SHOT_CHAR '!'
#define HERO_CHAR '@'
#define MAX_X 40
#define MAX_Y 24

typedef enum tEnemyType {
    ENEMY_NONE = 0,
    ENEMY_COMMODORE = 'C',
    ENEMY_ATARI = 'A'
} tEnemyType;

typedef struct tEnemy {
    uint8_t x;
    uint8_t y;
    tEnemyType type;
} tEnemy;

tEnemy gEnemies[MAX_ENEMIES];
uint8_t gNumEnemies;
uint32_t gScore;
uint8_t gHeroX;
bool gShotVisible = false;
uint8_t gShotX = 0;
uint8_t gShotY = 0;

void initGame(void)
{
    clrscr();
    memset(gEnemies, 0, sizeof(gEnemies));
    gNumEnemies = 0;
    gScore = 0;
    gHeroX = MAX_X / 2;
    gShotVisible = false;
}

void delay(void)
{
    uint16_t count;
    for (count = 0; count < 300; count++)
        ;
}

void playSound(int8_t freq, int16_t duration)
{
    int8_t temp;
    while (duration > 0) {
#ifdef __CC65__
        asm volatile ("STA %w", 0xc030);
#endif
        temp = freq;
        while (temp > 0) {
            temp--;
        }
        duration--;
    }
}

void displayInstructions(void)
{
    int seed = 0;
    char ch;
    
    clrscr();
    
    //               1111111111222222222233333333334
    //      1234567890123456789012345678901234567890
    printf(
           "              8 Bit Blast\n"
           "            By Jeremy Rand\n"
           "\n"
           "The goal is to shoot the non-Apple 8-bit"
           "computers dropping towards you.  Use the"
           "arrow keys to move the Apple left and\n"
           "right and space bar to shoot.  Press\n"
           "escape to quit.  Do not let the lesser\n"
           "computers crash into the apple.\n"
           "\n"
           "    Apple     : @ (You)\n"
           "    Commodore : C (10 points)\n"
           "    Atari     : A (100 points)\n"
           "\n"
           "Press any key to start");
    
    // The amount of time the user waits to read the in
    while (!kbhit())
        seed++;
    
    ch = cgetc();
    srand(seed);
    
    clrscr();
}

void killEnemy(uint8_t enemyNum)
{
    gShotVisible = false;
    gScore += (gEnemies[enemyNum].type == ENEMY_COMMODORE ? 10 : 100);
    gEnemies[enemyNum].type = ENEMY_NONE;
    playSound(70, 20);
    gNumEnemies--;
}

void clearEnemies(void)
{
    uint8_t enemyNum;
    
    for (enemyNum = 0; enemyNum < MAX_ENEMIES; enemyNum++) {
        if (gEnemies[enemyNum].type != ENEMY_NONE) {
            cputcxy(gEnemies[enemyNum].x, gEnemies[enemyNum].y, ' ');
            if ((gShotVisible) &&
                (gShotX == gEnemies[enemyNum].x) &&
                (gShotY == gEnemies[enemyNum].y)) {
                killEnemy(enemyNum);
            } else {
                if (++gEnemies[enemyNum].y >= MAX_Y) {
                    gEnemies[enemyNum].type = ENEMY_NONE;
                    gNumEnemies--;
                }
            }
        }
    }
}


void addEnemies(void)
{
    uint8_t enemiesToAdd = 0;
    uint8_t random;
    uint8_t x1;
    uint8_t x2;
    uint8_t x3;
    uint8_t enemyNum;
    
    if (gNumEnemies >= MAX_ENEMIES)
        return;
    
    random = rand() & 0xf;
    if (random == 0xf)
        enemiesToAdd = 3;
    else if (random > 8)
        enemiesToAdd = 2;
    else if (random > 2)
        enemiesToAdd = 1;
    
    if (gNumEnemies + enemiesToAdd > MAX_ENEMIES)
        enemiesToAdd = MAX_ENEMIES - gNumEnemies;
    
    if (enemiesToAdd == 0)
        return;
    
    x1 = rand() % MAX_X;
    do {
        x2 = rand() % MAX_X;
    } while (x2 == x1);
    do {
        x3 = rand() % MAX_X;
    } while ((x3 == x1) || (x3 == x2));
    
    for (enemyNum = 0; enemyNum < MAX_ENEMIES; enemyNum++) {
        if (gEnemies[enemyNum].type == ENEMY_NONE) {
            gNumEnemies++;
            gEnemies[enemyNum].y = 1;
            gEnemies[enemyNum].type = (((rand() & 0xf) > 0xc) ? ENEMY_ATARI : ENEMY_COMMODORE);
            if (enemiesToAdd == 3) {
                gEnemies[enemyNum].x = x3;
            } else if (enemiesToAdd == 2) {
                gEnemies[enemyNum].x = x2;
            } else {
                gEnemies[enemyNum].x = x1;
                return;
            }
            enemiesToAdd--;
        }
    }
}

void drawEnemies(void)
{
    uint8_t enemyNum;
    for (enemyNum = 0; enemyNum < MAX_ENEMIES; enemyNum++) {
        if (gEnemies[enemyNum].type != ENEMY_NONE) {
            if ((gShotVisible) &&
                (gShotX == gEnemies[enemyNum].x) &&
                (gShotY == gEnemies[enemyNum].y)) {
                killEnemy(enemyNum);
                cputcxy(gShotX, gShotY, ' ');
            } else
                cputcxy(gEnemies[enemyNum].x, gEnemies[enemyNum].y, gEnemies[enemyNum].type);
        }
    }
}

void gameOver(void)
{
    uint16_t count;
    char ch;
    
    cputcxy(gHeroX, MAX_Y - 1, '*');
    
    cputsxy(10, 11, "                     ");
    cputsxy(10, 12, "      GAME OVER      ");
    cputsxy(10, 13, "                     ");
    
    playSound(30, 200);
    playSound(50, 200);
    
    for (count = 0; count < 5000; count++)
        ;
    
    if (kbhit())
        cgetc();
    
    ch = cgetc();
    if ((ch == 'Q') ||
        (ch == 'q') ||
        (ch == 27)) {
        clrscr();
        exit(0);
    }
    initGame();
}

char charAtHero(void)
{
    char * ptr = ((char *)0x7d0) + gHeroX;
    return (*ptr & 0x7f);
}

void drawHero(void)
{
    int8_t delta = 0;
    char ch;
    
    if (kbhit()) {
        ch = cgetc();
        if (ch == ' ') {
            if (!gShotVisible) {
                playSound(50, 20);
                gShotVisible = true;
                gShotX = gHeroX;
                gShotY = MAX_Y - 1;
            }
        } else if (ch == 8) {
            // Move to the left
            if (gHeroX > 0)
                delta = -1;
        } else if (ch == 21) {
            // Move to the right
            if (gHeroX < MAX_X - 1)
                delta = 1;
        } else if ((ch == 27) ||
                   (ch == 'Q') ||
                   (ch == 'q')) {
            clrscr();
            exit(0);
        }
    }
    
    if (delta != 0) {
        ch = charAtHero();
        if ((ch != ' ') &&
            (ch != HERO_CHAR))
            gameOver();
        
        cputcxy(gHeroX, MAX_Y - 1, ' ');
        gHeroX += delta;
    }
    
    ch = charAtHero();
    if ((ch != ' ') &&
        (ch != HERO_CHAR))
        gameOver();
    
    cputcxy(gHeroX, MAX_Y - 1, HERO_CHAR);
}

void drawShot(void)
{
    if (!gShotVisible)
        return;
    
    if (gShotY < MAX_Y - 1)
        cputcxy(gShotX, gShotY, ' ');
    
    if (gShotY == 0) {
        gShotVisible = false;
        return;
    }
    
    gShotY--;
    cputcxy(gShotX, gShotY, SHOT_CHAR);
}

int main(void)
{
    initGame();
    displayInstructions();
    while (true) {
        gotoxy(0, 0);
        cprintf("  SCORE: %lu", gScore);
        clearEnemies();
        addEnemies();
        drawEnemies();
        drawHero();
        drawShot();
        delay();
    }
    return 0;
}
