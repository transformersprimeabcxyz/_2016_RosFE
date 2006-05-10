#ifndef __NTOSKRNL_INCLUDE_INTERNAL_PO_H
#define __NTOSKRNL_INCLUDE_INTERNAL_PO_H

extern PDEVICE_NODE PopSystemPowerDeviceNode;

VOID
NTAPI
PoInit(
    PROS_LOADER_PARAMETER_BLOCK LoaderBlock, 
    BOOLEAN ForceAcpiDisable
);

NTSTATUS
NTAPI
PopSetSystemPowerState(SYSTEM_POWER_STATE PowerState);

#endif /* __NTOSKRNL_INCLUDE_INTERNAL_PO_H */
