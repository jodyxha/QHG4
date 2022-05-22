#!/usr/bin/python
from sys import argv
import os
import shutil
import h5py
import numpy as np
import QDFWriter
import ArrayExtractor


#-----------------------------------------------------------------------------
#-- QDFInterpolator
#--
class QDFInterpolator:
    
    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdf_file1, qdf_file2):
        self.AE1       = ArrayExtractor.ArrayExtractor(qdf_file1)
        self.AE2       = ArrayExtractor.ArrayExtractor(qdf_file2)
        self.arr_descs = []
        self.interp_arrs = {}
    #-- end def


    #-----------------------------------------------------------------------------
    #-- extract_arrays
    #--
    def extract_arrays(self, arr_descs):
        self.arr_descs = arr_descs
        bOK1 = self.AE1.extract_arrays(arr_descs)
        bOK2 = self.AE2.extract_arrays(arr_descs)
        
        return bOK1 and bOK2
    #-- end def


    #-----------------------------------------------------------------------------
    #-- calc_increments
    #--
    def calc_increments(self, delta):
        self.interp_arrs = {}
        for arr in self.arr_descs:
            cur_a1 = np.array(self.AE1.get_array(arr))
            cur_a2 = np.array(self.AE2.get_array(arr))
            cur_a3 = (cur_a2 - cur_a1)/delta
            self.interp_arrs[arr] = cur_a3
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- interpolate
    #--
    def interpolate(self,  arr_descs, delta):
        bOK = self.extract_arrays(arr_descs)
        if (bOK):
            self.calc_increments(delta)
        #-- end if
        return bOK
    #-- end def

#-- end class

if __name__ == '__main__':
    if len(argv) > 5:
        qdf_file1   = argv[1]
        qdf_file2   = argv[2]
        delta       = float(argv[3])
        out_file    = argv[4]
        array_descs = argv[5:]
        
        print("qdf_file1: [%s]" % (qdf_file1))
        print("qdf_file2: [%s]" % (qdf_file2))
        print("delta:     [%f]" % (delta))
        print("out_file:  [%s]" % (out_file))
        print("array descs:")
        for s in array_descs:
            print("    [%s]" % s)
        #-- end if

        
        qi = QDFInterpolator(qdf_file1, qdf_file2)
        qi.interpolate(array_descs, delta)

        qdfw = QDFWriter.QDFWriter(out_file)
        qdfw.add_top_level_string_attribute("QHG", "a QHG data file")
        qdfw.add_top_level_attribute("Time", "15002")
        arrstr = "#".join(array_descs)
        qdfw.add_top_level_string_attribute("Targets", arrstr)
        ia = qi.interp_arrs
        for arr in ia:
            groupname, arrname = arr.split('/')
        
            gg=qdfw.add_group(groupname)
            qdfw.add_array(gg, arrname, ia[arr])
        #-- end for
        qdfw.write()
        print("trallala")
    else:
        print("%s - InterpolatExtract arrays from two QDF files and write as qdf file" % (argv[0]))
        print("Usage:")
        print("  %s <qdf_file1>  <qdf_file2> <delta> <out_file> [<group_name>/<array_name>]*" % (argv[0]))
        print("where");
        print("  qdf_file1   name of QDF environment file")
        print("  qdf_file2   name of QDF environment file")
        print("  delta       number of time steps between the 2 files")
        print("  out_file    name of output QDF file")
        print("  group_name  name of a group in <qdf_file>")
        print("  array_name  name of a numeric array in <group_name>")
        print("The qdf-files must be in temporal order.")
        print("Example:")
        print(" %s navworld_036_kya_256.qdf navworld_035_kya_256.qdf 1000 interp_036_kya_256.def Vegetation/BaseNPP Geography/Altitude"%argv[0]); 
    #-- end if
#-- end main
        
