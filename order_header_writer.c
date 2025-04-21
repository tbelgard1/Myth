// order_header_writer.c
#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t order_count;
    uint32_t unused[81];
} order_file_header;
#pragma pack(pop)

int main() {
    FILE *file = fopen("ORDERS_DB", "wb");
    if (!file) return 1;

    order_file_header header = {0};

    size_t header_size = sizeof(header);
    fwrite(&header, header_size, 1, file);
    fclose(file);

    printf("ORDERS_DB written successfully (%zu bytes)\n", header_size);
    return 0;
}

