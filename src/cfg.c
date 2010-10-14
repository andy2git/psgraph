/*
 * $Rev: 776 $ 
 * $Date: 2010-09-01 10:40:43 -0700 (Wed, 01 Sep 2010) $ 
 * $Author: Andy $
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 * This file reads the 'cfg.conf' for configuration info.
 * It fetches the value by the keyword, and if the keyword
 * is not present in 'cfg.conf', it will HALT the program.
 */

#include "cfg.h"

/* -----------------------------------------------------
 * fetch value from 'cFile' based on the given key
 *
 * @param cFile - configure file
 * @param key   - configure key name
 *
 * -----------------------------------------------------*/ 
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
    fprintf(stderr, "Error: cannot find config value for key: [%s]\n", key);
    exit(EXIT_FAILURE);

    return -1;
}
