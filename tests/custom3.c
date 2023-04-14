#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char * ptr_arr[100];

    for(int i = 0; i < 100; i++)
    {
        ptr_arr[i] = ( char * ) malloc ( i*22 ); 
    }
    
    char * ptr1 = ( char * ) malloc ( 1024 );
    char * ptr2 = ( char * ) malloc ( 100 );
    free(ptr2);
    char * ptr3 = ( char * ) malloc ( 600 );
    char * ptr4 = ( char * ) malloc ( 200 );

    for(int i = 0; i < 100; i++)
    {
        free( ptr_arr[i] );
    }

    char * ptr5 = ( char * ) malloc ( 200 );
    free(ptr5);
    char * ptr6 = ( char * ) malloc ( 200 );
    
    free(ptr1);
    free(ptr6);
    free(ptr3);
    free(ptr4);

    return 0;
}