#include <liballoc.h>
#include <_null.h>
#include <stdint.h>
#include <string.h>

#define VERSION 	"1.1"
#define ALIGNMENT	16ul//4ul				///< This is the byte alignment that memory must be allocated on. IMPORTANT for GTK and other stuff.

#define ALIGN_TYPE		char ///unsigned char[16] /// unsigned short
#define ALIGN_INFO		sizeof(ALIGN_TYPE)*16	///< Alignment information is stored right before the pointer. This is the number of bytes of information stored there.

/** This macro will conveniently align our pointer upwards */
#define ALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			uintptr_t diff;												\
			ptr = (void*)((uintptr_t)ptr + ALIGN_INFO);					\
			diff = (uintptr_t)ptr & (ALIGNMENT-1);						\
			if ( diff != 0 )											\
			{															\
				diff = ALIGNMENT - diff;								\
				ptr = (void*)((uintptr_t)ptr + diff);					\
			}															\
			*((ALIGN_TYPE*)((uintptr_t)ptr - ALIGN_INFO)) = 			\
				diff + ALIGN_INFO;										\
		}															


#define UNALIGN( ptr )													\
		if ( ALIGNMENT > 1 )											\
		{																\
			uintptr_t diff = *((ALIGN_TYPE*)((uintptr_t)ptr - ALIGN_INFO));	\
			if ( diff < (ALIGNMENT + ALIGN_INFO) )						\
			{															\
				ptr = (void*)((uintptr_t)ptr - diff);					\
			}															\
		}


static AllocMajor* l_memRoot = nullptr;	///< The root memory block acquired from the system.
static AllocMajor* l_bestBet = nullptr; ///< The major with the most free memory.

static uint64_t l_allocated = 0;		///< Running total of allocated memory.
static uint64_t l_inuse	 = 0;		///< Running total of used memory.

// ----------------------------------------------------------------

static AllocMajor* alloc_new_page(uint32_t size) {
	// This is how much space is required.
	uint32_t st = size + sizeof(AllocMajor) + sizeof(AllocMinor);
	AllocMajor* maj;

	// Perfect amount of space?
	if ((st % ALLOC_PAGE_SIZE) == 0) st  = st / (ALLOC_PAGE_SIZE);
	else st  = st / (ALLOC_PAGE_SIZE) + 1; // No, add the buffer. 

	// Make sure it's >= the minimum size.
	if (st < ALLOC_PAGE_COUNT) st = ALLOC_PAGE_COUNT;
	
	maj = (AllocMajor*)liballoc_alloc(st);
	if (maj == nullptr) return nullptr;	// uh oh, we ran out of memory.
	
	maj->prev = nullptr;
	maj->next = nullptr;
	maj->pages = st;
	maj->size = st * ALLOC_PAGE_SIZE;
	maj->usage = sizeof(AllocMajor);
	maj->first = nullptr;

	l_allocated += maj->size;
	
	return maj;
}

// ----------------------------------------------------------------

