#include <stdio.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t user_count;
    uint32_t unused[40];
} bungie_net_user_db_header;
#pragma pack(pop)

int main() {
    bungie_net_user_db_header hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.user_count = 0;

    FILE *f = fopen("blank_users.db", "wb");
    if (!f) {
        perror("Failed to create blank_users.db");
        return 1;
    }

    size_t written = fwrite(&hdr, sizeof(hdr), 1, f);
    if (written != 1) {
        perror("Failed to write header to blank_users.db");
        fclose(f);
        return 1;
    }

    fclose(f);
    printf("Successfully created blank_users.db with 164-byte header.\n");
    return 0;
}
