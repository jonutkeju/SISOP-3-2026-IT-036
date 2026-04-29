#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 0x00001234
#define MAX_USER 32
#define MAX_EVENT 256

#define BASE_HP 100
#define BASE_DMG 10

typedef struct {
    char type;
    int from;
    int to;
    int value;
    long timestamp;
    char log[128];
} Event;

typedef struct {
    char username[MAX_USER];
    char password[MAX_USER];

    int gold;
    int lvl;
    int xp;
    int wdb;

    int active;
    int weapon_owned;

    char history[10][128];
    int history_count;
    int in_queue;
    char weapon_name[64];
} Player;

typedef struct {
    Player players[100];
    int player_count;

    int queue[100];
    int qsize;

    int p1;
    int p2;

    int hp1;
    int hp2;

    int winner;

    Event events[MAX_EVENT];
    int event_head;
    int event_tail;

    int battle_active;
} Shared;

#endif
