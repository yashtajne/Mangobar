

#define MAX_BLOCKS_IN_SECTION 8

#define BAR_HEIGHT 24
#define BAR_OFFSET 0
#define V_OFFSET 18
#define H_PADDING 5

#define BAR_COLOR "#1f1f28"
#define BAR_FONT "Cascadia CodeNF:extrabold"


#define _PROCESS_INTERVAL 1.0





#include <ini.h>
#include <stdio.h>
#include <string.h>
#include "block.h"
#include "x11utils.h"

void _process();

int Mf_inihandler(void* user, const char* section,
                  const char* key, const char* value);
