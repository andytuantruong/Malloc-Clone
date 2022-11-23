// Professor R. Jesse Chaney
// rchaney@pdx.edu

// Student: Andy Truong 
// truong7@pdx.edu	
// April 11th, 2022
// CS333: Lab 1 Heap Allocation

#include "vikalloc.h"

#define BLOCK_SIZE (sizeof(mem_block_t))

// Get to the user data within the give block.
#define BLOCK_DATA(__curr) (((void *) __curr) + (BLOCK_SIZE))

// Just making it easier to print some pointers.
#define PTR "0x%07lx"
#define PTR_T PTR "\t"

//coalesce helper functions
static void coalesce_up(mem_block_t * curr);

// I use this as the head of the heap map. This is also used in the function
// vikalloc_dump2() to display the contents of the heap map.
static mem_block_t *block_list_head = NULL;

// I use this to use this to point to the last block in the heap map. However,
// I dont' really make use of this for any really good reason.
static mem_block_t *block_list_tail = NULL;

// I use these as the low water and high water pointer value for the heap
// that bevalloc is using. These are used in vikalloc_dump2().
static void *lower_mem_bound = NULL;
static void *upper_mem_bound = NULL;

// Sometimes we want a chatty allocator, like when debugging.
static uint8_t isVerbose = FALSE;

// This is where all the diagnostic output is sent (like all the stuff from
// vikalloc_dump2()). This is established with some gcc magic. You can alter
// it from the command line into another file.
static FILE *vikalloc_log_stream = NULL;

// The afore memtioned gcc magic.
static void init_streams(void) __attribute__((constructor));

// This is the variable that is used to determine how much space is requested
// from each call to sbrk(). Each call to sbrk() must be a multiple of this
// value.
static size_t min_sbrk_size = DEFAULT_SBRK_SIZE;

// The gcc magic implementation.
static void 
init_streams(void)
{
    vikalloc_log_stream = stderr;
}

// Allos us to chnage the multiple used for calls to sbrk().
size_t
vikalloc_set_min(size_t size)
{
    if (0 == size) {
        // just return the current value
        return min_sbrk_size;
    }
    if (size < (BLOCK_SIZE + BLOCK_SIZE)) {
        // In the event that is is set to something silly.
        size = MAX(BLOCK_SIZE + BLOCK_SIZE, SILLY_SBRK_SIZE);
    }
    min_sbrk_size = size;
    return min_sbrk_size;
}

// To be chatty or not to be chatty...
void 
vikalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose) {
        fprintf(vikalloc_log_stream, "Verbose enabled\n");
    }
}

// Redirect the diagnostic output into another file.
void 
vikalloc_set_log(FILE *stream)
{
    vikalloc_log_stream = stream;
}

// This is where the fun begins.
// You need to be able to split the first existing block that can support the
// the requested size.  
void * 
vikalloc(size_t size)
{
	mem_block_t *curr = NULL;
	size_t multiple = 0;
	size_t sbrk_size = 0;

	if(isVerbose) 
	{
		fprintf(vikalloc_log_stream, ">> %d: %s entry: size = %lu\n"
			, __LINE__, __FUNCTION__, size);
	}

   	if(0 == size)
	{
       		return NULL;
 	}

	//set errno if size is less than 0
	if(size < 0)
	{
		errno = ENOMEM;
		return NULL;
	}

	curr = block_list_head;

	//find the multiple of min_sbrk_size to allocate enough space
	multiple = (((size + BLOCK_SIZE) / min_sbrk_size) + 1);
	sbrk_size = min_sbrk_size * multiple;

	if(curr == NULL) //add the first node 
	{
		//allocate space for current
		curr = (mem_block_t *) sbrk(sbrk_size);
	
		//set low water mark
		lower_mem_bound = curr;

		//set head and tail to first node
		block_list_head = curr;
		block_list_tail = curr;


		//set members
		curr->size = size;
		curr->capacity = sbrk_size - BLOCK_SIZE;
		curr->free = FALSE;
		curr->prev = NULL;
		curr->next = NULL;

		//set high water mark
		upper_mem_bound = lower_mem_bound + curr->capacity + BLOCK_SIZE;

		return BLOCK_DATA(curr);
	}
	else
	{	
		//check blocks in list until end or conditions met	
		while(curr != NULL)
		{
			//check for reuse
			if(curr->free == TRUE && curr->capacity >= size)
			{
				//set mem vars for reuse, first fit
				curr->free = FALSE;
				curr->size = size;
				return BLOCK_DATA(curr);
			}
			
			//check for split
			if((curr->free == FALSE) && (curr->capacity - curr->size) >= size + BLOCK_SIZE)
			{
				//set address of the new block
				mem_block_t * new = (void *)curr + curr->size + BLOCK_SIZE;

				//set new member	
				new->free = FALSE;
				new->capacity = (curr->capacity - curr->size - BLOCK_SIZE);
				new->size = size;

				//currents capacity is now its size when split
				curr->capacity = curr->size;

				//link up new ptrs to currs 
				new->prev = curr;
				new->next = curr->next;
			
				//if split at the end, set tail to last node
				if(curr->next == NULL)
				{
					block_list_tail = new;
				}
				else
				{
					//attach to front prev ptr to new block
					curr->next->prev = new;
				}

				curr->next = new;
				curr = new;

				return BLOCK_DATA(curr);
			}
			//traverse the list 
			curr = curr->next;
		}

		//alocate memory for curr 
		curr = (mem_block_t *) sbrk(sbrk_size);

		//set members
		curr->size = size;
		curr->capacity = sbrk_size - BLOCK_SIZE;
		curr->free = FALSE;

		//add to the end oft he list using tail ptr
		block_list_tail->next = curr; //use tail to connect up "last node"
		curr->prev = block_list_tail;
		curr->next = NULL;
		block_list_tail = curr;

		//update upper water mark 
		upper_mem_bound += curr->capacity + BLOCK_SIZE;
	}
    //address of current ptr is returned 
    return BLOCK_DATA(curr);
}

