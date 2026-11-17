
#include "mp3_header.h"
#include <stdlib.h>

// Read ID3 synchsafe tag size
unsigned int get_tag_size(char header[])
{
    return (unsigned int)(unsigned char)header[6] * 2097152U +
           (unsigned int)(unsigned char)header[7] * 16384U +
           (unsigned int)(unsigned char)header[8] * 128U +
           (unsigned int)(unsigned char)header[9];
}

// Update ID3 synchsafe tag size
void set_tag_size(char header[], unsigned int size)
{
    header[6] = (size / 2097152U) % 128;
    header[7] = (size / 16384U) % 128;
    header[8] = (size / 128U) % 128;
    header[9] = size % 128;
}

// Compare command option with frame ID
int compare_tag(char *option, char *frame_id)
{
    if (strcmp(option, "-t") == 0 &&
        strcmp(frame_id, "TIT2") == 0)
        return 1;

    if (strcmp(option, "-a") == 0 &&
        strcmp(frame_id, "TPE1") == 0)
        return 1;

    if (strcmp(option, "-A") == 0 &&
        strcmp(frame_id, "TALB") == 0)
        return 1;

    if (strcmp(option, "-y") == 0 &&
        strcmp(frame_id, "TYER") == 0)
        return 1;

    if (strcmp(option, "-m") == 0 &&
        strcmp(frame_id, "TCON") == 0)
        return 1;

    if (strcmp(option, "-c") == 0 &&
        strcmp(frame_id, "COMM") == 0)
        return 1;

    return 0;
}

