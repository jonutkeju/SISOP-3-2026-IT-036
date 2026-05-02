# SISOP-3-2026-IT-036

| Modul 3 |     Identitas Praktikan     |
|---------|-----------------------------|
| Nama    | Jonathan Steven Tjahjaputra |
| NRP     | 5027251036                  |
| Kelas   | Sistem Operasi B            |
| Asisten | SCRA                        |

## Struktur Repository (tanpa assets)
```
.
├── soal1
│   ├── navi.c
│   ├── protocol.h
│   └── wired.c
└── soal2
    ├── Makefile
    ├── arena.h
    ├── eternal.c
    └── orion.c
```

## 1. [soal1] : Present Day, Present Time

### A. Deskripsi

Program diminta untuk membuat sistem komunikasi jaringan bernama **The Wired** menggunakan bahasa `C` dan socket TCP. Sistem terdiri dari:
- server (`wired`)
- client (`navi`)

Server harus mampu menangani banyak client, broadcast pesan, autentikasi admin, RPC command, serta logging aktivitas ke file `history.log`.

<br>

### B. Setup

1. Membuat direktori project.
```sh
mkdir soal_1
cd soal_1
```

2. Membuat file source code.
```sh
micro protocol.h
micro wired.c
micro navi.c
```
<br><br>

### C. Alur Pengerjaan

1. Fase awal pembangunan memerlukan koneksi yang stabil untuk memastikan NAVI terdaftar di jaringan pusat.

(`protocol.h`)
```c
#define IP_ADDRESS "127.0.0.1"
#define PORT 8080
```

(`wired.c`)
```c
int server_fd = socket(AF_INET, SOCK_STREAM, 0);

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

listen(server_fd, 10);
```

(`navi.c`)
```c
int sock = socket(AF_INET, SOCK_STREAM, 0);

connect(
    sock,
    (struct sockaddr *)&server_addr,
    sizeof(server_addr)
);
```

- `protocol.h` digunakan untuk menyimpan konfigurasi IP dan port.
- `socket()` digunakan untuk membuat socket TCP.
- `bind()` menghubungkan socket server ke IP dan port tertentu.
- `listen()` membuat server siap menerima koneksi.
- `connect()` digunakan agar client dapat terhubung ke server.

<br>

---

2. Unit NAVI harus mampu menjalankan dua fungsi secara asinkron.
(`navi.c`)
```c
void *receive_message(void *arg) {

    char buffer[1024];

    while (1) {

        memset(buffer, 0, sizeof(buffer));

        recv(sock, buffer, sizeof(buffer), 0);

        printf("%s", buffer);
    }
}
```

(`navi.c`)
```c
pthread_t tid;

pthread_create(
    &tid,
    NULL,
    receive_message,
    NULL
);
```

(`navi.c`)
```c
while (1) {

    fgets(buffer, sizeof(buffer), stdin);

    send(sock, buffer, strlen(buffer), 0);
}
```

- `pthread_create()` digunakan untuk membuat thread baru tanpa `fork()`.
- Thread `receive_message()` bertugas menerima pesan dari server secara terus-menerus.
- Loop utama tetap berjalan untuk menerima input user dan mengirimkannya ke server.

<br>

---

3. Server pusat The Wired dituntut untuk memiliki skalabilitas tinggi.
(`wired.c`)
```c
fd_set readfds;

FD_ZERO(&readfds);

FD_SET(server_fd, &readfds);

select(
    max_fd + 1,
    &readfds,
    NULL,
    NULL,
    NULL
);
```

(`wired.c`)
```c
if (FD_ISSET(server_fd, &readfds)) {

    int new_socket = accept(
        server_fd,
        (struct sockaddr *)&client_addr,
        &client_len
    );
}
```

(`wired.c`)
```c
if (FD_ISSET(clients[i], &readfds)) {

    recv(
        clients[i],
        buffer,
        sizeof(buffer),
        0
    );
}
```

(`navi.c`)
```c
if (strcmp(buffer, "/exit\n") == 0) {

    printf("[System] Disconnecting from The Wired...\n");

    close(sock);

    exit(0);
}
```

- `select()` digunakan agar server dapat memantau banyak socket sekaligus.
- `FD_ISSET()` digunakan untuk mendeteksi aktivitas socket.
- Server dapat membedakan koneksi baru dan pesan client.
- `/exit` digunakan untuk disconnect client secara bersih.

