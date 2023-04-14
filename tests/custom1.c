#include <stdlib.h>

int main()
{
    char * ptr1 = ( char * ) malloc ( 100 );
    char * ptr2 = ( char * ) malloc ( 100 );
    free(ptr1);
    free(ptr2);
    char * ptr3 = ( char * ) malloc ( 300 );
    char * ptr4 = ( char * ) malloc ( 100 );
    char * ptr5 = ( char * ) malloc ( 100 );
    free(ptr3);
    free(ptr4);
    free(ptr5);

    return 0;
}