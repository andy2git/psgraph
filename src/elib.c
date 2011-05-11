/*
 * $Rev$ 
 * $Date$ 
 * $Author: andy.cj.wu@gmail.com $
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */


#include "elib.h"

static char *name = NULL;    /* program name */
static size_t space = 0;     /* keep track of allocated mem */

/* ----------------------------------------------*
 * duplicate a copy of *s. 
 * NOTE: new piece of memory is allocated
 *
 * @param s - pointer to string to be copied
 * ----------------------------------------------*/
char *estrdup(char *s){
    char *t = NULL;
    int need;

    need = strlen(s) + 1;
    t = malloc(need);
    if(t == NULL)
        eprintf("estrdup(\"%.20s\") failed:", s);

    space += need;

    strcpy(t, s);
    return t;
}

/* ----------------------------------------------*
 * print error msg on stderr
 *
 * @param msg - error msg to be printed
 * ----------------------------------------------*/

void error(char *msg){
    fprintf(stderr, "LOG - ERROR: %s\n", msg);
}


/* ----------------------------------------------*
 * print error msg on stderr
 *
 * @param fmt - 
 * ----------------------------------------------*/
void eprintf(char *fmt, ...){

    va_list args;
    fflush(stdout);

    if(getProgName() != NULL)
        fprintf(stderr, "%s: ", getProgName());

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if(fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':')
        fprintf(stderr, " %s", strerror(errno));
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}


/* ----------------------------------------------*
 * allocate n size memory
 * NOTE: equeliant to malloc and NULL check
 *
 * @param n - size of memory
 * ----------------------------------------------*/
void *emalloc(size_t n){
    void *p = NULL;
    
    p = malloc(n);
    if(p == NULL)
        eprintf("malloc of %u bytes failed:", n);

    space += n;
    return p;
}


/* ----------------------------------------------*
 * allocate nmember size sized elems
 * NOTE: equeliant to calloc and NULL check
 *
 * @param nmemb - #elems
 * @param size - elem size
 * ----------------------------------------------*/
void *ecalloc(size_t nmemb, size_t size){
    void *p = NULL;

    p = calloc(nmemb, size);

    if(p == NULL)
        eprintf("calloc of (%u*%u) bytes failed:", nmemb, size);

    space += (nmemb*size);
    return p;
}

/* ---------------------------------------------*
 * open a file, and returns a file handler
 * 
 * @param fileName - file name
 * @param mode - file open mode
 * ---------------------------------------------*/

void *efopen(char *fileName, char *mode){
    FILE *fp = NULL;

    fp = fopen(fileName, mode);
    if(fp == NULL)
        eprintf("open file \"%.30s\" in mode \"%.10s\" failed:", fileName, mode);

    return fp;
}


void setProgName(char *str){
    name = estrdup(str);
}


char *getProgName(void){
    return name;
}

void printSpace(void){
    printf("space=%d\n", (int)space);
}

