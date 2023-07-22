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
#define SHOT_CHAR ('!' | (char)0x80)
#define HERO_CHAR ('@' | (char)0x80)
#define SPACE_CHAR (' ' | (char)0x80)
#define STAR_CHAR ('*' | (char)0x80)
#define MAX_X 40
#define MAX_Y 24

typedef enum tEnemyType {
    ENEMY_NONE = 0,
    ENEMY_COMMODORE = 'C' | (char)0x80,
    ENEMY_ATARI = 'A' | (char)0x80
} tEnemyType;

typedef struct tEnemy {
    tEnemyType type;
    uint8_t x;
    uint8_t y;
} tEnemy;

char * gScreenAddrs[MAX_Y] = {
    (char *)0x400,
    (char *)0x480,
    (char *)0x500,
    (char *)0x580,
    (char *)0x600,
    (char *)0x680,
    (char *)0x700,
    (char *)0x780,
    (char *)0x428,
    (char *)0x4A8,
    (char *)0x528,
    (char *)0x5A8,
    (char *)0x628,
    (char *)0x6A8,
    (char *)0x728,
    (char *)0x7A8,
    (char *)0x450,
    (char *)0x4D0,
    (char *)0x550,
    (char *)0x5D0,
    (char *)0x650,
    (char *)0x6D0,
    (char *)0x750,
    (char *)0x7D0
};

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

void killEnemy(tEnemy * enemy)
{
    gShotVisible = false;
    gScore += (enemy->type == ENEMY_COMMODORE ? 10 : 100);
    enemy->type = ENEMY_NONE;
    playSound(70, 20);
    gNumEnemies--;
}

void addClearAndDrawEnemies(void)
{
    uint8_t enemiesToAdd = 0;
    uint8_t random;
    uint8_t x1;
    uint8_t x2;
    uint8_t x3;
    uint8_t x;
    uint8_t y;
    tEnemy * enemy;
    tEnemy * lastEnemy = &(gEnemies[MAX_ENEMIES]);
    
    random = rand() & 0xf;
    if (random == 0xf)
        enemiesToAdd = 3;
    else if (random > 8)
        enemiesToAdd = 2;
    else if (random > 2)
        enemiesToAdd = 1;
    
    x1 = rand() % MAX_X;
    do {
        x2 = rand() % MAX_X;
    } while (x2 == x1);
    do {
        x3 = rand() % MAX_X;
    } while ((x3 == x1) || (x3 == x2));
    
    for (enemy = gEnemies; enemy < lastEnemy; enemy++) {
        if (enemy->type != ENEMY_NONE) {
            x = enemy->x;
            y = enemy->y;
            *(gScreenAddrs[y] + x) = SPACE_CHAR;
            if ((gShotVisible) &&
                (gShotX == x) &&
                (gShotY == y)) {
                killEnemy(enemy);
            } else {
                y++;
                if (y >= MAX_Y) {
                    enemy->type = ENEMY_NONE;
                    gNumEnemies--;
                } else {
                    enemy->y = y;
                }
            }
        }
    }
    
    for (enemy = gEnemies; enemy < lastEnemy; enemy++) {
        if ((enemiesToAdd != 0) &&
            (enemy->type == ENEMY_NONE)) {
            gNumEnemies++;
            enemy->y = 1;
            enemy->type = (((rand() & 0xf) > 0xc) ? ENEMY_ATARI : ENEMY_COMMODORE);
            if (enemiesToAdd == 3) {
                enemy->x = x3;
            } else if (enemiesToAdd == 2) {
                enemy->x = x2;
            } else {
                enemy->x = x1;
            }
            enemiesToAdd--;
        }
        if (enemy->type != ENEMY_NONE) {
            x = enemy->x;
            y = enemy->y;
            if ((gShotVisible) &&
                (gShotX == x) &&
                (gShotY == y)) {
                killEnemy(enemy);
                *(gScreenAddrs[y] + x) = SPACE_CHAR;
            } else
                *(gScreenAddrs[y] + x) = enemy->type;
        }
    }
}

void gameOver(void)
{
    uint16_t count;
    char ch;
    
    *(gScreenAddrs[MAX_Y - 1] + gHeroX) = STAR_CHAR;
    
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
        ch = *(((char *)0x7d0) + gHeroX);
        if ((ch != SPACE_CHAR) &&
            (ch != HERO_CHAR))
            gameOver();
        
        *(gScreenAddrs[MAX_Y - 1] + gHeroX) = SPACE_CHAR;
        gHeroX += delta;
    }
    
    ch = *(((char *)0x7d0) + gHeroX);
    if ((ch != SPACE_CHAR) &&
        (ch != HERO_CHAR))
        gameOver();
    
    *(gScreenAddrs[MAX_Y - 1] + gHeroX) = HERO_CHAR;
}

void drawShot(void)
{
    if (!gShotVisible)
        return;
    
    if (gShotY < MAX_Y - 1)
        *(gScreenAddrs[gShotY] + gShotX) = SPACE_CHAR;
    
    if (gShotY == 0) {
        gShotVisible = false;
        return;
    }
    
    gShotY--;
    *(gScreenAddrs[gShotY] + gShotX) = SHOT_CHAR;
}

int main(void)
{
    initGame();
    displayInstructions();
    while (true) {
        gotoxy(0, 0);
        cprintf("  SCORE: %lu", gScore);
        addClearAndDrawEnemies();
        drawHero();
        drawShot();
        delay();
    }
    return 0;
}
