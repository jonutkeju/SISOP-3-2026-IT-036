#include "arena.h"
#include <time.h>
#include <termios.h>
#include <unistd.h>

Shared *shm;
int shmid;

void clear() {
    printf("\033[2J\033[H");
}

void banner() {
    printf("\n==== ETERNAL ====\n\n");
}

void flush() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void attach() {
    shmid = shmget(SHM_KEY, sizeof(Shared), 0666);
    shm = (Shared *)shmat(shmid, NULL, 0);
}

int login_user(char *u, char *p) {
    for (int i = 0; i < shm->player_count; i++) {
        if (!strcmp(shm->players[i].username, u) &&
            !strcmp(shm->players[i].password, p) &&
            !shm->players[i].active) {

            shm->players[i].active = 1;
            return i;
        }
    }
    return -1;
}

void send_attack(int id) {
    Event e;
    e.type = 'A';
    e.from = id;
    e.to = (shm->p1 == id) ? shm->p2 : shm->p1;
    e.value = 10 + shm->players[id].wdb;
    e.timestamp = time(NULL);

    shm->events[shm->event_tail] = e;
    shm->event_tail = (shm->event_tail + 1) % 256;
}

void queue_player(int id) {
    shm->queue[shm->qsize++] = id;
}

void wait_match() {
    printf("\nSearching");

    while (!shm->battle_active) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }

    printf("\nMatch Found!\n");
}

void battle_view(int id) {
    while (shm->battle_active) sleep(1);

    int p1 = shm->p1;
    int p2 = shm->p2;

    char *a = shm->players[p1].username;
    char *b = (p2 == -1) ? "BOT" : shm->players[p2].username;

    clear();
    banner();

    printf("\n[BATTLE START]\n%s VS %s\n", a, b);
    printf("%s HP: %d | %s HP: %d\n",
        a, shm->hp1, b, shm->hp2);

    if (shm->winner == id)
        printf("\n>>> YOU WIN <<<\n");
    else
        printf("\n>>> YOU LOSE <<<\n");

    printf("\nPress ENTER...");
    getchar(); getchar();
}

int register_user(char *u, char *p) {
    for (int i = 0; i < shm->player_count; i++) {
        if (!strcmp(shm->players[i].username, u))
            return 0;
    }

    Player *pl = &shm->players[shm->player_count++];

    strcpy(pl->username, u);
    strcpy(pl->password, p);

    pl->gold = 150;
    pl->lvl = 1;
    pl->xp = 0;
    pl->wdb = 0;
    pl->active = 0;
    pl->weapon_owned = 0;
    pl->history_count = 0;

    return 1;
}

void armory(int id) {
    Player *p = &shm->players[id];

    int cost[7] = {0,100,300,600,1500,5000,33550336};
    int wdb[7]  = {0,5,15,30,60,150,999};

    int c;

    printf("\nARMORY\n");
    printf("1 Wood\n2 Iron\n3 Steel\n4 Demon\n5 God\n6 Jutsu\nChoice: ");
    scanf("%d", &c);
    flush();

    if (c >= 1 && c <= 6) {
        if (p->weapon_owned) {
            printf("\nAlready owned weapon!\n");
            sleep(2);
            return;
        }

        if (p->gold >= cost[c]) {
            p->gold -= cost[c];
            p->wdb = wdb[c];
            p->weapon_owned = 1;
            printf("\nWeapon equipped!\n");
        } else {
            printf("\nNot enough gold!\n");
        }

        sleep(2);
    }
}

void history(int id) {
    Player *p = &shm->players[id];

    printf("\nHISTORY\n");

    for (int i = 0; i < p->history_count; i++) {
        printf("%s\n", p->history[i]);
    }

    printf("\nPress ENTER...");
    getchar(); getchar();
}

void battle_mode(int id) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int p1 = shm->p1;
    int p2 = shm->p2;

    while (1) {
        system("clear");

        Player A = shm->players[p1];
        Player B = shm->players[p2];

        printf("========== BATTLE ==========\n");
        printf("%s  VS  %s\n\n", A.username, B.username);

        printf("%s\n", A.username);
        printf("Lvl: %d\n", A.lvl);
        printf("XP : %d\n", A.xp);
        printf("HP : %d\n", shm->hp1);
        printf("Weapon WDB: %d\n\n", A.wdb);

        printf("%s\n", B.username);
        printf("Lvl: %d\n", B.lvl);
        printf("XP : %d\n", B.xp);
        printf("HP : %d\n", shm->hp2);
        printf("Weapon WDB: %d\n\n", B.wdb);

        printf("----------------------------\n");
        printf("Press 'a' to attack\n");

        if (shm->hp1 <= 0 || shm->hp2 <= 0)
            break;

        char c = getchar();

        if (c == 'a') {
            Event e;

            e.type = 'A';
            e.from = id;
            e.to = (p1 == id) ? p2 : p1;
            e.value = 10 + shm->players[id].wdb;
            e.timestamp = time(NULL);

            shm->events[shm->event_tail] = e;
            shm->event_tail = (shm->event_tail + 1) % MAX_EVENT;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    system("clear");

    int win = (shm->hp1 > 0) ? p1 : p2;
    int lose = (win == p1) ? p2 : p1;

    printf("[BATTLE FINISHED] %s Wins, %s Loses\n",
        shm->players[win].username,
        shm->players[lose].username
    );

    printf("Press ENTER to continue...");
    getchar();
}

int main() {
    attach();

    int ch;
    char u[32], p[32];

    while (1) {
        clear();
        banner();

        printf("1 Register\n2 Login\n3 Exit\nChoice: ");
        scanf("%d", &ch);
        flush();

        if (ch == 1) {
            clear();
            banner();

            printf("REGISTER\nUsername: ");
            scanf("%s", u);
            flush();

            printf("Password: ");
            scanf("%s", p);
            flush();

            if (register_user(u, p))
                printf("\nAccount created!\n");
            else
                printf("\nUser exists!\n");

            sleep(2);
        }

        else if (ch == 2) {
            clear();
            banner();
        
            printf("LOGIN\nUsername: ");
            scanf("%s", u);
            flush();
        
            printf("Password: ");
            scanf("%s", p);
            flush();
        
            int id = login_user(u, p);
        
            if (id != -1) {
        
                while (1) {
        
                    if (shm->battle_active == 1 &&
                        (shm->p1 == id || shm->p2 == id)) {
        
                        battle_mode(id);
                        continue;
                    }
        
                    clear();
                    banner();
        
                    Player *pl = &shm->players[id];
        
                    printf("%s | Gold:%d | Lv:%d | XP:%d\n",
                        pl->username, pl->gold, pl->lvl, pl->xp);
        
                    int c;
        
                    printf("\n1 Battle\n2 Armory\n3 History\n4 Logout\nChoice: ");
                    scanf("%d", &c);
                    flush();
        
                    if (c == 1) {
                        queue_player(id);
                    }
                    else if (c == 2) {
                        armory(id);
                    }
                    else if (c == 3) {
                        history(id);
                    }
                    else break;
                }
            }
        }

        else break;
    }
}
