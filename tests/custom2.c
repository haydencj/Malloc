#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char * ptr_arr[500];

    for(int i = 0; i < 500; i++)
    {
        ptr_arr[i] = ( char * ) malloc ( i*10 ); 
    }
    
    for(int i = 0; i < 500; i++)
    {
        free( ptr_arr[i] );
    }
    
    char * ptr2 = ( char * ) malloc ( 100 );
    free(ptr2);
    char * ptr3 = ( char * ) malloc ( 100 );
    free(ptr3);
    char * ptr4 = ( char * ) malloc ( 100 );
    free(ptr4);
    char * ptr5 = ( char * ) malloc ( 1000 );
    char * ptr6 = ( char * ) malloc ( 500 );
    char * ptr7 = ( char * ) malloc ( 30 );

    free(ptr5);
    free(ptr6);
    free(ptr7);

    return 0;
}