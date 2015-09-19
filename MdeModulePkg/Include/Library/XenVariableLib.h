#ifndef _XEN_VARIABLE_LIB_H
#define _XEN_VARIABLE_LIB_H

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include "Library/IoLib.h"

enum command_t {
  COMMAND_GET_VARIABLE,
  COMMAND_SET_VARIABLE,
  COMMAND_GET_NEXT_VARIABLE,
  COMMAND_QUERY_VARIABLE_INFO,
  COMMAND_NOTIFY_SB_FAILURE,
};

#define PORT_ADDRESS 0x0100
#define SHMEM_PAGES  16

static inline void
serialize_name(UINT8 **ptr, CHAR16 *VariableName)
{
  UINTN VarNameSize = StrLen(VariableName) * sizeof(*VariableName);
  CopyMem (*ptr, &VarNameSize, sizeof VarNameSize);
  *ptr += sizeof VarNameSize;
  CopyMem (*ptr, VariableName, VarNameSize);
  *ptr += VarNameSize;
}

static inline void
serialize_data(UINT8 **ptr, VOID *Data, UINTN DataSize)
{
  CopyMem (*ptr, &DataSize, sizeof DataSize);
  *ptr += sizeof DataSize;
  CopyMem (*ptr, Data, DataSize);
  *ptr += DataSize;
}

static inline void
serialize_uintn(UINT8 **ptr, UINTN var)
{
  CopyMem (*ptr, &var, sizeof var);
  *ptr += sizeof var;
}

static inline void
serialize_uint32(UINT8 **ptr, UINT32 var)
{
  CopyMem (*ptr, &var, sizeof var);
  *ptr += sizeof var;
}

static inline void
serialize_boolean(UINT8 **ptr, BOOLEAN var)
{
  CopyMem (*ptr, &var, sizeof var);
  *ptr += sizeof var;
}

static inline void
serialize_command(UINT8 **ptr, enum command_t cmd)
{
  serialize_uint32(ptr, (UINT32)cmd);
}

static inline void
serialize_guid(UINT8 **ptr, EFI_GUID *Guid)
{
  CopyMem (*ptr, Guid, 16);
  *ptr += 16;
}

static inline void
unserialize_data(UINT8 **ptr, VOID *Data, UINTN *DataSize)
{
  CopyMem(DataSize, *ptr, sizeof(*DataSize));
  *ptr += sizeof(*DataSize);
  CopyMem(Data, *ptr, *DataSize);
  *ptr += *DataSize;
}

static inline UINTN
unserialize_uintn(UINT8 **ptr)
{
  UINTN ret;

  CopyMem(&ret, *ptr, sizeof ret);
  *ptr += sizeof ret;

  return ret;
}

static inline UINT32
unserialize_uint32(UINT8 **ptr)
{
  UINT32 ret;

  CopyMem(&ret, *ptr, sizeof ret);
  *ptr += sizeof ret;

  return ret;
}

static inline UINT64
unserialize_uint64(UINT8 **ptr)
{
  UINT64 ret;

  CopyMem(&ret, *ptr, sizeof ret);
  *ptr += sizeof ret;

  return ret;
}

static inline void
unserialize_guid(UINT8 **ptr, EFI_GUID *Guid)
{
  CopyMem (Guid, *ptr, 16);
  *ptr += 16;
}

static inline EFI_STATUS
unserialize_result(UINT8 **ptr)
{
  EFI_STATUS status;

  CopyMem(&status, *ptr, sizeof status);
  *ptr += sizeof status;

  return status;
}

static inline void
exec_command(VOID *buf)
{
  MemoryFence ();
  IoWrite32 (PORT_ADDRESS, ((UINTN)buf) >> 12);
  MemoryFence ();
}

#endif