<br>

---

4. Setiap entitas yang memasuki The Wired memerlukan identitas digital.

(`navi.c`)
```c
printf("Enter your name: ");

fgets(name, sizeof(name), stdin);

name[strcspn(name, "\n")] = 0;

send(sock, name, strlen(name), 0);
```

(`wired.c`)
```c
for (int i = 0; i < 10; i++) {

    if (strcmp(usernames[i], name) == 0) {

        taken = 1;

        break;
    }
}
```

(`wired.c`)
```c
if (taken) {

    send(
        new_socket,
        "TAKEN",
        5,
        0
    );
}
```

(`wired.c`)
```c
strcpy(usernames[i], name);
```

- Client diminta memasukkan username sebelum masuk ke server.
- `strcmp()` digunakan untuk mengecek duplicate username.
- Server menolak username yang sudah digunakan.
- Username valid disimpan ke array `usernames`.

<br>

---

5. Distribusi informasi di dalam The Wired harus bersifat kolektif dan menyeluruh.

(`wired.c`)
```c
sprintf(
    message,
    "[%s]: %s",
    usernames[i],
    buffer
);
```

(`wired.c`)
```c
for (int j = 0; j < 10; j++) {

    if (
        clients[j] > 0 &&
        j != i
    ) {

        send(
            clients[j],
            message,
            strlen(message),
            0
        );
    }
}
```

- `sprintf()` digunakan untuk membuat format pesan chat.
- Server mengirim pesan ke seluruh client selain pengirim.
- Mekanisme ini disebut broadcast.

<br>

---

6. Selain ruang obrolan, The Wired harus menyediakan prosedur jarak jauh.

(`wired.c`)
```c
if (
    strcmp(name, "The Knights") == 0
)
```

(`wired.c`)
```c
recv(
    new_socket,
    password,
    sizeof(password),
    0
);
```

(`wired.c`)
```c
if (
    strcmp(password, "when yah") == 0
)
```

(`wired.c`)
```c
if (
    strcmp(buffer, "RPC_GET_USERS\n") == 0
)
```

(`wired.c`)
```c
long uptime =
    time(NULL) - start_time;
```

(`wired.c`)
```c
close(server_fd);

return 0;
```

- Username `The Knights` digunakan sebagai admin server.
- Server meminta password admin.
- RPC command digunakan untuk menjalankan fungsi internal server.
- Admin dapat melihat jumlah user aktif.
- Admin dapat melihat uptime server.
- Admin dapat melakukan emergency shutdown.

<br>

---

7. Setiap transmisi di dalam The Wired harus tercatat secara permanen.

(`wired.c`)
```c
void write_log(
    const char *category,
    const char *message
)
```

(`wired.c`)
```c
FILE *log =
    fopen("history.log", "a");
```

(`wired.c`)
```c
time_t now = time(NULL);

struct tm *t =
    localtime(&now);
```

(`wired.c`)
```c
fprintf(
    log,
    "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s]\n",
    ...
);
```

(`wired.c`)
```c
write_log(
    "User",
    message
);
```

(`wired.c`)
```c
write_log(
    "Admin",
    "RPC_GET_USERS"
);
```

- `history.log` digunakan untuk menyimpan seluruh aktivitas server.
- `fopen(..., "a")` digunakan agar log baru ditambahkan tanpa menghapus log lama.
- `time()` dan `localtime()` digunakan untuk membuat timestamp.
- `fprintf()` digunakan untuk menulis format log.
- Aktivitas user dan admin dicatat secara permanen.
<br><br>

### D. Output
Placeholder

### E. Markdown C
<details>
<summary><strong>navi.c</strong></summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "protocol.h"

int sock;

void *receive_message(void *arg) {
    (void *)arg;

    char buffer[1024];

    while (1) {

        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(sock, buffer, sizeof(buffer), 0);

        if (bytes <= 0) {

            printf("\n[System] Disconnecting from The Wired...\n");

            exit(0);
        }

        printf("%s", buffer);

        if (strcmp(buffer, "EMERGENCY SHUTDOWN\n") == 0) {

            printf("[System] Disconnecting from The Wired...\n");

            close(sock);

            exit(0);
        }
    }

    return NULL;
}