void* Prefix(malloc)(size_t req_size) {
	uint64_t size = req_size;

	// For alignment, we adjust size so there's enough space to align.
	if ( ALIGNMENT > 1 ) 
		size += ALIGNMENT + ALIGN_INFO;
	// So, ideally, we really want an alignment of 0 or 1 in order
	// to save space.
	
	liballoc_lock();

	if (size == 0) {
		liballoc_unlock();
		return Prefix(malloc)(1);
	}
	
	if (l_memRoot == nullptr) {	
		// This is the first time we are being used.
		l_memRoot = alloc_new_page(size);
		if (l_memRoot == nullptr) {
		  liballoc_unlock();
		  return nullptr;
		}
	}

	//? Now we need to bounce through every major and find enough space...
	int startedBet = 0;
	uint64_t bestSize = 0;
	void* p = nullptr;
	uintptr_t diff;
	AllocMajor* maj = l_memRoot;
	AllocMinor* min;
	AllocMinor* new_min;
	
	// Start at the best bet....
	if (l_bestBet != nullptr) {
		bestSize = l_bestBet->size - l_bestBet->usage;

		if (bestSize > (size + sizeof(AllocMinor))) {
			maj = l_bestBet;
			startedBet = 1;
		}
	}
	
	while (maj != nullptr) {
		diff  = maj->size - maj->usage;	// free memory in the block

		if (bestSize < diff) {
			// Hmm.. this one has more memory then our bestBet. Remember!
			l_bestBet = maj;
			bestSize = diff;
		}
		
		// CASE 1:  There is not enough space in this major block.
		if (diff < (size + sizeof(AllocMinor))) {				
			// Another major block next to this one?
			if (maj->next != nullptr) {
				maj = maj->next;		// Hop to that one.
				continue;
			}

			// If we started at the best bet, let's start all over again.
			if (startedBet == 1) {
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}

			// Create a new major block next to this one and...
			maj->next = alloc_new_page(size);	// next one will be okay.
			if (maj->next == nullptr) break;			// no more memory.
			maj->next->prev = maj;
			maj = maj->next;

			// .. fall through to CASE 2 ..
		}
		
		// CASE 2: It's a brand new block.
		if (maj->first == NULL) {
			maj->first = (AllocMinor*)((uintptr_t)maj + sizeof(AllocMajor));

			maj->first->magic = ALLOC_MARKER_MAGIC;
			maj->first->prev = nullptr;
			maj->first->next = nullptr;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage 	+= size + sizeof(AllocMinor);

			l_inuse += size;
			
			p = (void*)((uintptr_t)(maj->first) + sizeof(AllocMinor));

			ALIGN(p);

			liballoc_unlock();		// release the lock
			return p;
		}

		// CASE 3: Block in use and enough space at the start of the block.
		diff =  (uintptr_t)(maj->first);
		diff -= (uintptr_t)maj;
		diff -= sizeof(AllocMajor);

		if (diff >= (size + sizeof(AllocMinor))) {
			// Yes, space in front. Squeeze in.
			maj->first->prev = (AllocMinor*)((uintptr_t)maj + sizeof(AllocMajor));
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;
				
			maj->first->magic = ALLOC_MARKER_MAGIC;
			maj->first->prev = nullptr;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage += size + sizeof(AllocMinor);

			l_inuse += size;

			p = (void*)((uintptr_t)(maj->first) + sizeof(AllocMinor));
			ALIGN(p);

			liballoc_unlock();		// release the lock
			return p;
		}

		// CASE 4: There is enough space in this block. But is it contiguous?
		min = maj->first;
		
		// Looping within the block now...
		while (min != nullptr) {
			// CASE 4.1: End of minors in a block. Space from last and end?
			if (min->next == nullptr) {
				// the rest of this block is free...  is it big enough?
				diff = (uintptr_t)(maj) + maj->size;
				diff -= (uintptr_t)min;
				diff -= sizeof(AllocMinor);
				diff -= min->size; // minus already existing usage..

				if (diff >= (size + sizeof(AllocMinor))) {
					// yay....
					min->next = (AllocMinor*)((uintptr_t)min + sizeof(AllocMinor) + min->size);
					min->next->prev = min;
					min = min->next;
					min->next = nullptr;
					min->magic = ALLOC_MARKER_MAGIC;
					min->block = maj;
					min->size = size;
					min->req_size = req_size;
					maj->usage += size + sizeof(AllocMinor);

					l_inuse += size;
					
					p = (void*)((uintptr_t)min + sizeof(AllocMinor));
					ALIGN(p);

					liballoc_unlock();		// release the lock
					return p;
				}
			}

			// CASE 4.2: Is there space between two minors?
			if (min->next != nullptr) {
				// is the difference between here and next big enough?
				diff  = (uintptr_t)(min->next);
				diff -= (uintptr_t)min;
				diff -= sizeof(AllocMinor);
				diff -= min->size; // minus our existing usage.

				if (diff >= (size + sizeof(AllocMinor))) {
					// yay......
					new_min = (AllocMinor*)((uintptr_t)min + sizeof(AllocMinor) + min->size);

					new_min->magic = ALLOC_MARKER_MAGIC;
					new_min->next = min->next;
					new_min->prev = min;
					new_min->size = size;
					new_min->req_size = req_size;
					new_min->block = maj;
					min->next->prev = new_min;
					min->next = new_min;
					maj->usage += size + sizeof(AllocMinor);
					
					l_inuse += size;
					
					p = (void*)((uintptr_t)new_min + sizeof(AllocMinor));
					ALIGN(p);
					
					liballoc_unlock();		// release the lock
					return p;
				}
			}	// min->next != NULL

			min = min->next;
		} // while min != NULL ...

		// CASE 5: Block full! Ensure next block and loop.
		if (maj->next == nullptr) {
			if (startedBet == 1) {
				maj = l_memRoot;
				startedBet = 0;
				continue;
			}
				
			// we've run out. we need more...
			maj->next = alloc_new_page(size);		// next one guaranteed to be okay
			if (maj->next == nullptr) break;			//  uh oh,  no more memory.....
			maj->next->prev = maj;
		}

		maj = maj->next;
	} // while (maj != NULL)

	liballoc_unlock();		// release the lock
	return nullptr;
}

