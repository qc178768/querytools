#ifndef __objalloc_h__
#define __objalloc_h__

//#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>

struct OA_BLOCK_HEAD;

struct OA_POOL_HEAD {
	OA_BLOCK_HEAD*	free;		// Pointer to free list
	OA_BLOCK_HEAD*	used;		// Pointer to allocated list
};

struct OA_BLOCK_HEAD {
	OA_POOL_HEAD*	head;		// Pointer to pool header
	OA_BLOCK_HEAD*	prev;		// Pointer to prev block
	OA_BLOCK_HEAD*	next;		// Pointer to next block
};


/*****************************************************************************
*
* Size of headers
*
*****************************************************************************/
const size_t OA_PHSIZE = sizeof(OA_POOL_HEAD);
const size_t OA_BHSIZE = sizeof(OA_BLOCK_HEAD);




/*****************************************************************************
*
* Internal used allocate functions.
* Use this function to transfer a memory block between Used list and
* Free list.
*
*****************************************************************************/
inline void objalloc_move(register OA_BLOCK_HEAD*& from, register OA_BLOCK_HEAD*& to, register OA_BLOCK_HEAD* block) {
	// Remove from source
	if (block->prev) {
		block->prev->next = block->next;
	} else {
		from = block->next;
	}
	if (block->next) {
		block->next->prev = block->prev;
	}

	// Add to target
	if (NULL!=(block->next = to)) {
		to->prev = block;
	}
	block->prev = NULL;
	to = block;
}



/*****************************************************************************
*
* Use this function to free memories allocated by objalloc classes.
* Called by overloaded operator delete of the object allocated.
*
*****************************************************************************/
inline void objalloc_free(register void* addr) {
	register OA_BLOCK_HEAD* block = ((OA_BLOCK_HEAD*)addr)-1;
	register OA_POOL_HEAD* head = block->head;
	objalloc_move(head->used, head->free, block);
}


/*****************************************************************************
*
* Internal used allocate functions.
* Allocate meomory from pool structure.
*
*****************************************************************************/
inline void* objalloc_doalloc(register OA_POOL_HEAD* head) {
	register OA_BLOCK_HEAD* block = head->free;
	objalloc_move(head->free, head->used, block);
	return (((char*)block)+OA_BHSIZE);
}



/*****************************************************************************
*
* Internal used allocate function.
* Allocate meomory from pool.
*
*****************************************************************************/
inline void* objalloc_alloc(void* buff) {
	register OA_POOL_HEAD* head = (OA_POOL_HEAD*)(buff);
	return head->free ? objalloc_doalloc(head) : NULL;
}






template <
	typename T,
	size_t BCount = 64,
	size_t BSize = (OA_BHSIZE+sizeof(T))
>
struct OAMemBlock {
	char			buff[OA_PHSIZE+BCount*BSize];
	OAMemBlock*		next;
};





template <
	typename T,
	size_t BCount = 64,
	size_t BSize = (OA_BHSIZE+sizeof(T)),
	typename MemBlk = OAMemBlock< T, BCount, BSize >
>
class objalloc {
	MemBlk*			memb;

protected:
	// format the new allocated pool
	void* format_alloc(char* buff) {
		// Pool buffer
		register char* pool = buff + OA_PHSIZE;
		register char* pend = pool + BCount*BSize;

		// Pool head
		register OA_POOL_HEAD* head = (OA_POOL_HEAD*)buff;
		head->free = (OA_BLOCK_HEAD*)pool;
		head->used = NULL;

		// Pool blocks
		register OA_BLOCK_HEAD* block = NULL;
		while (pool<pend) {
			block = (OA_BLOCK_HEAD*)pool;
			block->head = head;
			block->prev = (OA_BLOCK_HEAD*)(pool-BSize);
			block->next = (OA_BLOCK_HEAD*)(pool+BSize);
			pool += BSize;
		}

		// Header and tail
		block->next = NULL;
		head->free->prev = NULL;

		return objalloc_doalloc(head);
	}

public:
	objalloc():
		memb(NULL)
	{
		// Nothing to do else
	}

	virtual ~objalloc() {
		register MemBlk* next = NULL;
		while (memb) {
			next = memb->next;
			::free(memb);
			memb = next;
		}
	}

	void* alloc() {
		// Search the free block
		register void* addr = NULL;
		register MemBlk* pmemb = memb;
		while (pmemb) {
			if (NULL!=(addr = objalloc_alloc(pmemb->buff))) {
				return addr;
			}
			pmemb = pmemb->next;
		}

		// No free block found, then expand memory
		if ((pmemb = (MemBlk*)::malloc(sizeof(MemBlk)))==NULL) {
			return NULL;
		} else {
			// Add the mem block to list
			pmemb->next = memb;
			memb = pmemb;
			// Initialize the new pool
			return format_alloc(pmemb->buff);
		}
	}