int main() {

    char ip[] = IP_ADDRESS;
    int port = PORT;

    int admin_mode = 0;

    while (1) {

        sock = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in server_addr;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip);

        connect(
            sock,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        );

        char name[100];

        printf("Enter your name: ");

        fgets(name, sizeof(name), stdin);

        send(sock, name, strlen(name), 0);

        name[strcspn(name, "\n")] = 0;

        if (strcmp(name, "The Knights") == 0) {

            char password[100];

            printf("Enter Password: ");

            fgets(password, sizeof(password), stdin);

            send(sock, password, strlen(password), 0);
        }

        char response[100];

        memset(response, 0, sizeof(response));

        recv(sock, response, sizeof(response), 0);

        if (strcmp(response, "OK") == 0) {

            printf(
                "--- Welcome to The Wired, %s ---\n",
                name
            );

            break;
        }

        if (strcmp(response, "ADMIN_OK") == 0) {

            admin_mode = 1;

            printf(
                "\n[System] Authentication Successful. Granted Admin privileges.\n"
            );

            break;
        }

        printf(
            "[System] The identity '%s' is already synchronized in The Wired.\n",
            name
        );

        close(sock);
    }

    pthread_t tid;

    pthread_create(&tid, NULL, receive_message, NULL);

    if (admin_mode) {

        while (1) {

            char cmd[10];

            printf(
                "\n=== THE KNIGHTS CONSOLE ===\n"
                "1. Check Active Entities (Users)\n"
                "2. Check Server Uptime\n"
                "3. Execute Emergency Shutdown\n"
                "4. Disconnect\n"
                "Command >> "
            );

            fgets(cmd, sizeof(cmd), stdin);

            if (strcmp(cmd, "1\n") == 0) {

                send(
                    sock,
                    "RPC_GET_USERS\n",
                    15,
                    0
                );

                sleep(1);
            }

            else if (strcmp(cmd, "2\n") == 0) {

                send(
                    sock,
                    "RPC_GET_UPTIME\n",
                    16,
                    0
                );

                sleep(1);
            }

            else if (strcmp(cmd, "3\n") == 0) {

                send(
                    sock,
                    "RPC_SHUTDOWN\n",
                    13,
                    0
                );

                printf(
                    "[System] Disconnecting from The Wired...\n"
                );

                break;
            }

            else if (strcmp(cmd, "4\n") == 0) {

                send(sock, "/exit\n", 6, 0);

                printf(
                    "[System] Disconnecting from The Wired...\n"
                );

                break;
            }
        }
    }

    else {

        char buffer[1024];

        while (1) {

            fgets(buffer, sizeof(buffer), stdin);

            send(sock, buffer, strlen(buffer), 0);

            if (strcmp(buffer, "/exit\n") == 0) {

                printf(
                    "[System] Disconnecting from The Wired...\n"
                );

                break;
            }
        }
    }

    close(sock);

    return 0;
}
```
</details>


<details>
<summary><strong>wired.c</strong></summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#include "protocol.h"

void write_log(const char *category, const char *message) {

    FILE *log = fopen("history.log", "a");

    time_t now = time(NULL);

    struct tm *t = localtime(&now);

    fprintf(
        log,
        "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s]\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        category,
        message
    );

    fclose(log);
}

int main() {

    time_t start_time = time(NULL);

    write_log("System", "SERVER ONLINE");

    char ip[] = IP_ADDRESS;
    int port = PORT;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_fd, 10);

    int clients[10] = {0};

    char usernames[10][100];

    int is_admin[10] = {0};

    fd_set readfds;

    while (1) {

        FD_ZERO(&readfds);

        FD_SET(server_fd, &readfds);

        int max_fd = server_fd;

        for (int i = 0; i < 10; i++) {

            if (clients[i] > 0) {

                FD_SET(clients[i], &readfds);

                if (clients[i] > max_fd) {
                    max_fd = clients[i];
                }
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {

            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int new_socket = accept(
                server_fd,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            char name[100];

            memset(name, 0, sizeof(name));

            recv(new_socket, name, sizeof(name), 0);

            name[strcspn(name, "\n")] = 0;

            int taken = 0;

            for (int i = 0; i < 10; i++) {

                if (strcmp(usernames[i], name) == 0) {

                    taken = 1;

                    break;
                }
            }

            if (taken) {

                send(new_socket, "TAKEN", 5, 0);

                close(new_socket);
            }

            else {

                int admin_flag = 0;

                if (strcmp(name, "The Knights") == 0) {

                    char password[100];

                    recv(new_socket, password, sizeof(password), 0);

                    password[strcspn(password, "\n")] = 0;

                    if (strcmp(password, "when yah") == 0) {

                        admin_flag = 1;

                        send(new_socket, "ADMIN_OK", 8, 0);
                    }

                    else {

                        send(new_socket, "TAKEN", 5, 0);

                        close(new_socket);

                        continue;
                    }
                }

                else {

                    send(new_socket, "OK", 2, 0);
                }

                for (int i = 0; i < 10; i++) {

                    if (clients[i] == 0) {

                        clients[i] = new_socket;

                        strcpy(usernames[i], name);

                        is_admin[i] = admin_flag;

                        char logmsg[200];

                        sprintf(
                            logmsg,
                            "User '%s' connected",
                            name
                        );

                        write_log("System", logmsg);

                        break;
                    }
                }
            }
        }

        for (int i = 0; i < 10; i++) {

            int client_socket = clients[i];

            if (client_socket > 0 &&
                FD_ISSET(client_socket, &readfds)) {

                char buffer[1024];

                memset(buffer, 0, sizeof(buffer));

                int bytes = recv(
                    client_socket,
                    buffer,
                    sizeof(buffer),
                    0
                );

                if (bytes <= 0 ||
                    strcmp(buffer, "/exit\n") == 0) {

                    char logmsg[200];

                    sprintf(
                        logmsg,
                        "User '%s' disconnected",
                        usernames[i]
                    );

                    write_log("System", logmsg);

                    close(client_socket);

                    clients[i] = 0;

                    is_admin[i] = 0;

                    memset(usernames[i], 0, sizeof(usernames[i]));
                }

                else if (is_admin[i]) {

                    if (strcmp(buffer, "RPC_GET_USERS\n") == 0) {

                        write_log(
                            "Admin",
                            "RPC_GET_USERS"
                        );

                        int count = 0;

                        for (int j = 0; j < 10; j++) {

                            if (clients[j] > 0 &&
                                !is_admin[j]) {

                                count++;
                            }
                        }

                        char msg[100];

                        sprintf(
                            msg,
                            "Active Users: %d\n",
                            count
                        );

                        send(
                            client_socket,
                            msg,
                            strlen(msg),
                            0
                        );
                    }

                    else if (
                        strcmp(buffer, "RPC_GET_UPTIME\n") == 0
                    ) {

                        write_log(
                            "Admin",
                            "RPC_GET_UPTIME"
                        );

                        long uptime =
                            time(NULL) - start_time;

                        char msg[100];

                        sprintf(
                            msg,
                            "Server Uptime: %ld seconds\n",
                            uptime
                        );

                        send(
                            client_socket,
                            msg,
                            strlen(msg),
                            0
                        );
                    }

                    else if (
                        strcmp(buffer, "RPC_SHUTDOWN\n") == 0
                    ) {

                        write_log(
                            "Admin",
                            "RPC_SHUTDOWN"
                        );

                        write_log(
                            "System",
                            "EMERGENCY SHUTDOWN INITIATED"
                        );

                        for (int j = 0; j < 10; j++) {

                            if (clients[j] > 0) {

                                send(
                                    clients[j],
                                    "EMERGENCY SHUTDOWN\n",
                                    21,
                                    0
                                );

                                close(clients[j]);
                            }
                        }

                        close(server_fd);

                        return 0;
                    }
                }

                else {

                    char message[1200];

					buffer[strcspn(buffer, "\n")] = 0;
                    
                    sprintf(
                        message,
                        "[%s]: %s",
                        usernames[i],
                        buffer
                    );

                    write_log("User", message);

                    for (int j = 0; j < 10; j++) {

                        if (clients[j] > 0 &&
                            j != i &&
                            !is_admin[j]) {

                            send(
                                clients[j],
                                message,
                                strlen(message),
                                0
                            );
                        }
                    }
                }
            }
        }
    }

    close(server_fd);

    return 0;
}
```
</details>
<br>

