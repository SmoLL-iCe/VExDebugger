#pragma once
#include <windows.h>
#include <winnt.h>

typedef struct _VECTORED_HANDLER_ENTRY
{
    LIST_ENTRY ExecuteHandlerList;
    union
    {
        struct
        {
            ULONG Refs;
            PVOID Handler;
        } Old;
        struct
        {
            PVOID Unknown1;
            ULONG Unknown2;
            PVOID Handler;
        } New;
    };
} VECTORED_HANDLER_ENTRY, * PVECTORED_HANDLER_ENTRY;

typedef struct _VECTORED_HANDLER_LIST
{
    SRWLOCK SrwLock;

    LIST_ENTRY ExecuteHandlerList;

} VECTORED_HANDLER_LIST, * PVECTORED_HANDLER_LIST;