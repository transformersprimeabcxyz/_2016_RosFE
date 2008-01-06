/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ke/powerpc/thread.c
 * PURPOSE:         i386 Thread Context Creation
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 *                  arty (ppc adaptation)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>
#include <ndk/powerpc/ketypes.h>

typedef struct _KSWITCHFRAME
{
    PVOID ExceptionList;
    BOOLEAN ApcBypassDisable;
    PVOID RetAddr;
} KSWITCHFRAME, *PKSWITCHFRAME;

typedef struct _KSTART_FRAME
{
    PKSYSTEM_ROUTINE SystemRoutine;
    PKSTART_ROUTINE StartRoutine;
    PVOID StartContext;
    BOOLEAN UserThread;
} KSTART_FRAME, *PKSTART_FRAME;

typedef struct _KUINIT_FRAME
{
    KSWITCHFRAME CtxSwitchFrame;
    KSTART_FRAME StartFrame;
    KTRAP_FRAME TrapFrame;
    FX_SAVE_AREA FxSaveArea;
} KUINIT_FRAME, *PKUINIT_FRAME;

typedef struct _KKINIT_FRAME
{
    KSWITCHFRAME CtxSwitchFrame;
    KSTART_FRAME StartFrame;
    FX_SAVE_AREA FxSaveArea;
} KKINIT_FRAME, *PKKINIT_FRAME;

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
KePPCInitThreadWithContext(IN PKTHREAD Thread,
                           IN PKSYSTEM_ROUTINE SystemRoutine,
                           IN PKSTART_ROUTINE StartRoutine,
                           IN PVOID StartContext,
                           IN PCONTEXT ContextPointer)
{
    PFX_SAVE_AREA FxSaveArea;
    PKSTART_FRAME StartFrame;
    PKSWITCHFRAME CtxSwitchFrame;
    PKTRAP_FRAME TrapFrame;
    CONTEXT LocalContext;
    PCONTEXT Context = NULL;
    ULONG ContextFlags;

    /* Check if this is a With-Context Thread */
    if (ContextPointer)
    {
        /* Set up the Initial Frame */
        PKUINIT_FRAME InitFrame;
        InitFrame = (PKUINIT_FRAME)((ULONG_PTR)Thread->InitialStack -
                                    sizeof(KUINIT_FRAME));

        /* Copy over the context we got */
        RtlCopyMemory(&LocalContext, ContextPointer, sizeof(CONTEXT));
        Context = &LocalContext;
        ContextFlags = CONTEXT_CONTROL;

        /* Zero out the trap frame and save area */
        RtlZeroMemory(&InitFrame->TrapFrame,
                      KTRAP_FRAME_LENGTH + sizeof(FX_SAVE_AREA));

        /* Setup the Fx Area */
        FxSaveArea = &InitFrame->FxSaveArea;

        /* Disable any debug regiseters */
        Context->ContextFlags &= ~CONTEXT_DEBUG_REGISTERS;

        /* Setup the Trap Frame */
        TrapFrame = &InitFrame->TrapFrame;

        /* Set up a trap frame from the context. */
        KeContextToTrapFrame(Context,
                             NULL,
                             TrapFrame,
                             Context->ContextFlags | ContextFlags,
                             UserMode);

        /* Set the previous mode as user */
        TrapFrame->PreviousMode = UserMode;

        /* Terminate the Exception Handler List */
        RtlZeroMemory(TrapFrame->ExceptionRecord, sizeof(TrapFrame->ExceptionRecord));

        /* Setup the Stack for KiThreadStartup and Context Switching */
        StartFrame = &InitFrame->StartFrame;
        CtxSwitchFrame = &InitFrame->CtxSwitchFrame;

        /* Tell the thread it will run in User Mode */
        Thread->PreviousMode = UserMode;

        /* Tell KiThreadStartup of that too */
        StartFrame->UserThread = TRUE;
    }
    else
    {
        /* Set up the Initial Frame for the system thread */
        PKKINIT_FRAME InitFrame;
        InitFrame = (PKKINIT_FRAME)((ULONG_PTR)Thread->InitialStack -
                                    sizeof(KKINIT_FRAME));

        /* Setup the Fx Area */
        FxSaveArea = &InitFrame->FxSaveArea;
        RtlZeroMemory(FxSaveArea, sizeof(FX_SAVE_AREA));

        /* Setup the Stack for KiThreadStartup and Context Switching */
        StartFrame = &InitFrame->StartFrame;
        CtxSwitchFrame = &InitFrame->CtxSwitchFrame;

        /* Tell the thread it will run in Kernel Mode */
        Thread->PreviousMode = KernelMode;

        /* Tell KiThreadStartup of that too */
        StartFrame->UserThread = FALSE;
    }

    /* Now setup the remaining data for KiThreadStartup */
    StartFrame->StartContext = StartContext;
    StartFrame->StartRoutine = StartRoutine;
    StartFrame->SystemRoutine = SystemRoutine;

    /* And set up the Context Switch Frame */
    CtxSwitchFrame->RetAddr = KiThreadStartup;
    CtxSwitchFrame->ApcBypassDisable = TRUE;
    CtxSwitchFrame->ExceptionList = EXCEPTION_CHAIN_END;;

    /* Save back the new value of the kernel stack. */
    Thread->KernelStack = (PVOID)CtxSwitchFrame;
}

/* EOF */


