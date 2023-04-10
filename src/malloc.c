#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1) //enter header, get ptr to allocated data
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1) //enter allocated data, get ptr to block head

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

//env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block // size = 24 bytes
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *next;  /* Pointer to the next _block of allocated memory   */
   bool   free;          /* Is this _block free?                            */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3ByaW5nIDIwMjM            */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *roving_ptr = NULL; /* Used to keep track of where we left off in next fit*/
/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   //
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr; //sets last equal to address held in curr
      curr  = curr->next;
   }
#endif

// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
   struct _block *winner = NULL;
   int winning_remainder = INT_MAX;

   while (curr) 
   {
      if(!(curr->free && curr->size >= size))
      {
         *last = curr;
         curr = curr->next;
         continue;
      }

      if( (curr->size - size) < winning_remainder)
      {
         winner = curr;
         winning_remainder = curr->size - size;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = winner;

#endif

// \TODO Put your Worst Fit code in this #ifdef block
#if defined WORST && WORST == 0
   struct _block *winner = NULL;
   int winning_remainder = 0; //INT_MIN causes incorrect address

   while (curr) 
   {
      if(!(curr->free && curr->size >= size))
      {
         *last = curr;
         curr = curr->next;
         continue;
      }

      if( (curr->size - size) > winning_remainder)
      {
         winner = curr;
         winning_remainder = curr->size - size;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = winner;
   
#endif

// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   //if(roving_ptr!=NULL)
   //{
   curr = roving_ptr;
   //}

   //While block is not free and block size is less than requested size, iterate through list.
   while (curr && curr->next != roving_ptr && (curr->free != true && curr->size < size)) 
   {
      *last = curr; //holds the last block
      curr  = curr->next;

      //if we reached the end of the list, start at beginning
      if(curr == NULL)
      {
         curr = heapList;
      }
   }
   roving_ptr = curr;

#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0); //Calling sbrk() with an increment of 0 can be used to find the current location of the program break.
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */

   //if next>size > size then check:
   //if next->size - size > sizeof(_block)+4 then split
   //if next->size - size < sizeof(_block)+4 then dont split

   if(next != NULL && next->size > sizeof(struct _block)+4)
   {
      struct _block *temp = NULL;
      struct _block *original_next = next->next;
      size_t original_size = next->size;

      //subtract requested size from original block
      next->size = next->size - size;
      //set original block free to false
      next->free = false;
      //set original block next to BLOCK_DATA + requested size
      unsigned char* new_mem = ((unsigned char*)BLOCK_DATA(next) + size);
      next->next = (struct _block*)new_mem;

      //starting at new split block
      temp = next->next;
      //new free block size should be original size of block - (requested size + header size)
      temp->size = original_size - (size + sizeof(struct _block));
      //set new block next = to what the original block next was
      temp->next = original_next;
      //set new block free to true
      temp->free = true;

      num_splits++;
   }


   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   num_mallocs++;
   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   /* TODO: Coalesce free _blocks.  If the next block or previous block 
            are free then combine them with this block being freed.
   */
   struct _block *temp = heapList;

   while(temp)
   {
      if(temp->free == true && temp->next->free == true)
      {
         //combine sizes. current block size + next block size + size of block header
         temp->size = temp->size + temp->next->size + sizeof(struct _block);
         temp->next = temp->next->next;
      }
      temp = temp->next;
   }
   
   num_frees++;
}

void *calloc( size_t nmemb, size_t size )
{
   // \TODO Implement calloc
   malloc(0);
   return NULL;
}

void *realloc( void *ptr, size_t size )
{
   // \TODO Implement realloc
   return NULL;
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwMjM= -----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
