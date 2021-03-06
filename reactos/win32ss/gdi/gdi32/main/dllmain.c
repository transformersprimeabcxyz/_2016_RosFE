/*
 * dllmain.c
 */

#include <precomp.h>

extern HGDIOBJ stock_objects[];
BOOL SetStockObjects = FALSE;
PDEVCAPS GdiDevCaps = NULL;
PGDIHANDLECACHE GdiHandleCache = NULL;
BOOL gbLpk = FALSE;
RTL_CRITICAL_SECTION semLocal;
extern CRITICAL_SECTION gcsClientObjLinks;

/*
 * GDI32.DLL does have an entry point for disable threadlibrarycall,. The initialization is done by a call
 * to GdiDllInitialize(). This call is done from the entry point of USER32.DLL.
 */
BOOL
WINAPI
DllMain (
    HANDLE  hDll,
    DWORD   dwReason,
    LPVOID  lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH :
        DisableThreadLibraryCalls(hDll);
        break;

    default:
        break;
    }
    return TRUE;
}


VOID
WINAPI
GdiProcessSetup (VOID)
{
    hProcessHeap = GetProcessHeap();

    /* map the gdi handle table to user space */
    GdiHandleTable = NtCurrentTeb()->ProcessEnvironmentBlock->GdiSharedHandleTable;
    GdiSharedHandleTable = NtCurrentTeb()->ProcessEnvironmentBlock->GdiSharedHandleTable;
    GdiDevCaps = &GdiSharedHandleTable->DevCaps;
    CurrentProcessId = NtCurrentTeb()->ClientId.UniqueProcess;
    GDI_BatchLimit = (DWORD) NtCurrentTeb()->ProcessEnvironmentBlock->GdiDCAttributeList;
    GdiHandleCache = (PGDIHANDLECACHE)NtCurrentTeb()->ProcessEnvironmentBlock->GdiHandleBuffer;
    RtlInitializeCriticalSection(&semLocal);
    InitializeCriticalSection(&gcsClientObjLinks);
}


/*
 * @implemented
 */
BOOL
WINAPI
GdiDllInitialize (
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        GdiProcessSetup ();
        break;

    case DLL_THREAD_ATTACH:
        NtCurrentTeb()->GdiTebBatch.Offset = 0;
        NtCurrentTeb()->GdiBatchCount = 0;
        break;

    default:
        return FALSE;
    }

    // Very simple, the list will fill itself as it is needed.
    if(!SetStockObjects)
    {
        RtlZeroMemory( &stock_objects, NB_STOCK_OBJECTS); //Assume Ros is dirty.
        SetStockObjects = TRUE;
    }

    return TRUE;
}

/* EOF */
