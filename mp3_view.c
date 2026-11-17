
#include "mp3_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void view(struct MP3 *mp3)
{
    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if (mp3->org_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to open MP3 file\n");
        return;
    }

    printf("MP3 file opened successfully\n");

    // Skip 10-byte ID3 header
    if (fseek(mp3->org_mp3_fptr, 10, SEEK_SET) != 0)
    {
        printf("ERROR: Unable to skip ID3 header\n");
        fclose(mp3->org_mp3_fptr);
        return;
    }

    printf("ID3 header skipped successfully\n");

    unsigned char frame_id[5] = {0};
    unsigned char frame_size_bytes[4];
    unsigned int frame_size;
    unsigned char *data;

    while (1)
    {
        // Read Frame ID
        if (fread(frame_id, 1, 4, mp3->org_mp3_fptr) != 4)
            break;

        frame_id[4] = '\0';

        // Check padding
        if (frame_id[0] == '\0')
            break;

        // Read Frame Size
        if (fread(frame_size_bytes, 1, 4,
                  mp3->org_mp3_fptr) != 4)
            break;

        // Convert Big Endian to host integer
        memcpy(&frame_size, frame_size_bytes, 4);
        toggle_endianess(mp3, (int *)&frame_size);

        // Skip 2-byte frame flags
        if (fseek(mp3->org_mp3_fptr, 2, SEEK_CUR) != 0)
            break;

        if (frame_size == 0)
            continue;

        // Allocate memory
        data = malloc(frame_size);

        if (data == NULL)
        {
            printf("Memory allocation failed\n");
            break;
        }

        // Read frame data
        if (fread(data, 1, frame_size,
                  mp3->org_mp3_fptr) != frame_size)
        {
            free(data);
            break;
        }

        // Display tag information
if (strcmp((char *)frame_id, "TIT2") == 0)
    print_tag_data("TITLE", data, frame_size);

else if (strcmp((char *)frame_id, "TPE1") == 0)
    print_tag_data("ARTIST", data, frame_size);

else if (strcmp((char *)frame_id, "TALB") == 0)
    print_tag_data("ALBUM", data, frame_size);

else if (strcmp((char *)frame_id, "TYER") == 0 ||
         strcmp((char *)frame_id, "TDRC") == 0)
    print_tag_data("YEAR", data, frame_size);

else if (strcmp((char *)frame_id, "TCON") == 0)
    print_tag_data("GENRE", data, frame_size);

else if (strcmp((char *)frame_id, "COMM") == 0)
{
    print_tag_data("COMMENT", data, frame_size);
}

free(data);
    }

    fclose(mp3->org_mp3_fptr);
    mp3->org_mp3_fptr = NULL;

    printf("\nMP3 file reading completed\n");
}


// Display tag data
void print_tag_data(char *tag, unsigned char *data, int size)
{
    if (size <= 1)
        return;

    printf("%s : ", tag);

    // Skip encoding byte
    for (int i = 1; i < size; i++)
    {
        if (data[i] == '\0')
            break;

        printf("%c", data[i]);
    }

    printf("\n");
}
void print_comment(unsigned char *data, int size)
{
    if (size <= 4)
        return;

    int encoding = data[0];
    int i = 4;

    // Skip description
    if (encoding == 0 || encoding == 3)
    {
        while (i < size && data[i] != '\0')
            i++;

        if (i < size)
            i++;
    }
    else
    {
        // UTF-16 description: skip two-byte null terminator
        while (i + 1 < size &&
               !(data[i] == 0 && data[i + 1] == 0))
        {
            i += 2;
        }

        if (i + 1 < size)
            i += 2;
    }

    if (i >= size)
        return;

    printf("COMMENT : ");

    for (; i < size; i++)
    {
        if (data[i] == '\0')
            break;

        printf("%c", data[i]);
    }

    printf("\n");
}