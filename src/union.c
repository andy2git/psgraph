#include "union.h"

struct ufind *initUnion(int size) {
	int i;
	struct ufind *ufSet;

	ufSet = emalloc(size*(sizeof* ufSet));

	/* init all set elems */
	for (i=0; i<size; i++) {
		ufSet[i].parent = i;
		ufSet[i].rank = 0;
	}

	return ufSet;
}

void freeUnion(struct ufind *uf){
    free(uf);
}

int find(struct ufind *ufSet, int elemIndex) {
	int myParent;
	myParent = ufSet[elemIndex].parent;

	if (ufSet[myParent].parent != myParent) {
        /* path compression part */
		myParent = ufSet[elemIndex].parent = find(ufSet, myParent);

        /* elemIndex's parent is updated, so the rank should decrease */
		ufSet[myParent].rank--;
		return myParent;
	} else {
		return myParent;
	}
}


void unionElems(struct ufind *ufSet, int elem1, int elem2) {
    mergeElems(ufSet, find(ufSet, elem1), find(ufSet, elem2));
}

/* can never been called outside of this file */
void mergeElems(struct ufind *ufSet, int elem1, int elem2) {
	if (elem1!=elem2) {
		if (ufSet[elem1].rank > ufSet[elem2].rank) {
			ufSet[elem2].parent = elem1;
		} else {
			ufSet[elem1].parent = elem2;
			if (ufSet[elem1].rank==ufSet[elem2].rank) {
				ufSet[elem2].rank++;
			}
		}
	}
}

void disp(struct ufind *ufSet, int size) {
	int i;
	for (i=0; i<size; i++) {
		if (i!=ufSet[i].parent)
			printf("(%d,%d) ", i, ufSet[i].parent);
		fflush(stdout);
	}
	printf("\n");
}
