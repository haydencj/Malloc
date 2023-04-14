#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char * ptr1 = ( char * ) malloc ( 10000 );
    char * ptr2 = ( char * ) malloc ( 50 );

    free(ptr1);

    ptr1 = ( char * ) malloc ( 500 );
    char * ptr3 = ( char * ) malloc ( 500 );

    free(ptr1);
    free(ptr2);
    free(ptr3);

    return 0;
}