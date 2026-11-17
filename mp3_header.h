#ifndef MP3_HEADER_H
#define MP3_HEADER_H

#include <stdio.h>
#include <string.h>

#define VIEW 	1
#define EDIT 	2


struct MP3
{
    char *mp3_filename;
    FILE *org_mp3_fptr;
    FILE *dup_mp3_fptr;
    char *edit_option;
    char *new_value;
    char *tag_option;
};

int validate_cla(int argc, char *argv[], struct MP3 *mp3);
void view(struct MP3 *mp3);
void edit(struct MP3 *mp3);
void toggle_endianess(struct MP3 *mp3, int *size);
void print_tag_data(char *tag, unsigned char *data, int size);
void print_comment(unsigned char *data, int size);
void tag_to_name(char *);

#endif
