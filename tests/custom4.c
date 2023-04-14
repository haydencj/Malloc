#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char * ptr1 = ( char * ) malloc ( 1000 );
    free(ptr1); 
    char * ptr2 = ( char * ) malloc ( 1000 );
    free(ptr2); 
    char * ptr3 = ( char * ) malloc ( 1000 );
    free(ptr3);
    char * ptr4 = ( char * ) malloc ( 1000 );
    free(ptr4);
    
    return 0;
}