### F. Kendala & Revisi
Tidak ada kendala dan revisi pada `soal1`
<br><br>

## [soal2] : The Battle of Eterion

### A. Deskripsi

Program diminta untuk membuat sistem arena pertempuran realtime bernama **Eterion** menggunakan bahasa `C` dengan komunikasi berbasis IPC. Sistem terdiri dari:

* server (`orion`)
* client (`eternal`)

Komunikasi antar proses dilakukan menggunakan:

* Shared Memory
* Message Queue

Sistem harus mendukung:

* register dan login akun
* matchmaking realtime
* battle realtime
* sistem weapon
* combat log
* history battle
* persistence data
* anti race condition

<br>

### B. Setup

1. Membuat struktur repository.

```sh
mkdir soal2
cd soal2
```

2. Membuat file source code.

```sh
micro arena.h
micro eternal.c
micro orion.c
micro Makefile
```

3. Mengisi `Makefile`.
   (`Makefile`)

```makefile
CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -lrt

all: server client

server: orion.c arena.h
	$(CC) $(CFLAGS) orion.c -o orion $(LDFLAGS)

client: eternal.c arena.h
	$(CC) $(CFLAGS) eternal.c -o eternal $(LDFLAGS)

clean:
	rm -f orion eternal

clear_ipc:
	ipcs -m | grep 0x00001234 | awk '{print $$2}' | xargs -r ipcrm -m
	ipcs -q | grep 0x00005678 | awk '{print $$2}' | xargs -r ipcrm -q
	ipcs -s | grep 0x00009012 | awk '{print $$2}' | xargs -r ipcrm -s
```

