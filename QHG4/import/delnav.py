#!/usr/bin/python
import h5py
from sys import argv

h=h5py.File(argv[1], "r+")
del h["/Navigation"]
h.flush()
h.close()

