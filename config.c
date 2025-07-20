#include "config.h"
#include <stddef.h>


extern M_TextBlock* left_blocks[];
extern M_TextBlock* center_blocks[];
extern M_TextBlock* right_blocks[];

extern size_t left_blocks_length;
extern size_t mid_blocks_length;
extern size_t right_blocks_length;


extern Display* display;
extern int workspace;


void _process() {
    workspace = get_current_desktop(display);
}


int Mf_inihandler(void* user, const char* section, const char* key, const char* value) {
    printf("Section: %s, Key: %s, Value: %s\n", section, key, value);

    char* slash = strchr(section, '/');
    if (slash) {
        size_t len1 = slash - section;
        char sname[len1 + 1];
        strncpy(sname, section, len1);
        sname[len1] = '\0';
        char *sbname = slash + 1;

        M_TextBlock* block;
        if (strcmp(sname, "LEFT") == 0) {
            block = Mf_SearchBlock(left_blocks, left_blocks_length, sbname);
            if (block == NULL) {
                block = malloc(sizeof(M_TextBlock));
                if (block == NULL) {
                    printf("Error allocating memory for block named %s\n", sbname);
                    return 0;
                }
                memset(block, 0, sizeof(M_TextBlock));
                block->name = strdup(sbname);
                left_blocks[left_blocks_length] = block;
                left_blocks_length++;
            }
        } else if (strcmp(sname, "CENTER") == 0) {
            block = Mf_SearchBlock(center_blocks, mid_blocks_length, sbname);
            if (block == NULL) {
                block = malloc(sizeof(M_TextBlock));
                if (block == NULL) {
                    printf("Error allocating memory for block named %s\n", sbname);
                    return 0;
                }
                memset(block, 0, sizeof(M_TextBlock));
                block->name = strdup(sbname);
                center_blocks[mid_blocks_length] = block;
                mid_blocks_length++;
            }
        } else if (strcmp(sname, "RIGHT") == 0) {
            block = Mf_SearchBlock(right_blocks, right_blocks_length, sbname);
            if (block == NULL) {
                block = malloc(sizeof(M_TextBlock));
                if (block == NULL) {
                    printf("Error allocating memory for block named %s\n", sbname);
                    return 0;
                }
                memset(block, 0, sizeof(M_TextBlock));
                block->name = strdup(sbname);
                right_blocks[right_blocks_length] = block;
                right_blocks_length++;
            }
        }

        if (strcmp(key, "text") == 0) {
            block->text = strdup(value);
        } else if (strcmp(key, "font") == 0) {
            block->font = strdup(value);
        } else if (strcmp(key, "text_color") == 0) {
            block->text_color = strdup(value);
        } else if (strcmp(key, "background_color") == 0) {
            block->background_color = strdup(value);
        }
    }

    return 1;
}


