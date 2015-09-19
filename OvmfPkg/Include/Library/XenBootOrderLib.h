/** @file

  Copyright (C) 2018 Citrix Systems Ltd.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __XEN_BOOT_ORDER_LIB_H__
#define __XEN_BOOT_ORDER_LIB_H__

#include <Uefi/UefiBaseType.h>
#include <Base.h>

/**
  Set the boot order based on configuration retrieved from QEMU via CMOS.

  Platform BDS should call this function after connecting any expected boot
  devices and calling EfiBootManagerRefreshAllBootOption ().

  @retval RETURN_SUCCESS            BootOrder NvVar rewritten.

  @retval RETURN_OUT_OF_RESOURCES   Memory allocation failed.

  @return                           Values returned by gBS->LocateProtocol ()
                                    or gRT->SetVariable ().

**/
RETURN_STATUS
EFIAPI
SetBootOrderFromXen (
  VOID
  );

#endif
