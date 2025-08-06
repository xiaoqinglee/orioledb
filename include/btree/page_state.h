/*-------------------------------------------------------------------------
 *
 * page_state.h
 *		Declarations of OrioleDB B-tree page state.
 *
 * Copyright (c) 2021-2025, Oriole DB Inc.
 *
 * IDENTIFICATION
 *	  contrib/orioledb/include/btree/page_state.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef __BTREE_PAGE_STATE_H__
#define __BTREE_PAGE_STATE_H__

#include "btree.h"
#include "page_contents.h"

/* Flags stored in OrioleDBPageHeader.state */
#define PAGE_STATE_LOCKED_FLAG	UINT64CONST(0x0000000000040000)
#define PAGE_STATE_NO_READ_FLAG	UINT64CONST(0x0000000000080000)
#define PAGE_STATE_CHANGE_COUNT_ONE UINT64CONST(0x0000000000100000)
#define PAGE_STATE_CHANGE_COUNT_MASK UINT64CONST(0x000FFFFFFFF00000)
#define PAGE_STATE_CHANGE_NON_WAITERS_MASK UINT64CONST(0x000FFFFFFFFC0000)
#define PAGE_STATE_CHANGE_USAGE_COUNT_MASK UINT64CONST(0x00F0000000000000)
#define PAGE_STATE_CHANGE_USAGE_COUNT_ONE UINT64CONST(0x0010000000000000)
#define PAGE_STATE_CHANGE_USAGE_COUNT_SHIFT (52)
#define PAGE_STATE_LIST_TAIL_MASK UINT64CONST(0x000000000003FFFF)

#define PAGE_STATE_INVALID_PROCNO PAGE_STATE_LIST_TAIL_MASK

/* Macros for dealing with OrioleDBPageHeader.state */
#define O_PAGE_STATE_IS_LOCKED(state) ((state) & PAGE_STATE_LOCKED_FLAG)
#define O_PAGE_STATE_LOCK(state) ((state) | PAGE_STATE_LOCKED_FLAG)
#define O_PAGE_STATE_BLOCK_READ(state) ((state) | PAGE_STATE_LOCKED_FLAG | PAGE_STATE_NO_READ_FLAG)
#define O_PAGE_STATE_READ_IS_BLOCKED(state) ((state) & PAGE_STATE_NO_READ_FLAG)

#define O_PAGE_STATE_GET_USAGE_COUNT(state) (((state) & PAGE_STATE_CHANGE_USAGE_COUNT_MASK) >> PAGE_STATE_CHANGE_USAGE_COUNT_SHIFT)
#define O_PAGE_STATE_SET_USAGE_COUNT(state, usageCount) (((state) & ~PAGE_STATE_CHANGE_USAGE_COUNT_MASK) | ((uint64) (usageCount) << PAGE_STATE_CHANGE_USAGE_COUNT_SHIFT))

#define BTREE_PAGE_MAX_CHUNK_ITEMS \
	(ORIOLEDB_BLCKSZ / (MAXIMUM_ALIGNOF + sizeof(LocationIndex)))

#define BTREE_PAGE_MAX_SPLIT_ITEMS (2 * BTREE_PAGE_MAX_CHUNK_ITEMS)

typedef enum
{
	OLockPageWithTupleResultLocked,
	OLockPageWithTupleResultRefindNeeded,
	OLockPageWithTupleResultInserted
} OLockPageWithTupleResult;

/*
 * Enable this to recheck page struct on every unlock.
 */
/* #define CHECK_PAGE_STRUCT */

#ifdef CHECK_PAGE_STRUCT
extern void o_check_page_struct(BTreeDescr *desc, Page p);
#endif

extern Size page_state_shmem_needs(void);
extern void page_state_shmem_init(Pointer buf, bool found);
extern bool have_locked_pages(void);
extern int	get_waiters_with_tuples(BTreeDescr *desc,
									OInMemoryBlkno blkno,
									int result[BTREE_PAGE_MAX_SPLIT_ITEMS]);
extern void wakeup_waiters_with_tuples(OInMemoryBlkno blkno,
									   int procnums[BTREE_PAGE_MAX_SPLIT_ITEMS],
									   int count);
extern void lock_page(OInMemoryBlkno blkno);
extern OLockPageWithTupleResult lock_page_with_tuple(BTreeDescr *desc,
													 OInMemoryBlkno *blkno,
													 uint32 *pageChangeCount,
													 OTupleXactInfo xactInfo,
													 OTuple tuple);
extern void relock_page(OInMemoryBlkno blkno);
extern bool try_lock_page(OInMemoryBlkno blkno);
extern void delare_page_as_locked(OInMemoryBlkno blkno);
extern bool page_is_locked(OInMemoryBlkno blkno);
extern void page_block_reads(OInMemoryBlkno blkno);
extern void unlock_page(OInMemoryBlkno blkno);
extern void unlock_page_after_split(BTreeDescr *desc, OInMemoryBlkno blkno,
									OInMemoryBlkno rightBlkno,
									int *procnums, int procnumsCount);
extern void release_all_page_locks(void);
extern void page_wait_for_read_enable(OInMemoryBlkno blkno);
extern void btree_register_inprogress_split(OInMemoryBlkno rightBlkno);
extern void btree_unregister_inprogress_split(OInMemoryBlkno rightBlkno);
extern void btree_mark_incomplete_splits(void);
extern void btree_split_mark_finished(OInMemoryBlkno rightBlkno, bool use_lock,
									  bool success);

#endif							/* __BTREE_PAGE_STATE_H__ */