// Free the block that contains the passed pointer. You need to coalesce adjacent
// free blocks.
void 
vikfree(void *ptr)
{
    	mem_block_t * curr = NULL;

    	if(ptr == NULL)
	{
		return;
	}

	//return if head is empty, nothing to free
	if(block_list_head == NULL)
	{
		return;
	}
	
	//get to the address, don't have to traverse to find ptr
	curr = ptr - BLOCK_SIZE; 

	//set member vars accordingly to be free
	curr->free = TRUE; 
	curr->size = 0;

	//coalesce up
	if(curr->next != NULL && curr->next->free == TRUE) //check if NULL first before checking its free
	{
		coalesce_up(curr);

	}

	//check block below and perform a recursive call to coalsce up 
        if(curr->prev != NULL && curr->prev->free == TRUE)	
	{
		coalesce_up(curr->prev);	
		//can also use a recursive call
		/*
		//set as a pseudo false to coalesce back up
		curr->prev->free = FALSE;
		vikfree(BLOCK_DATA(curr->prev));
		*/
	}


	if(curr->free == TRUE)
	{
		if(isVerbose)
		{
    			fprintf(vikalloc_log_stream, "Block is already free: ptr = " PTR "\n"
	    			, (long) (ptr - lower_mem_bound));
		}
	return;
	}
    return;
}

static void coalesce_up(mem_block_t * curr)
{
	if(curr->next->next == NULL)
	{
		block_list_tail = curr;
		
	}
	else
	{
		curr->next->next->prev = curr;
	}

	//adjust capacity and link up
	curr->capacity += curr->next->capacity + BLOCK_SIZE;
	curr->next = curr->next->next;
	return;
}

// Release the kraken, or at least all the vikalloc heap. This should leave
// everything as though it had never been allocated at all. A call to vikalloc
// that follows a call the vikalloc_reset starts compeletely from nothing.
void 
vikalloc_reset(void)
{
	//check if the lower water mark exists
    	if(lower_mem_bound)
	{
		brk(lower_mem_bound);
		lower_mem_bound = NULL; //low water mark
		upper_mem_bound = NULL; //high watermark
		block_list_head = NULL; 
		block_list_tail = NULL;
	}

}

// Is like the regular calloc().
void *
vikcalloc(size_t nmemb, size_t size)
{
	//no need to error check, handled in vikalloc()
	void *ptr = NULL;
	ptr = vikalloc(nmemb * size);
	memset(ptr, 0, nmemb * size); //initialize memory to 0
	return ptr;
}

// Like realloc, but simpler.
// If the requested new size does not fit into the current block, create 
// a new block and copy contents. If the requested new size does fit, just 
// adjust the size data member.
void *
vikrealloc(void *ptr, size_t size)
{
	mem_block_t * curr = NULL;
	if(size == 0)
	{
		return NULL;
	}

	if(ptr == NULL)
	{
		ptr = vikalloc(size);
	}
	else
	{
		//get to the ptr 
		curr = ptr - BLOCK_SIZE;

		if(size <= curr->capacity) //adjust size if its too small
		{
			curr->size = size;
		}
		else 
		{
			void * new = vikalloc(size);
			//copy over ptr with new amt of mem
			memcpy(new, ptr, curr->size); 

			//free the ptr
			vikfree(ptr);
			ptr = new;
		}
	}
	return ptr;
}

// Like the old strdup, but uses vikalloc().
void *
vikstrdup(const char *s)
{
    	void *ptr = NULL;
	size_t len = 0;
	len = strlen(s);
	++len; //account for \0
	ptr = vikalloc(len);
	memcpy(ptr, s, len);
	return ptr;
}

// It is totaly gross to include C code like this. But, it pulls out a big
// chunk of code that you don't need to change. It needs access to the static
// variables defined in this module, so it either needs to be included, as
// here, as gross as it may be, or all the code directly in the module.
#include "vikalloc_dump.c"
