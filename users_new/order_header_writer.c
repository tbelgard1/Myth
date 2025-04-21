#include <stdio.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t order_count;
    uint32_t unused[81]; // 4 + 81*4 = 328 bytes
} order_file_header;
#pragma pack(pop)

int main() {
    order_file_header header;
    memset(&header, 0, sizeof(header));
    header.order_count = 0;

    FILE *f = fopen("orders.db", "wb");
    if (!f) {
        perror("Failed to create orders.db");
        return 1;
    }

    size_t written = fwrite(&header, sizeof(header), 1, f);
    if (written != 1) {
        perror("Failed to write header to orders.db");
        fclose(f);
        return 1;
    }

    fclose(f);
    printf("Successfully created orders.db with %zu-byte header.\n", sizeof(header));
    return 0;
}