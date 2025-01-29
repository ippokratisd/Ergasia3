#include "e-shop.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BACKLOG 5

void init_catalog(products catalog[]) {
    int i;
    float value;
    for (i = 0; i < PRODUCTS_NUMBER; i++) {
        snprintf(catalog[i].description, sizeof(catalog[i].description), "Product-%d", i + 1);
        value = rand() % 500 + 1;
        catalog[i].price = value;
        catalog[i].item_count = INITIAL_STOCK;
        catalog[i].item_requests = 0;
        catalog[i].item_sold = 0;
        for (int j = 0; j < CUSTOMERS; j++) {
            snprintf(catalog[i].failed_customers[j], 50, "N/A");
        }
    }
}

void process_order(int client_socket, products catalog[]) {
    int product;
    read(client_socket, &product, sizeof(int));
    catalog[product].item_requests++;
    char response[350];

    if (catalog[product].item_count > 0) {
        catalog[product].item_count--;
        catalog[product].item_sold++;
        snprintf(response, sizeof(response), "Order successful! %s purchased for %.2f EUR", catalog[product].description, catalog[product].price);
    } else {
        snprintf(response, sizeof(response), "Order failed! %s is out of stock", catalog[product].description);
    }
    write(client_socket, response, strlen(response) + 1);
}

void print_catalog_summary(products catalog[]) {
    printf("\nFinal Catalog Summary:\n");
    for (int i = 0; i < PRODUCTS_NUMBER; i++) {
        printf("%s - Price: %.2f EUR, Sold: %d, Requested: %d, Stock Left: %d\n",
               catalog[i].description, catalog[i].price, catalog[i].item_sold,
               catalog[i].item_requests, catalog[i].item_count);
    }
}

void info(products catalog[]) {
    printf("\nDetailed Sales Report:\n");
    for (int i = 0; i < PRODUCTS_NUMBER; i++) {
        printf("%s - Sold: %d, Requests: %d, Stock Left: %d\n",
               catalog[i].description, catalog[i].item_sold,
               catalog[i].item_requests, catalog[i].item_count);
        printf("Failed Customers: ");
        for (int j = 0; j < CUSTOMERS; j++) {
            if (strcmp(catalog[i].failed_customers[j], "N/A") != 0) {
                printf("%s ", catalog[i].failed_customers[j]);
            }
        }
        printf("\n");
    }
}

void server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    products catalog[PRODUCTS_NUMBER];
    init_catalog(catalog);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, BACKLOG);
    printf("Server listening on port %d...\n", PORT);

    int completed_clients = 0;
    while (completed_clients < CUSTOMERS) {
        addr_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (fork() == 0) {
            close(server_fd);
            for (int i = 0; i < 10; i++) {
                process_order(client_fd, catalog);
                sleep(1);
            }
            close(client_fd);
            exit(0);
        }
        close(client_fd);
        completed_clients++;
    }

    // Wait for all client processes to finish
    while (wait(NULL) > 0);

    // Print final summary after all orders are processed
    print_catalog_summary(catalog);
    info(catalog);
    close(server_fd);
}

void client() {
    srand(time(NULL) ^ getpid()); // Ensure different random numbers for each process
    int sockfd;
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    for (int i = 0; i < 10; i++) {
        int product = rand() % PRODUCTS_NUMBER;
        write(sockfd, &product, sizeof(int));
        char response[350];
        read(sockfd, response, sizeof(response));
        printf("Customer: %s\n", response);
        sleep(1);
    }
    close(sockfd);
}

int main() {
    srand(time(NULL));
    if (fork() == 0) {
        sleep(1);
        for (int i = 0; i < CUSTOMERS; i++) {
            if (fork() == 0) {
                client();
                exit(0);
            }
        }
        exit(0);
    } else {
        server();
    }
    return 0;
}

