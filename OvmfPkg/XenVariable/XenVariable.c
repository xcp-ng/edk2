#include "XenVariable.h"
#include <Library/XenVariableLib.h>
#include <Library/SynchronizationLib.h>

static EFI_EVENT mXenVirtualAddressChangeEvent = NULL;

static SPIN_LOCK var_lock;

static VOID *comm_buf_phys;
VOID *comm_buf;

STATIC
EFI_STATUS
XenGetVariableLocked (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data OPTIONAL
  )
{
  UINT8 *ptr;
  EFI_STATUS status;
  UINT32 attr;

  if (!VariableName || !VendorGuid || !DataSize)
      return EFI_INVALID_PARAMETER;

  ptr = comm_buf;
  serialize_uint32(&ptr, 1); /* version */
  serialize_command(&ptr, COMMAND_GET_VARIABLE);
  serialize_name(&ptr, VariableName);
  serialize_guid(&ptr, VendorGuid);
  serialize_uintn(&ptr, *DataSize);
  serialize_boolean(&ptr, EfiAtRuntime());

  exec_command(comm_buf_phys);

  ptr = comm_buf;
  status = unserialize_result(&ptr);
  switch (status) {
  case EFI_SUCCESS:
    if (!Data)
        return EFI_INVALID_PARAMETER;
    attr = unserialize_uint32(&ptr);
    if (Attributes)
        *Attributes = attr;
    unserialize_data(&ptr, Data, DataSize);
    break;
  case EFI_BUFFER_TOO_SMALL:
    *DataSize = unserialize_uintn(&ptr);
    break;
  default:
    break;
  }

  return status;
}

STATIC
EFI_STATUS
EFIAPI
XenGetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data OPTIONAL
  )
{
  EFI_STATUS status;

  DEBUG ((DEBUG_VARIABLE, "XenGetVariable -> %g-%s (0x%lx)\n", VendorGuid,
          VariableName, *DataSize));

  AcquireSpinLock(&var_lock);

  status = XenGetVariableLocked(VariableName, VendorGuid, Attributes,
                                DataSize, Data);

  ReleaseSpinLock(&var_lock);

  DEBUG ((DEBUG_VARIABLE, "XenGetVariable <- %g-%s (0x%lx, %r)\n", VendorGuid,
          VariableName, *DataSize, status));

  return status;
}

STATIC
EFI_STATUS
XenGetNextVariableNameLocked (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  UINT8 *ptr;
  EFI_STATUS status;

  if (!VariableNameSize || !VariableName || !VendorGuid)
      return EFI_INVALID_PARAMETER;

  if (StrSize(VariableName) > *VariableNameSize)
      return EFI_INVALID_PARAMETER;

  ptr = comm_buf;
  serialize_uint32(&ptr, 1); /* version */
  serialize_command(&ptr, COMMAND_GET_NEXT_VARIABLE);
  serialize_uintn(&ptr, *VariableNameSize);
  serialize_name(&ptr, VariableName);
  serialize_guid(&ptr, VendorGuid);
  serialize_boolean(&ptr, EfiAtRuntime());

  exec_command(comm_buf_phys);

  ptr = comm_buf;
  status = unserialize_result(&ptr);
  switch (status) {
  case EFI_SUCCESS:
    unserialize_data(&ptr, VariableName, VariableNameSize);
    VariableName[*VariableNameSize / 2] = '\0';
    *VariableNameSize += sizeof(*VariableName);
    unserialize_guid(&ptr, VendorGuid);
    break;
  case EFI_BUFFER_TOO_SMALL:
    *VariableNameSize = unserialize_uintn(&ptr);
    break;
  default:
    break;
  }
  return status;
}

STATIC
EFI_STATUS
EFIAPI
XenGetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  EFI_STATUS status;

  DEBUG ((DEBUG_VARIABLE, "XenGetNextVariableName -> %g-%s (0x%lx)\n",
          VendorGuid, VariableName, *VariableNameSize));

  AcquireSpinLock(&var_lock);

  status = XenGetNextVariableNameLocked(VariableNameSize, VariableName,
                                        VendorGuid);

  ReleaseSpinLock(&var_lock);

  DEBUG ((DEBUG_VARIABLE, "XenGetNextVariableName <- %g-%s (0x%lx, %r)\n",
          VendorGuid, VariableName, *VariableNameSize, status));

  return status;
}

