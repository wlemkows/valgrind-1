#ifndef MMT_TRACE_H_
#define MMT_TRACE_H_

#include "pub_tool_basics.h"

#include <sys/select.h>

#ifdef __LP64__
#define MMT_64BIT
#endif

//#define MMT_PRINT_FILENAMES
#define MMT_DEBUG
//#define MMT_DEBUG_VERBOSE
#define MMT_MAX_TRACE_FILES 10
#define MMT_MAX_REGIONS 1000

extern int mmt_binary_output;

struct mmt_mmap_data {
	Addr start;
	Addr end;
	int fd;
	Off64T offset;
	UInt id;
	UWord data1;
	UWord data2;
};

struct mmt_trace_file {
	const char *path;
	fd_set fds;
};

#define maybe_unused __attribute__((unused))

extern int mmt_trace_opens;
extern struct mmt_trace_file mmt_trace_files[MMT_MAX_TRACE_FILES];
extern int mmt_trace_all_files;

void mmt_free_region(struct mmt_mmap_data *m);
struct mmt_mmap_data *mmt_add_region(int fd, Addr start, Addr end,
		Off64T offset, UInt id, UWord data1, UWord data2);

struct mmt_mmap_data *mmt_find_region_by_fd_offset(int fd, Off64T offset);
struct mmt_mmap_data *mmt_find_region_by_fdset_offset(fd_set *fds, Off64T offset);
struct mmt_mmap_data *mmt_find_region_by_fdset_data(fd_set *fds, UWord data1, UWord data2);

void mmt_pre_syscall(ThreadId tid, UInt syscallno, UWord *args, UInt nArgs);

void mmt_post_syscall(ThreadId tid, UInt syscallno, UWord *args, UInt nArgs, SysRes res);

struct mmt_mmap_data *__find_mmap_slow(Addr addr);

#define force_inline	inline __attribute__((always_inline))

struct negative_region {
	Addr start, end;
	int score;
};

extern struct negative_region neg_regions[];
extern struct mmt_mmap_data *last_used_region;

static force_inline struct mmt_mmap_data *find_mmap(Addr addr)
{
	struct negative_region *neg = neg_regions;

	if (LIKELY(addr >= neg->start && addr < neg->end))
	{
		neg->score++;
		return NULL;
	}

	if (LIKELY(addr >= last_used_region->start && addr < last_used_region->end))
		return last_used_region;

	return __find_mmap_slow(addr);
}

#endif /* MMT_TRACE_H_ */
