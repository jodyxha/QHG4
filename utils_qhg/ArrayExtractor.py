#!/usr/bin/python
from sys import argv, stderr
import os
import shutil
import h5py
import numpy as np


#-----------------------------------------------------------------------------
#-- ArrayExtractor
#--
class ArrayExtractor:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    #--  arr_desc: list of strings "group_name/array_name"
    #-- 
    def __init__(self, qdf_file):
        self.qdf_file = qdf_file
        
        self.arrays = {}
        self.num_cells = 0
    #-- end def


    #-----------------------------------------------------------------------------
    #-- check_descriptions
    #--
    def check_descriptions(self):
        bOK = True
        for desc in self.arr_descs:
            if desc.count("/") != 1:
                bOK = False
                stderr.write("array description need exactly 1 slash (%s)\n" % (desc))
            #-- end if
        #-- end for
        return bOK
    #-- end def
    
        
    #-----------------------------------------------------------------------------
    #-- extract_arrays
    #--
    def extract_arrays(self, arr_descs):
        bSuccess=False
        self.arr_descs    = arr_descs

        if (self.check_descriptions()):
            try:
                qdf = h5py.File(self.qdf_file,'r+')
            except:
                stderr.write("Couldn't open [%s] as QDF file\n" % (self.qdf_file))
            #-- end try

            try:
                for arr in self.arr_descs:
                    temp_arr = self.extract_array(qdf, arr)
                    self.arrays[arr] = temp_arr
                    print("%s: %d (%s)" % (arr, len(temp_arr), temp_arr.dtype))
                #-- end for
            except Exception as e:
                stderr.write("error obtaining array [%s]\n" % (arr))
                stderr.write("Exception: %s\n" % (e))
                        
            else:
                stderr.write("got %d arrays\n" % (len(self.arrays)))
                if (len(self.arrays) > 0):
                    # checking length
                    if (self.check_lengths()):
                        print("+++ success +++")
                        bSuccess = True
                    else:
                        stderr.write("Not all arrays have same length\n")
                        for n in self.lengths:
                            stderr.write("%-25s %d\n" % (n, self.lengths[n]))
                        #-- end for

                    #-- end if
                else:
                    stderr.write("Writing no output\n")
                #-- end if
            #-- end try
            qdf.close()
        #-- end if
        return bSuccess
    #--end def


    #-----------------------------------------------------------------------------
    #-- check_lengths
    #--
    def check_lengths(self):
        self.num_cells = -1
        self.lengths = {}
        bEqualSize = True

        for arr in self.arrays:
            l = len(self.arrays[arr])
            self.lengths[arr] = l
            
            #for i in range(len(self.arrays)):
            #l = len(self.arrays[i])
            #self.lengths[self.arr_desc[i]] = l
            if (self.num_cells < 0):
                self.num_cells = l
            elif (self.num_cells != l):
                bEqualSize = False
            #-- end if
        #-- end  for
        return bEqualSize
    #-- end def

        
    #-----------------------------------------------------------------------------
    #-- extract_array
    #--
    def extract_array(self, qdf, group_arr):
        arr = None
        (group_name,array_name) = group_arr.split("/")
           
        group = qdf["/"+group_name]
        if group.__contains__(array_name):
            arr = np.array(group[array_name])
        else:
            raise Exception("could not find array [%s] in group [%s]" % (group_name, array_name))
        #-- end if
        return arr
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_array
    #--
    def get_array(self, arr_desc):
        cur_arr = []
        if arr_desc in self.arr_descs:
            cur_arr = self.arrays[arr_desc]
        #-- end if
        return cur_arr
    #-- end if

    #-----------------------------------------------------------------------------
    #-- write_arrays
    #--
    def write_arrays(self, out_file):
        stderr.write("write_arrays(%s)" % out_file)
        if True: #try:
            fOut = open(out_file, "w")
            sHeader = " ".join(self.arr_descs)
            fOut.write("%s\n" % (sHeader))

            for i in range(self.num_cells):
                sLine = ""
                for arr in self.arrays:
                    v = self.arrays[arr]
                    if (sLine != ""):
                        sLine = sLine + " "
                    #-- end if
                    sLine = sLine + "%f"%(v[i])
                #-- end for
                fOut.write("%s\n" % (sLine))
            #-- end for
            fOut.close()
            print("Written %d arrays with %d elemenrs each to [%s]" % (len(self.arrays), self.num_cells, out_file))
        else: # except Exception as e:
            stderr.write("Error writing output file [%s]" % out_file)
            stderr.write("Exception: %s" % (e))
        #-- end try
    #-- end def
#-- end class


if __name__ == '__main__':
    if len(argv) > 3:
        qdf_file = argv[1]
        out_file = argv[2]
        array_desc = argv[3:]
        
        stderr.write("qdf_file: [%s]\n" % (qdf_file))
        stderr.write("out_file: [%s]\n" % (out_file))
        stderr.write("array descs:\n")
        for s in array_desc:
            stderr.write("    [%s]\n" % s)
        #-- end if
        ae = ArrayExtractor(qdf_file)
        bOK = ae.extract_arrays(array_desc)
        if (bOK):
            x=ae.get_array(argv[3])
            
            
            ae.write_arrays(out_file)
        #-- end if
    else:
        print("%s - Extract arrays from a QDF file and write as text file" % (argv[0]))
        print("Usage:")
        print("  %s <qdf_file> <out_file> [<group_name>/<array_name>]*" % (argv[0]))
        print("where");
        print("  qdf_file    name of QDF environment file")
        print("  out_file    name of output file")
        print("  group_name  name of a group in <qdf_file>")
        print("  array_name  name of a numeric array in <group_name>")
    #-- end if
#-- end main