STATIC
EFI_STATUS
XenSetVariableLocked (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  )
{
  UINT8 *ptr;

  ptr = comm_buf;
  serialize_uint32(&ptr, 1); /* version */
  serialize_command(&ptr, COMMAND_SET_VARIABLE);
  serialize_name(&ptr, VariableName);
  serialize_guid(&ptr, VendorGuid);
  serialize_data(&ptr, Data, DataSize);
  serialize_uint32(&ptr, Attributes);
  serialize_boolean(&ptr, EfiAtRuntime());

  exec_command(comm_buf_phys);

  ptr = comm_buf;
  return unserialize_result(&ptr);
}

STATIC
EFI_STATUS
EFIAPI
XenSetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
)
{
  EFI_STATUS status;

  DEBUG ((DEBUG_VARIABLE, "XenSetVariable -> %g-%s (0x%x, 0x%lx)\n",
          VendorGuid, VariableName, Attributes, DataSize));

  AcquireSpinLock(&var_lock);

  status = XenSetVariableLocked(VariableName, VendorGuid, Attributes,
                                DataSize, Data);

  ReleaseSpinLock(&var_lock);

  DEBUG ((DEBUG_VARIABLE, "XenSetVariable <- %g-%s (%r)\n", VendorGuid,
          VariableName, status));

  return status;
}

STATIC
EFI_STATUS
XenQueryVariableInfoLocked (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  UINT8 *ptr;
  EFI_STATUS status;

  ptr = comm_buf;
  serialize_uint32(&ptr, 1); /* version */
  serialize_command(&ptr, COMMAND_QUERY_VARIABLE_INFO);
  serialize_uint32(&ptr, Attributes);

  exec_command(comm_buf_phys);

  ptr = comm_buf;
  status = unserialize_result(&ptr);
  switch (status) {
  case EFI_SUCCESS:
    *MaximumVariableStorageSize = unserialize_uint64(&ptr);
    *RemainingVariableStorageSize = unserialize_uint64(&ptr);
    *MaximumVariableSize = unserialize_uint64(&ptr);
    break;
  default:
    break;
  }
  return status;
}

STATIC
EFI_STATUS
EFIAPI
XenQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  EFI_STATUS status;

  DEBUG ((DEBUG_VARIABLE, "XenQueryVariableInfo -> (0x%x)\n", Attributes));

  AcquireSpinLock(&var_lock);

  status = XenQueryVariableInfoLocked(Attributes, MaximumVariableStorageSize,
                                      RemainingVariableStorageSize,
                                      MaximumVariableSize);

  ReleaseSpinLock(&var_lock);

  DEBUG ((DEBUG_VARIABLE, "XenQueryVariableInfo <- (0x%lx, 0x%lx, 0x%lx, %r)\n",
          *MaximumVariableStorageSize, *RemainingVariableStorageSize,
          *MaximumVariableSize, status));

  return status;
}

STATIC
VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  /*
   * Convert the comm_buf pointer from a physical to a virtual address for use
   * at runtime.
   */
  EfiConvertPointer (0x0, (VOID **) &comm_buf);
}

EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      NewHandle;

  comm_buf_phys = AllocateRuntimePages(SHMEM_PAGES);
  comm_buf = comm_buf_phys;

  InitializeSpinLock(&var_lock);

  SystemTable->RuntimeServices->GetVariable         = XenGetVariable;
  SystemTable->RuntimeServices->GetNextVariableName = XenGetNextVariableName;
  SystemTable->RuntimeServices->SetVariable         = XenSetVariable;
  SystemTable->RuntimeServices->QueryVariableInfo   = XenQueryVariableInfo;

  //
  // Now install the Variable Runtime Architectural Protocol on a new handle
  //
  NewHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewHandle,
                  &gEfiVariableArchProtocolGuid,
                  NULL,
                  &gEfiVariableWriteArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  VariableClassAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mXenVirtualAddressChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = PcdSetBoolS (PcdOvmfFlashVariablesEnable, TRUE);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
