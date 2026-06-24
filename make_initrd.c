#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct initrd_header {
    uint32_t nfiles;
};

struct initrd_file_header {
    char name[64];
    uint32_t offset;
    uint32_t length;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: make_initrd file1 [file2 ...]\n");
        return 1;
    }

    uint32_t nfiles = argc - 1;
    struct initrd_header header;
    header.nfiles = nfiles;

    struct initrd_file_header headers[64];
    memset(headers, 0, sizeof(struct initrd_file_header) * 64);

    FILE *out = fopen("initrd.img", "wb");
    if (!out) {
        printf("Error creating initrd.img\n");
        return 1;
    }

    // Write global header
    fwrite(&header, sizeof(struct initrd_header), 1, out);

    // Calculate initial offset for file data
    uint32_t current_offset = sizeof(struct initrd_header) + sizeof(struct initrd_file_header) * nfiles;

    // Write file headers
    for (uint32_t i = 0; i < nfiles; i++) {
        FILE *stream = fopen(argv[i + 1], "rb");
        if (!stream) {
            printf("Error opening file %s\n", argv[i + 1]);
            return 1;
        }

        // Get file length
        fseek(stream, 0, SEEK_END);
        uint32_t length = ftell(stream);
        fseek(stream, 0, SEEK_SET);

        // Strip path from filename (simple implementation)
        char *name = strrchr(argv[i + 1], '/');
        if (name) name++;
        else name = argv[i + 1];

        strncpy(headers[i].name, name, 63);
        headers[i].offset = current_offset;
        headers[i].length = length;

        current_offset += length;
        fclose(stream);
        
        fwrite(&headers[i], sizeof(struct initrd_file_header), 1, out);
    }

    // Write file data
    for (uint32_t i = 0; i < nfiles; i++) {
        FILE *stream = fopen(argv[i + 1], "rb");
        unsigned char *buffer = (unsigned char *)malloc(headers[i].length);
        fread(buffer, 1, headers[i].length, stream);
        fwrite(buffer, 1, headers[i].length, out);
        fclose(stream);
        free(buffer);
    }

    fclose(out);
    printf("Created initrd.img with %d files.\n", nfiles);
    return 0;
}