void edit(struct MP3 *mp3)
{
    char header[10];
    char frame_id[5];
    char frame_size_bytes[4];
    char frame_flags[2];
    char buffer[1024];

    int frame_size;
    int new_size;
    int edited = 0;

    unsigned int tag_size;
    unsigned int remaining;

    size_t bytes_read;

    // Open original MP3
    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if (mp3->org_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to open original MP3\n");
        return;
    }

    printf("Original MP3 file opened successfully\n");

    // Open temporary MP3
    mp3->dup_mp3_fptr = fopen("temp.mp3", "wb+");

    if (mp3->dup_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to create temp.mp3\n");
        fclose(mp3->org_mp3_fptr);
        return;
    }

    printf("Temporary MP3 file opened successfully\n");

    printf("Selected option: %s\n", mp3->tag_option);
    printf("New value: %s\n", mp3->new_value);

    // Read 10-byte ID3 header
    if (fread(header, 1, 10, mp3->org_mp3_fptr) != 10)
    {
        printf("Unable to read ID3 header\n");
        goto cleanup;
    }

    // Validate ID3v2.3
    if (memcmp(header, "ID3", 3) != 0 || header[3] != 3)
    {
        printf("This code requires ID3v2.3\n");
        goto cleanup;
    }

    tag_size = get_tag_size(header);
    remaining = tag_size;

    printf("ID3 tag size: %u\n", tag_size);

    // Copy header
    if (fwrite(header, 1, 10, mp3->dup_mp3_fptr) != 10)
        goto cleanup;

    // Skip extended header if present
    if ((unsigned char)header[5] & 0x40)
    {
        unsigned char ext_size_bytes[4];

        if (fread(ext_size_bytes, 1, 4,
                  mp3->org_mp3_fptr) != 4)
            goto cleanup;

        int ext_size;
        memcpy(&ext_size, ext_size_bytes, 4);
        toggle_endianess(mp3, &ext_size);

        if (ext_size < 0 || (unsigned int)ext_size + 4 > remaining)
        {
            printf("Invalid extended header size\n");
            goto cleanup;
        }

        unsigned int total_ext_size = ext_size + 4;

        if (fwrite(ext_size_bytes, 1, 4,
                   mp3->dup_mp3_fptr) != 4)
            goto cleanup;

        while (total_ext_size > 4)
        {
            bytes_read = total_ext_size - 4;
            if (bytes_read > sizeof(buffer))
                bytes_read = sizeof(buffer);

            if (fread(buffer, 1, bytes_read,
                      mp3->org_mp3_fptr) != bytes_read)
                goto cleanup;

            if (fwrite(buffer, 1, bytes_read,
                       mp3->dup_mp3_fptr) != bytes_read)
                goto cleanup;

            total_ext_size -= bytes_read;
        }

        remaining -= ext_size + 4;

        printf("Extended header skipped successfully\n");
    }

    // Read frames
    while (remaining >= 10)
    {
        // Read frame ID
        if (fread(frame_id, 1, 4,
                  mp3->org_mp3_fptr) != 4)
            goto cleanup;

        frame_id[4] = '\0';

        printf("\nFrame ID found: [%s]\n", frame_id);

        // Padding
        if (frame_id[0] == '\0')
        {
            if (fwrite(frame_id, 1, 4,
                       mp3->dup_mp3_fptr) != 4)
                goto cleanup;

            remaining -= 4;
            break;
        }

        // Read frame size
        if (fread(frame_size_bytes, 1, 4,
                  mp3->org_mp3_fptr) != 4)
            goto cleanup;

        memcpy(&frame_size, frame_size_bytes, 4);
        toggle_endianess(mp3, &frame_size);

        printf("Frame size: %d\n", frame_size);

        // Read flags
        if (fread(frame_flags, 1, 2,
                  mp3->org_mp3_fptr) != 2)
            goto cleanup;

        if (frame_size < 0 ||
            (unsigned int)frame_size > remaining - 10)
        {
            printf("Invalid frame size\n");
            goto cleanup;
        }

        remaining -= 10 + frame_size;

        // Allocate frame data
        char *data = malloc(frame_size > 0 ? frame_size : 1);

        if (data == NULL)
        {
            printf("Memory allocation failed\n");
            goto cleanup;
        }

        // Read frame data
        if (fread(data, 1, frame_size,
                  mp3->org_mp3_fptr) != (size_t)frame_size)
        {
            free(data);
            goto cleanup;
        }

        // Compare tag
        if (!edited && compare_tag(mp3->tag_option, frame_id))
        {
            printf("Matching tag found: %s\n", frame_id);

            new_size = strlen(mp3->new_value) + 1;

            // Resize allocated memory
            char *temp = realloc(data, new_size);

            if (temp == NULL)
            {
                free(data);
                goto cleanup;
            }

            data = temp;

            // Text encoding
            data[0] = 0;

            // Copy new text
            strcpy(data + 1, mp3->new_value);

            // Write frame ID
            if (fwrite(frame_id, 1, 4,
                       mp3->dup_mp3_fptr) != 4)
            {
                free(data);
                goto cleanup;
            }

            // Write new frame size
            int temp_size = new_size;
            toggle_endianess(mp3, &temp_size);

            if (fwrite(&temp_size, 1, 4,
                       mp3->dup_mp3_fptr) != 4)
            {
                free(data);
                goto cleanup;
            }

            // Write flags
            if (fwrite(frame_flags, 1, 2,
                       mp3->dup_mp3_fptr) != 2)
            {
                free(data);
                goto cleanup;
            }

            // Write new data
            if (fwrite(data, 1, new_size,
                       mp3->dup_mp3_fptr) != (size_t)new_size)
            {
                free(data);
                goto cleanup;
            }

            tag_size = tag_size - frame_size + new_size;
            edited = 1;
        }
        else
        {
            // Copy unchanged frame
            if (fwrite(frame_id, 1, 4,
                       mp3->dup_mp3_fptr) != 4 ||
                fwrite(frame_size_bytes, 1, 4,
                       mp3->dup_mp3_fptr) != 4 ||
                fwrite(frame_flags, 1, 2,
                       mp3->dup_mp3_fptr) != 2 ||
                fwrite(data, 1, frame_size,
                       mp3->dup_mp3_fptr) != (size_t)frame_size)
            {
                free(data);
                goto cleanup;
            }
        }

        free(data);
    }

    // Copy remaining ID3 padding
    while (remaining > 0)
    {
        bytes_read = remaining > sizeof(buffer)
                   ? sizeof(buffer) : remaining;

        if (fread(buffer, 1, bytes_read,
                  mp3->org_mp3_fptr) != bytes_read)
            goto cleanup;

        if (fwrite(buffer, 1, bytes_read,
                   mp3->dup_mp3_fptr) != bytes_read)
            goto cleanup;

        remaining -= bytes_read;
    }

    if (!edited)
    {
        printf("\nSelected tag was not found\n");
        goto cleanup;
    }

    // Update header size
    set_tag_size(header, tag_size);

    if (fseek(mp3->dup_mp3_fptr, 0, SEEK_SET) != 0)
        goto cleanup;

    if (fwrite(header, 1, 10,
               mp3->dup_mp3_fptr) != 10)
        goto cleanup;

    // Move to end
    if (fseek(mp3->dup_mp3_fptr, 0, SEEK_END) != 0)
        goto cleanup;

    // Copy remaining audio data
    while ((bytes_read = fread(buffer, 1,
                               sizeof(buffer),
                               mp3->org_mp3_fptr)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read,
                   mp3->dup_mp3_fptr) != bytes_read)
            goto cleanup;
    }

    if (ferror(mp3->org_mp3_fptr))
        goto cleanup;

    printf("\nMP3 editing completed successfully\n");

    fclose(mp3->org_mp3_fptr);
    fclose(mp3->dup_mp3_fptr);

    mp3->org_mp3_fptr = NULL;
    mp3->dup_mp3_fptr = NULL;

    if (rename("temp.mp3", mp3->mp3_filename) != 0)
    {
        perror("Unable to replace original file");
        return;
    }

    printf("Original MP3 file updated successfully\n");
    return;

cleanup:

    fclose(mp3->org_mp3_fptr);
    fclose(mp3->dup_mp3_fptr);

    mp3->org_mp3_fptr = NULL;
    mp3->dup_mp3_fptr = NULL;

    printf("MP3 editing failed\n");
}
