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
