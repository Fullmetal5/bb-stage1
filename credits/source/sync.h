#ifndef __SYNC_H__
#define __SYNC_H__

#include "types.h"

void sync_before_read(void *p, u32 len);
void sync_after_write(const void *p, u32 len);
void sync_before_exec(const void *p, u32 len);

#endif
