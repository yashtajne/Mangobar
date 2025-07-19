#include "command.h"


char* Mf_RunCommand(const char* cmd)
{
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("Failed to run command");
        return NULL;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), fp)) {
        pclose(fp);
        return strdup("NULL");
    }

    pclose(fp);
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    return strdup(buffer);
}