void Prefix(free)(void* ptr) {
	AllocMinor *min;
	AllocMajor *maj;

	if (ptr == nullptr) return;
	
	UNALIGN(ptr);
	liballoc_lock();		// lockit

	min = (AllocMinor*)((uintptr_t)ptr - sizeof(AllocMinor));

	if (min->magic != ALLOC_MARKER_MAGIC) {		
		liballoc_unlock();		// release the lock
		return;
	}

	maj = min->block;
	l_inuse -= min->size;
	maj->usage -= (min->size + sizeof(AllocMinor));
	min->magic = ALLOC_MARKER_DEAD;		// No mojo.

	if (min->next != nullptr) min->next->prev = min->prev;
	if (min->prev != nullptr) min->prev->next = min->next;

	if (min->prev == nullptr) maj->first = min->next; // Might empty the block. This was the first minor.

	// We need to clean up after the majors now....
	if (maj->first == nullptr) {
		if (l_memRoot == maj) l_memRoot = maj->next;
		if (l_bestBet == maj) l_bestBet = nullptr;
		if (maj->prev != nullptr) maj->prev->next = maj->next;
		if (maj->next != nullptr) maj->next->prev = maj->prev;
		l_allocated -= maj->size;

		liballoc_free(maj, maj->pages);
	} else {
		if (l_bestBet != nullptr) {
			int bestSize = l_bestBet->size - l_bestBet->usage;
			int majSize = maj->size - maj->usage;
			if (majSize > bestSize) l_bestBet = maj;
		}
	}
	
	liballoc_unlock();		// release the lock
}

void* Prefix(calloc)(size_t nobj, size_t size) {
       int real_size;
       void* p;

       real_size = nobj * size;
       p = Prefix(malloc)(real_size);
       memset(p, 0, real_size);

       return p;
}

void* Prefix(realloc)(void* p, size_t size) {
	void* ptr;
	AllocMinor* min;
	uint32_t real_size;
	
	// Honour the case of size == 0 => free old and return NULL
	if (size == 0) {
		Prefix(free)(p);
		return nullptr;
	}

	// In the case of a NULL pointer, return a simple malloc.
	if (p == nullptr) return Prefix(malloc)(size);

	// Unalign the pointer if required.
	ptr = p;
	UNALIGN(ptr);

	liballoc_lock();		// lockit

	min = (AllocMinor*)((uintptr_t)ptr - sizeof( AllocMinor ));

	// Ensure it is a valid structure.
	if (min->magic != ALLOC_MARKER_MAGIC) {		
		liballoc_unlock();		// release the lock
		return NULL;
	}	
	
	// Definitely a memory block.
	real_size = min->req_size;

	if (real_size >= size) {
		min->req_size = size;
		liballoc_unlock();
		return p;
	}

	liballoc_unlock();

	// If we got here then we're reallocating to a block bigger than us.
	ptr = Prefix(malloc)(size);					// We need to allocate new memory
	memcpy(ptr, p, real_size);
	Prefix(free)(p);

	return ptr;
}
