#ifndef __PERMDUMPRESTORE_H__
#define __PERMDUMPRESTORE_H__

#include <hdf5.h>

#include "Permutator.h"

int dumpPerm(Permutator **apPerm, int iNumPerm, const char *pOwner, hid_t hSpeciesGroup);
int restorePerm(Permutator **apPerm, int iNumPerm, const char *pOwner, hid_t hSpeciesGroup);

#endif
