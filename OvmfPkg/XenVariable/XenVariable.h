#ifndef _XEN_VARIABLE_H
#define _XEN_VARIABLE_H

//
// Statements that include other header files
//
#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DevicePathLib.h>

#include <Guid/VariableFormat.h>
#include <Guid/GlobalVariable.h>
#include <Protocol/Variable.h>
#include <Protocol/VariableWrite.h>


//
// Functions
//

EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

#endif