4. Compile program menggunakan Makefile.

```sh
make
```

5. Menghapus IPC.

```sh
make clear_ipc
```

<br><br>

### C. Alur Pengerjaan

1. Arena pertempuran mulai disiapkan dengan membuat struktur repository serta konfigurasi sistem IPC.

(`arena.h`)

```c
#define SHM_KEY 0x00001234
#define MSG_KEY 0x00005678
#define SEM_KEY 0x00009012

#define BASE_HP 100
#define BASE_DMG 10
#define MAX_EVENT 256
```

(`arena.h`)

```c
typedef struct {
    char username[32];
    char password[32];

    int gold;
    int lvl;
    int xp;

    int wdb;
    int active;
    int in_queue;
} Player;
```

(`arena.h`)

```c
typedef struct {
    Player players[100];

    int p1;
    int p2;

    int hp1;
    int hp2;

    int battle_active;
} Shared;
```

* `arena.h` digunakan sebagai pusat konfigurasi arena.
* Shared Memory digunakan untuk komunikasi realtime.
* Struct digunakan untuk menyimpan data prajurit dan battle.

<br>

---

2. Membuat gerbang awal arena dengan menu utama pada `eternal` serta server `orion`.

(`orion.c`)

```c
printf(
    "Orion is ready (PID: %d)\n",
    getpid()
);
```

(`eternal.c`)

```c
printf("1. Register\n");
printf("2. Login\n");
printf("3. Exit\n");
printf("Choice: ");
```

(`eternal.c`)

```c
if (shmid == -1) {

    printf(
        "Orion are you there?\n"
    );

    return 0;
}
```

* `orion` bertindak sebagai server arena.
* `eternal` bertindak sebagai client.
* Hubungan antar program berjalan menggunakan IPC.

<br>

---

3. Komunikasi antara `orion` dan `eternal` dilakukan menggunakan Shared Memory dan Message Queue.

(`orion.c`)

```c
shmid = shmget(
    SHM_KEY,
    sizeof(Shared),
    IPC_CREAT | 0666
);
```

(`orion.c`)

```c
shm = shmat(
    shmid,
    NULL,
    0
);
```

(`eternal.c`)

```c
msgid = msgget(
    MSG_KEY,
    0666
);
```

* Shared Memory digunakan untuk pertukaran data realtime.
* Message Queue digunakan untuk komunikasi event.

<br>

---

4. Membuat sistem Register dan Login dengan data yang bersifat persistent.

(`eternal.c`)

```c
printf("CREATE ACCOUNT\n");

printf("Username: ");
scanf("%s", username);

printf("Password: ");
scanf("%s", password);
```

(`orion.c`)

```c
if (
    strcmp(
        shm->players[i].username,
        username
    ) == 0
)
```

(`orion.c`)

```c
strcpy(
    shm->players[id].username,
    username
);
```

(`orion.c`)

```c
shm->players[id].gold = 150;
shm->players[id].lvl = 1;
shm->players[id].xp = 0;
```

(`orion.c`)

```c
if (
    shm->players[id].active
)
```

