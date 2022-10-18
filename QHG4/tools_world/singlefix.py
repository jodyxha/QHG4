from sys import argv, exit
import os
import h5py
import grid_fixer as gf

if ('QHG4_DIR') in os.environ:
    QHG4_BASE = os.environ['QHG4_DIR']
else:
    QHG4_BASE    =  os.environ['HOME']+"/progs/QHG4/"
#-- end if

CORE_DATA    = QHG4_BASE+"/useful_stuff/core_data"
gridfix_file = CORE_DATA+"/gridfixes2_darda_256.dat"

gfixer = gf.GridFixer()
iResult = gfixer.applyFixes(argv[1], gridfix_file)
