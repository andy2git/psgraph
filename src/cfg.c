#include "cfg.h"

/* ---------------------------------------------------*
 * Read cFile and return the value corresponding key
 *
 * @param cFile - configure file
 * @param key   - configure key name
 * ---------------------------------------------------*/
int getCfgVal(char *cFile, char *key){
    FILE *fp = NULL;
    char line[CFG_MAX_LINE_LEN];
    int val;
    char *p = NULL;
    
    fp = efopen(cFile, "r");

    while(fgets(line, CFG_MAX_LINE_LEN, fp)){
        /* comment line starts with '#' */
        if(strchr(line, COMMENT)) continue;

        /* empty line */
        if(line[0] == '\n') continue;

        /* config line */
        if(strstr(line, key)) {
            p = line;
            while(isspace(*p++));

            /* %*s used to skip the first item */
            sscanf(p, "%*s %d\n", &val);
            fclose(fp);
            return val;
        }  
    }

    fclose(fp);
    printf("Error: cannot find config value for key: [%s]\n", key);
    exit(EXIT_FAILURE);

    return -1;
}