* Username harus unik.
* Akun yang sedang login tidak dapat digunakan kembali.
* Semua akun memiliki data awal:

  * Gold : 150
  * Lvl : 1
  * XP : 0

<br>

---

5. Setelah login, prajurit memasuki menu utama arena.

(`eternal.c`)

```c
printf(
    "%s | Gold:%d | Lv:%d | XP:%02d\n",
    player->username,
    player->gold,
    player->lvl,
    player->xp
);
```

(`eternal.c`)

```c
printf("1. Battle\n");
printf("2. Armory\n");
printf("3. History\n");
printf("4. Logout\n");
```

* UI status menampilkan:

  * Nama
  * Gold
  * Level
  * XP

<br>

---

6. Pilihan Battle akan memasuki matchmaking realtime.

(`orion.c`)

```c
if (shm->qsize >= 2)
```

(`orion.c`)

```c
fight(player1, player2);
```

(`orion.c`)

```c
if (wait_time >= 35)
```

(`orion.c`)

```c
fight(player, BOT_ID);
```

* Matchmaking mempertemukan dua prajurit.
* Jika tidak menemukan lawan selama 35 detik maka melawan bot.
* Player yang sedang battle tidak dapat masuk queue kembali.

<br>

---

7. Sistem pertempuran berjalan realtime dan bukan turn-based.

(`eternal.c`)

```c
if (c == 'a')
```

(`eternal.c`)

```c
send_attack(id);
```

(`orion.c`)

```c
damage =
    BASE_DMG +
    (xp / 50) +
    wdb;
```

(`orion.c`)

```c
health =
    BASE_HP +
    (xp / 10);
```

(`eternal.c`)

```c
printf(
    "%s VS %s\n",
    p1->username,
    p2->username
);
```

(`eternal.c`)

```c
print_last_5_events();
```

* Battle berjalan realtime.
* Tombol `a` digunakan untuk menyerang.
* UI battle menampilkan:

  * User VS User
  * HP
  * Weapon
  * Level
  * Combat Log
* Cooldown attack `a` adalah 1 detik.

<br>

---

8. Sistem reward dan perhitungan status dilakukan setelah battle selesai.

(`orion.c`)

```c
winner->xp += 50;
winner->gold += 120;
```

(`orion.c`)

```c
loser->xp += 15;
loser->gold += 30;
```

(`orion.c`)

```c
while (xp >= 100) {

    xp -= 100;

    lvl++;
}
```

* Winner mendapat:

  * +50 XP
  * +120 Gold
* Loser mendapat:

  * +15 XP
  * +30 Gold
* Level bertambah setiap XP mencapai 100.

<br>

---

9. Sistem Armory digunakan untuk membeli dan menggunakan weapon.

(`eternal.c`)

```c
printf(
    "1. Wood Sword - 100 Gold - +5 WDB\n"
);
```

(`orion.c`)

```c
player->gold -= price;
player->wdb += bonus;
```

(`orion.c`)

```c
damage = damage * 3;
```

(`orion.c`)

```c
if (
    strcmp(
        weapon_name,
        "Coding-di-Kertas Jutsu"
    ) == 0
)
```

* Weapon dapat dibeli menggunakan Gold.
* Hanya satu weapon yang dapat digunakan.
* Attack `u` memberikan damage 3x lipat.
* Weapon spesial dapat langsung membunuh lawan.

<br>

---

10. Sistem History menyimpan seluruh hasil matchmaking dan battle.

(`eternal.c`)

```c
printf(
    "| Time | Enemy | Result | XP |\n"
);
```

(`orion.c`)

```c
sprintf(
    player->history[i],
    "%s %s WIN +50XP",
    time_string,
    enemy_name
);
```

(`orion.c`)

```c
pthread_mutex_lock(&lock);
```

(`orion.c`)

```c
pthread_mutex_unlock(&lock);
```

* History menampilkan:

  * waktu selesai battle
  * nama lawan
  * hasil pertandingan
  * XP yang diperoleh
  * Mutex digunakan untuk mencegah race condition.

### D. Output
Placeholder

### E. Markdown
<details>
<summary><strong>arena.h</strong></summary>

```h
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
```

</details>

<details>
<summary><strong>eternal.c</strong></summary>

```c
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
```

</details>

<details>
<summary><strong>orion.c</strong></summary>

```c
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
```
