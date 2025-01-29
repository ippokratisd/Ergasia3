#define CUSTOMERS 5
#define PRODUCTS_NUMBER 20
#define INITIAL_STOCK 2
#define PORT 8080

#include <netinet/in.h>

typedef struct {
    int item_count;
    float price;
    char description[100];
    int item_requests;
    int item_sold;
    char failed_customers[CUSTOMERS][50];
} products;

void init_catalog(products catalog[]);
void process_order(int client_socket, products catalog[]);
void info(products catalog[]);
void server();
void client();
