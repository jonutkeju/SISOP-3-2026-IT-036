#include "arena.h"

Shared *shm;
int shmid;

int hp_calc(int xp) {
    return BASE_HP + (xp / 10);
}

int dmg_calc(int xp, int wdb) {
    return BASE_DMG + (xp / 50) + wdb;
}

void init() {
    shmid = shmget(SHM_KEY, sizeof(Shared), IPC_CREAT | 0666);
    shm = (Shared *)shmat(shmid, NULL, 0);

    if (shm->player_count == 0) {
        shm->player_count = 0;
        shm->qsize = 0;
        shm->event_head = 0;
        shm->event_tail = 0;
        shm->battle_active = 0;
    }
}

void reward(int win, int lose) {
    char buf[128];

    shm->players[win].xp += 50;
    shm->players[win].gold += 120;

    shm->players[lose].xp += 15;
    shm->players[lose].gold += 30;

    snprintf(buf, sizeof(buf), "WIN vs %s +50XP +120G", shm->players[lose].username);
    strcpy(shm->players[win].history[shm->players[win].history_count++], buf);

    snprintf(buf, sizeof(buf), "LOSE vs %s +15XP +30G", shm->players[win].username);
    strcpy(shm->players[lose].history[shm->players[lose].history_count++], buf);
}

void process_event(Event e) {
    if (e.type == 'A') {
        if (e.to == shm->p2) {
            shm->hp2 -= e.value;
        } else {
            shm->hp1 -= e.value;
        }
    }

    if (shm->hp1 < 0) shm->hp1 = 0;
    if (shm->hp2 < 0) shm->hp2 = 0;
}

void push_event(Event e) {
    shm->events[shm->event_tail] = e;
    shm->event_tail = (shm->event_tail + 1) % MAX_EVENT;
}

void fight_loop() {
    shm->battle_active = 1;

    shm->hp1 = hp_calc(shm->players[shm->p1].xp);
    shm->hp2 = hp_calc(shm->players[shm->p2].xp);

    while (shm->hp1 > 0 && shm->hp2 > 0) {

        while (shm->event_head != shm->event_tail) {
            Event e = shm->events[shm->event_head];
            shm->event_head = (shm->event_head + 1) % MAX_EVENT;

            process_event(e);
        }

        usleep(100000);
    }

    int win = (shm->hp1 > 0) ? shm->p1 : shm->p2;
    int lose = (win == shm->p1) ? shm->p2 : shm->p1;

    shm->winner = win;
    reward(win, lose);

    shm->battle_active = 0;
}

void fight(int a, int b) {
    shm->battle_active = 1;

    shm->p1 = a;
    shm->p2 = b;

    shm->hp1 = BASE_HP + (shm->players[a].xp / 10);
    shm->hp2 = (b == -1) ? BASE_HP : (BASE_HP + (shm->players[b].xp / 10));

    shm->event_head = 0;
    shm->event_tail = 0;

    printf("[BATTLE START] %s VS %s\n",
        shm->players[a].username,
        (b == -1) ? "BOT" : shm->players[b].username
    );

    while (shm->hp1 > 0 && shm->hp2 > 0) {

        while (shm->event_head != shm->event_tail) {
            Event e = shm->events[shm->event_head];
            shm->event_head = (shm->event_head + 1) % MAX_EVENT;

            if (e.type == 'A') {
                if (e.to == shm->p1) {
                    shm->hp1 -= e.value;
                } else if (e.to == shm->p2) {
                    shm->hp2 -= e.value;
                }
            }
        }

        if (shm->hp1 < 0) shm->hp1 = 0;
        if (shm->hp2 < 0) shm->hp2 = 0;

        usleep(100000);
    }

    int win = (shm->hp1 > 0) ? a : b;
    int lose = (win == a) ? b : a;

    shm->winner = win;

    printf("[BATTLE FINISHED] %s Wins, %s Loses\n",
        shm->players[win].username,
        shm->players[lose].username
    );

    reward(win, lose);

    shm->battle_active = 0;
}

void *matchmaker(void *arg) {
    int timer = 0;
    int last_qsize = 0;

    while (1) {
        sleep(1);

        if (shm->battle_active)
            continue;

        // reset timer hanya kalau state berubah
        if (shm->qsize != last_qsize) {
            last_qsize = shm->qsize;
            timer = 0;
        }

        // PvP match
        if (shm->qsize >= 2) {
            int a = shm->queue[0];
            int b = shm->queue[1];

            for (int i = 2; i < shm->qsize; i++)
                shm->queue[i - 2] = shm->queue[i];

            shm->qsize -= 2;
            timer = 0;

            fight(a, b);
        }

        // BOT match
        else if (shm->qsize == 1) {
            timer++;

            if (timer >= 35) {
                int a = shm->queue[0];

                shm->qsize = 0;
                timer = 0;

                fight(a, -1);
            }
        }
    }
}

int main() {
    init();
    printf("Orion ready (PID: %d)\n", getpid());
    matchmaker(NULL);
}
