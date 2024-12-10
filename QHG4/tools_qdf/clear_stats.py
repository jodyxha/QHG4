#!/usr/bin/python

from sys import argv
import h5py

if len(argv) > 1:

  [qdf_file,species] = argv[1].split(':')

  h=h5py.File(qdf_file, "r+")
  nc=h['Grid'].attrs['NumCells'][0]
  nulls=[0]*nc
  negs=[-1]*nc

  dsnames=[('Dist',nulls), ('Hops',negs), ('Time',negs)]
  for s in dsnames:
      dd=h['Populations/%s/%s'%(species, s[0])]
      dd[...]=s[1]
  #-- end for
  h.flush()
  h.close()
else:
    print("%s - clear move stats of qdf file"%argv[0])
    print("usage:")
    print("  %s <qdf-file>:[<species>]"%argv[0])
#-- end if
