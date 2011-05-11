/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */


#include "lib.h"

/* int power function */
int power(int base, int n){
    int p;
    
    for(p = 1; n > 0; n--){
        assert(p < INT_MAX/base);
        p *= base;
    }

    return p;
}

/* compute 2^x */
int pow2(int x){
    return 1 << x;
}

/* return the integer floor */
int flr(int dividend, int divisor){
    return (dividend % divisor) ? (dividend/divisor+1) : (dividend/divisor);
}


