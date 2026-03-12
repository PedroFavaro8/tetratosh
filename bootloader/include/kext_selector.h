#ifndef _KEXT_SELECTOR_H_
#define _KEXT_SELECTOR_H_

#include "tetratosh.h"

EFI_STATUS SelectKexts(SYSTEM_INFO *Info, KEXT_LIST *List);
VOID ShowKextList(KEXT_LIST *List);

#endif
