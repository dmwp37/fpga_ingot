#ifndef DUMP_STRUCT_H
#define DUMP_STRUCT_H

#define DUMP_STRUCT(pointer) dump_struct(# pointer, pointer)

void dump_struct(const char* name, void* address);

#endif

