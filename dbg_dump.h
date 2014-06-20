#ifndef DBG_DUMP_H
#define DBG_DUMP_H

#define DBG_PRINT(name) dbg_dump(# name)

void dbg_dump(const char* name);

#endif

