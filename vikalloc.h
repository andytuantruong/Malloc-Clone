// R. Jesse Chaney
// rchaney@pdx.edu

#ifndef __VIKALLOC_H
# define __VIKALLOC_H

# include <stdint.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <stddef.h>
# include <values.h>
# include <stdlib.h>
# include <stdio.h>

// enable the define below to turn off assert.
//# define NDEBUG
# include <assert.h>

#ifndef MAX
# define MAX(_a,_b) ((_a) > (_b) ? (_a) : (_b))
#endif // MAX

#ifndef MIN
# define MIN(_a,_b) ((_a) < (_b) ? (_a) : (_b))
#endif // MIN

#ifndef TRUE
# define TRUE 1
#endif // TRUE
#ifndef FALSE
# define FALSE 0
#endif // FALSE

#ifndef DEFAULT_SBRK_SIZE
# define DEFAULT_SBRK_SIZE 1024
#endif // DEFAULT_SBRK_SIZE

#ifndef SILLY_SBRK_SIZE
# define SILLY_SBRK_SIZE 128
#endif // SILLY_SBRK_SIZE

typedef struct mem_block_s {
    uint8_t free; //in-use or free

    size_t capacity; //tracks the total bytes of data that can be held
    size_t size; //keeps track of bytes of data allocated to user

    struct mem_block_s *prev;
    struct mem_block_s *next;
} mem_block_t;


// The basic memory allocator.
// If you pass NULL or 0, then NULL is returned.
// If, for some reason, the system cannot allocate the requested
// memory, set errno and return NULL.
// You must use sbrk() or brk() in requesting more memory for your
// vikalloc() routine to manage.
// Split blocks where there is enough excess capcacity to support the
//   requested size.
void *vikalloc(size_t size);

// A pointer returned from a prevous call to vikalloc() must
// be passed.
// If a pointer is passed to a block than is already free, 
// simply return. If you are in verbose mode, print a snarky message
// before returning.
// Blocks must be coalesced, where possible, as they are free'ed.
void vikfree(void *ptr);

// This code is provided in a seperate .c file. It prints the entire
// contents of the heap map.
void vikalloc_dump2(long);

// Completely reset your heap back to zero bytes allocated.
// You are going to like being able to do this.
// Implementation could be done in as few as 1 line, though
// you will probably use more to reset the stats you keep
// about heap.
// After you've called this function, everything you had in
// the heap is just __GONE__!!!
// You should be able to call vikalloc() after calling vikalloc_reset()
// to restart building the heap again.
void vikalloc_reset(void);

// Set the verbosity of your vikalloc() code (and related functions).
// This should modify a variable that is static to your C module.
void vikalloc_set_verbose(uint8_t);

// Set the stream into which diagnostic information will be sent. This function
// is provided.
void vikalloc_set_log(FILE *);

// This is like the regular calloc() call. See the man page for details.
void *vikcalloc(size_t nmemb, size_t size);

// This is like the regular realloc() call. This acts like the regular
// realloc(), but is a bit simpler. If the resuested new size does not fit into
// the current block, create a new block and copy contents. If the requested
// new size does fit, just adjust the size data member.
void *vikrealloc(void *ptr, size_t size);

// This should act like the regular strdup() function, but is calls
// vikalloc(), not malloc().
void *vikstrdup(const char *s);

// Provided code. This changes the multiple used for allocation of memory
// using sbrk(). When making calls to sbrk(), you must use a multiple of the 
// value set in this function.
size_t vikalloc_set_min(size_t);

#endif // __VIKALLOC_H
