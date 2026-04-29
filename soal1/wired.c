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
