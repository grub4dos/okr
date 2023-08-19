
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <unistd.h>

#include "secaudit.h"

struct CHKALLOC *firstalloc=NULL;

gid_t getgid()
{
	return 0;
}

struct group *getgrnam(const char *group)
{
	//ntfs_log_error("Cannot interpret id \"%s\"", group);
	//ntfs_log_error("please use numeric gids in UserMapping file\n");
	return ((struct group*)NULL);
}


void dumpalloc(const char *txt)
{
	struct CHKALLOC *q;

	if (firstalloc) {
		printf("alloc table at %s\n",txt);
		for (q=firstalloc; q; q=q->next)
			printf("%016llx : %u bytes at %016llx allocated at %s line %d\n",
			(s64)q,(unsigned int)q->size,
			(s64)q->alloc,q->file,q->line);
	}
}

//#define malloc(size) AllocatePool(size)
//#define free(Buffer) FreePool(Buffer)

void *chkmalloc(size_t size, const char *file, int line)
{
	void *p;
	struct CHKALLOC *q;

	p = (void*)malloc(size+1);
	if (p) {
		((unsigned char*)p)[size] = 0xaa;
		q = (struct CHKALLOC*)malloc(sizeof(struct CHKALLOC));
		if (q) {
			q->next = firstalloc;
			q->alloc = p;
			q->size = size;
			q->file = file;
			q->line = line;
			firstalloc = q;
		}
	}
	return (p);
}

void *chkcalloc(size_t cnt, size_t size, const char *file, int line)
{
	return (chkmalloc(cnt*size,file,line));
}

void chkfree(void *p, const char *file, int line)
{
	struct CHKALLOC *q;
	struct CHKALLOC *r;

	if (p) {
		if (firstalloc && (firstalloc->alloc == p)) {
			r = firstalloc;
			firstalloc = firstalloc->next;
		} else {
			q = firstalloc;
			if (q)
				while (q->next && (q->next->alloc != p))
					q = q->next;
			if (q && q->next) {
				r = q->next;
				q->next = r->next;
			} else {
				r = (struct CHKALLOC*)NULL;
				printf("** freeing unallocated memory in %s line %d\n",file,line);
				if (!isatty(1))
					fprintf(stderr,"** freeing unallocated memory in %s line %d\n",file,line);
			}
		}
		if (r) {
			if (((unsigned char*)p)[r->size] != 0xaa) {
				printf("** memory corruption, alloc in %s line %d release in %s %d\n",
					r->file,r->line,file,line);
				if (!isatty(1))
					fprintf(stderr,"** memory corruption, alloc in %s line %d release in %s %d\n",
					r->file,r->line,file,line);
			}
			memset(p,0xaa,r->size);
			free(r);
			free(p);
		}
	}
}