	static void free(void* addr) {
		objalloc_free(addr);
	}
};




/*****************************************************************************
*
*	Bits mapped memory allocator
*
*	Notes:
*		Use this template to allocate 32 object a time from memory. It's a
*		little more fast than objalloc.
*
*	Structure of the bit allocator pool:
*
*	+-----------------------------------+
*	+            Bits Masks             +
*	+-----------------------------------+
*	+      Pointer to Bits Masks 1      +
*	+-----------------------------------+
*	+      Index (0)                    +
*	+-----------------------------------+
*	+      T 1                          +
*	+-----------------------------------+
*	+      Pointer to Bits Masks 2      +
*	+-----------------------------------+
*	+      Index (1)                    +
*	+-----------------------------------+
*	+      T 2                          +
*	+-----------------------------------+
*	+                                   +
*	+       ...........                 +
*	+                                   +
*	+-----------------------------------+
*	+      Pointer to Bits Masks 32     +
*	+-----------------------------------+
*	+      Index (31)                   +
*	+-----------------------------------+
*	+      T 32                         +
*	+-----------------------------------+
*
*****************************************************************************/


// Map bits
typedef unsigned long mm_map_t;


// Flags used by templates
const mm_map_t MM_BIT_TEST = 0x80000000;
const mm_map_t MM_BIT_INIT = 0xffffffff;

// Constants used by templates
const size_t MM_MAP_SIZE = sizeof(mm_map_t);
const size_t MM_PTR_SIZE = sizeof(mm_map_t*);
const size_t MM_BIT_COUNT = MM_MAP_SIZE*8;


// Global functions to call
#define mmalloc_alloc(sz, addr) ((sz) ? (addr) : (addr))

inline void mmalloc_free(void* addr) {
	*(*((mm_map_t**)(((char*)addr)-sizeof(size_t)-sizeof(mm_map_t*)))) ^=
		(MM_BIT_TEST>>(*((size_t*)(((char*)addr)-sizeof(size_t)))));
}



// Templates implementation
template
<
	typename T,
	size_t MSize = MM_MAP_SIZE,
	size_t BSize = MM_PTR_SIZE+sizeof(size_t)+sizeof(T),	/* [map-ptr][index][T] */
	size_t BCount = MM_BIT_COUNT
>
struct mm_pool_t
{
	char			buff[MSize+BCount*BSize];
	mm_pool_t*		next;
};



template
<
	typename T,
	size_t MSize = MM_MAP_SIZE,
	size_t BSize = MM_PTR_SIZE+sizeof(size_t)+sizeof(T),
	size_t BCount = MM_BIT_COUNT,
	size_t OBJOffset = MSize+MM_PTR_SIZE+sizeof(size_t)
>
class mmalloc
{
	typedef mm_pool_t< T, MSize, BSize, BCount >	Pool;

protected:
	Pool*	pool;

protected:
	// Get index of first valid bit.
	size_t getfree(register mm_map_t x) {
		if (x==0) {
			return 32;
		}
		register size_t n = 1;
		if ((x>>16)==0) {
			n += 16;
			x <<= 16;
		}
		if ((x>>24)==0) {
			n += 8;
			x <<= 8;
		}
		if ((x>>28)==0) {
			n += 4;
			x <<= 4;
		}
		if ((x>>30)==0) {
			n += 2;
			x <<= 2;
		}
		return (n -= (x>>31));
	}

	// Allocate the memory.
	void* do_alloc(register char* buff) {
		register size_t index = getfree(*((mm_map_t*)buff));
		*((mm_map_t*)buff) ^= (MM_BIT_TEST>>index);
		return (buff+OBJOffset+BSize*index);
	}

	void* format_alloc(register char* buff) {
		// Init map
		*((mm_map_t*)buff) = MM_BIT_INIT;
		// Init blocks
		register char* block = buff+MSize;	// Skip map bits
		for (register size_t i = 0; i<BCount; i++) {
			*((mm_map_t**)block) = (mm_map_t*)buff;		// Pointer to map bits
			*((size_t*)(block+sizeof(mm_map_t*))) = i;	// Index of this block
			block += BSize;
		}
		return do_alloc(buff);
	}

public:
	mmalloc(): pool(NULL) {
		// Nothing to do here
	}

	virtual ~mmalloc() {
		register Pool* next = NULL;
		while (pool) {
			next = pool->next;
			::free(pool);
			pool = next;
		}
	}

	void* alloc() {
		register Pool* p = pool;
		while (p) {
			if (*((mm_map_t*)(pool->buff))) {
				return do_alloc(pool->buff);
			}
			p = p->next;
		}

		if (NULL==(p = (Pool*)::malloc(sizeof(Pool)))) {
			return NULL;
		}

		// Add the pool to list.
		p->next = pool;
		pool = p;

		return format_alloc(pool->buff);
	}

	static void free(void* addr) {
		mmalloc_free(addr);
	}
};

#endif


