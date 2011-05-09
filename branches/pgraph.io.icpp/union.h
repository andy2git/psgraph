#ifndef UFIND_H_
#define UFIND_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "type.h"
#include "elib.h"

typedef struct ufind {
  int parent;
  int rank;
}UF;

struct link {
  int id;
  struct link *next;
};

struct clusters {
  int count;
  char singleton;      /* 1 if singleton 0 if no */
  struct link *head;
};

struct ufind *initUnion(int size);
void freeUnion(struct ufind *uf);
int find(struct ufind *ufSet,int inputIndex);
void unionElems(struct ufind *ufSet,int elem1,int elem2);
void mergeElems(struct ufind *ufSet,int elem1,int elem2);

void disp(struct ufind *ufSet,int size);

#endif /* end of union.h */
