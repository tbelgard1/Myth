// generate_order_database.c

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FILE_PATH "./spaghetti/users_new/order_database.dat"

// Minimal fake struct for testing
typedef struct {
    uint64_t order_id;
    char name[32];
} __attribute__((packed)) dummy_order_t;

int main() {
    FILE *fp = fopen(FILE_PATH, "wb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    // --- Header ---
    bungie_net_order_database_header header = {
        .signature = {'R','D','R','O'},
        .version = 0,
        .order_count = 1
    };
    fwrite(&header, sizeof(header), 1, fp);

    // --- Dummy Order ---
    dummy_order_t order = {0};
    order.order_id = 1001;
    strncpy(order.name, "Test Order", sizeof(order.name));
    fwrite(&order, sizeof(order), 1, fp);

    fclose(fp);
    printf("✅ Wrote dummy order database to: %s\n", FILE_PATH);
    return 0;
}
