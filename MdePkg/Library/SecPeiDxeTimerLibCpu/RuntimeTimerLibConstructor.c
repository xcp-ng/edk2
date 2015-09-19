/** @file
  Contructor for Timer Library functions

  Copyright (c) 2022, Citrix Systems, Inc.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

extern UINT32 mFSBClock;

RETURN_STATUS
EFIAPI
TimerLibConstruct (
  VOID
  )
{
  //
  // Cache current value of PcdFSBClock when it's a dynamic PCD.
  //
  mFSBClock = PcdGet32 (PcdFSBClock);
  return RETURN_SUCCESS;
}
