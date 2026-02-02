#ifndef SAH_H
#define SAH_H

/*
 * SAH is implemented both to Linux and Windows
 * to reach your desired version jump or filter
 * between "__unix__" or "_WIN32"
 *
 * current status:
 * Linux	Ready (functional but incomplete)
 * Windows	W.I.P
 * */

#ifdef __unix__


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define STACK_SIZE 4096


/* =========================
   Private types
   ========================= */


struct _stack_header {
	size_t size;
};


/* =========================
   Public Types
   ========================= */


struct sah_stack {
	uint8_t* bp;
	uint8_t* sp;
};


/* =========================
   Public API
   ========================= */


struct sah_stack* stack_create(void);
void stack_destroy(struct sah_stack*);
static inline void* push(struct sah_stack*, size_t);
static inline void pop(struct sah_stack*, size_t);
static inline void* spush(struct sah_stack*, size_t);
static inline void spop(struct sah_stack*);


/* =========================
   Implementation
   ========================= */

static inline void* push(struct sah_stack* s, size_t n)
{
	s->sp -= n;
	return s->sp;
}

static inline void pop(struct sah_stack* s, size_t n)
{
	s->sp += n;
}

#ifdef SAH_IMPLEMENTATION

struct sah_stack* stack_create(void)
{
	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + STACK_SIZE;
	uint8_t* mem = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mem == MAP_FAILED)
		return NULL;

	mprotect(mem, guard, PROT_NONE);

	struct sah_stack* s = malloc(sizeof(struct sah_stack));
	if (!s) {
		munmap(mem, total);
		return NULL;
	}

	s->bp = mem + total;
	s->sp = s->bp;

	return s;
}

void stack_destroy(struct sah_stack* s)
{
	if (s == NULL) return;

	size_t guard = sysconf(_SC_PAGESIZE);
	size_t total = guard + STACK_SIZE;

	uint8_t* mem = s->bp - total;
	
	munmap(mem, total);
	free(s);
}

void* spush(struct sah_stack* s, size_t n)
{
	size_t total = sizeof(struct _stack_header) + n;

	s->sp -= total;

	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	hdr->size = n;

	return (void*)(hdr + 1);
}

void spop(struct sah_stack* s)
{
	struct _stack_header* hdr = (struct _stack_header*)s->sp;
	size_t total = sizeof(struct _stack_header) + hdr->size;

	s->sp += total;
}


#endif /* LINUX_IMPLEMENTATION */
#endif /* __unix__ */


#ifdef _WIN32


// W.I.P


#endif /* _WIN32 */
#endif /* SAH_H */
