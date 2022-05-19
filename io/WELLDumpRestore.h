#ifndef __WELLDUMPRESTORE_H__
#define __WELLDUMPRESTORE_H__

#include <string>
#include <hdf5.h>

#include "WELL512.h"

int dumpWELL(WELL512 **apWELL, int iNumWELL, const std::string sOwner, hid_t hSpeciesGroup);
int restoreWELL(WELL512 **apWELL, int iNumWELL, const std::string sOwner, hid_t hSpeciesGroup);

#endif
