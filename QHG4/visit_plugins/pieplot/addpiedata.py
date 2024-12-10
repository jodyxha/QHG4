#!/usr/bin/python


# expect file with format
#  file      ::= <dsname><cs><numpies><cr><numvals><cr><dataline>*
#  dsname    ::= "DATASET="<name_of_dataset>
#  numpies   ::= "NUM_PIES="<number_of_pies>
#  numvals   ::= "NUM_VALS="<number of_values>
#  valnames  ::= "VAL_NAMES="<name_list>
#  name_list ::= <name:[" "<name>}*
#  dataline  ::= <pos><norm><value>*
#  pos       ::= <pos_x>  <pos_y>  <pos_z>
#  norm      ::= <norm_x> <norm_y> <norm_z>

import sys
from sys import argv
import numpy as np
import h5py

sys.path.append('/home/jody/progs/QHG4/useful_stuff')
from QHGError import QHGError
ROOT_ATTR_QHG_NAME  = "QHG"
ROOT_ATTR_QHG_VAL   = "QHG data file for pies"
ROOT_ATTR_TIME_NAME = "Time"
ROOT_ATTR_TIME_VAL  = 0


PIEGROUP_NAME      = "PiePlots"
PIE_ATTR_NUM_PIES  = "NumPies"
PIE_ATTR_NUM_VALS  = "NumVals"
PIE_DATASET_NAME   = "PieDataSet"
PIE_ATTR_VAL_NAMES = "ValNames"

class PieWriter:
    #--------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, hfile, data_file, bRemovePieGroup):
        self.hFile = hFile
        self.data_file = data_file
        if (PIEGROUP_NAME in self.hFile):
            if bRemovePieGroup:
                del hFile[PIEGROUP_NAME]
                self.hPieGroup = self.hFile.create_group(PIEGROUP_NAME)
            else:
                self.hPieGroup = self.hFile[PIEGROUP_NAME]
            #-- end if
        else:
            self.hPieGroup = self.hFile.create_group(PIEGROUP_NAME)
        #-- end if
        self.num_pies  = 0
        self.num_vals  = 0
        self.val_names = ""
        self.ds_name   = ""
        self.output    = []
        
    #-- end def


    #--------------------------------------------------------------------------
    #-- remove_unneeded
    #--
    def remove_unneeded(self, data_lines):
        out = []
        for x in data_lines:
            x = x.strip()
            if (len(x) > 0) and (x[0] != "#"):
                out.append(x)
            #-- end if
        #-- end for
        return out
    #-- end def
   

    #--------------------------------------------------------------------------
    #-- read_data
    #--
    def read_data(self, data_file):
        
        sErr = ""
        f = open(data_file)
        lines = f.read().split("\n")
        lines = self.remove_unneeded(lines)
        i = 0
        iTotalPies = 0

        # check dataset-line
        if lines[i].startswith("DATASET="):
            ds_name = lines[i].split("=")[1]
            if len(ds_name) > 0:
                self.ds_name = ds_name
                i = i+1
            else:
                sErr = "expected data set name after '=' in [%s]"%lines[i]
           #-- end if
        else:
            sErr = "expected entry 'DATASET=<dataset_name>'"
        #-- end if
       
        # check numpies-line
        if not sErr:
            if lines[i].startswith("NUM_PIES="):
                num_pies = lines[i].split("=")[1]
                if num_pies.isnumeric():
                    self.num_pies = int(num_pies)
                    i = i+1
                else:
                    sErr = "expected number of pies '=' in [%s]"%lines[i]
                #-- end if
            else:
                sErr = "expected entry 'NUM_PIES=<number_of_pies>' but got[%s]"%lines[i]
            #-- end if
        #-- end if
       
        # check numvals-line
        if not sErr:
            if lines[i].startswith("NUM_VALS="):
                num_vals = lines[i].split("=")[1]
                if num_vals.isnumeric():
                    self.num_vals = int(num_vals)
                    i = i+1
                else:
                    sErr = "expected number of values '=' in [%s]"%lines[i]
                #-- end if
            else:
                sErr = "expected entry 'NUM_VALS=<number_of_values>'"
            #-- end if
        #-- end if

        # check valnames-line
        if not sErr:
            if lines[i].startswith("VAL_NAMES="):
                self.val_names = lines[i].split("=")[1].split()
                if len(self.val_names) == self.num_vals:
                    i = i+1
                else:
                    sErr = "number of value names (%d) not equal to number of values (%d)"%(len(self.val_names),self.num_vals)
                #-- end if
            else:
                sErr = "expected entry 'VAL_NAMES=<value_name_list>'"
            #-- end if
        #-- end if


        # read the data
        print("reading data")
        while (i < len(lines)) and not sErr:
            data = lines[i].split()
            if (len(data) == 6 + self.num_vals):
                for x in data:
                    try:
                        f = float(x)
                        self.output.append(f)
                    except Exception as e:
                        sErr = "ecpected number, not %s"%x
                        break
                    #-- end try
                #-- end for
                if not sErr:
                    iTotalPies = iTotalPies + 1
                    i = i+1
                #-- end if
            else:
                sErr = "exepcted 6 + %d = %d numeric elements, not %d"%(self.num_vals, 6+self.num_vals, len(data))
            #-- end if
        #-- end while

        if iTotalPies != self.num_pies:
            sErr = "number of data lines (%d) is not equal to number of pies (%d)"%(iTotalPies, self.num_pies)
        #-- end if
        
        if not sErr:
            print("collected data for %d pies"%iTotalPies)
        else:
            raise QHGError(sErr)
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- write_output
    #--
    def write_output(self):
        if not self.ds_name in self.hPieGroup:
            arr = np.array(self.output)
           
            hSubGroup = self.hPieGroup.create_group(self.ds_name)

            hSubGroup.attrs.create(PIE_ATTR_NUM_PIES, self.num_pies, None, np.int_);
            hSubGroup.attrs.create(PIE_ATTR_NUM_VALS, self.num_vals, None, np.int_);
            sTemp =  ';'.join(self.val_names)
            ttype = h5py.string_dtype('utf-8', len(sTemp)+1 )
            hSubGroup.attrs[PIE_ATTR_VAL_NAMES] = np.array(sTemp.encode("utf-8"), dtype=ttype)

            dset = hSubGroup.create_dataset(PIE_DATASET_NAME, (len(arr),), 'f')
            dset[:] = arr
        else:
            raise QHGError("The subgroupdata set [%s] already exists in the pie group [%s]"%(self.ds_name, PIEGROUP_NAME))
       #-- end if
    #-- end def
   
#-- end class

def creatEmptyQDF(file_name):
    hFile = h5py.File(qdf_file, "w")
    hFile.attrs[ROOT_ATTR_QHG_NAME]  = ROOT_ATTR_QHG_VAL
    hFile.attrs[ROOT_ATTR_TIME_NAME] = ROOT_ATTR_TIME_VAL
    hFile.flush()
    return hFile
#-- end def

    
if len(argv) > 3:
    if (argv[1] == "-r"):
        bRemovePieGroup = True
    else:
        bRemovePieGroup = False
    #-- end if
    qdf_file  = argv[2]
    data_file = argv[3]

    try: 
        hFile = h5py.File(qdf_file, "r+")
    except FileNotFoundError:
        print("File [%s] does not exist"%qdf_file)
        hFile = creatEmptyQDF(qdf_file)
    except Exception as e:
        print("exception: [%s]"%e)
        hFile = None
    #-- end try
    if not hFile is None:
        try:
            print("starting pie writer")
            pw = PieWriter(hFile, data_file, bRemovePieGroup)
            print("read data")
            pw.read_data(data_file)
            print("write output")
            pw.write_output()
            hFile.flush()
            hFile.close()
            print("+++ success +++")
        except QHGError as q:
            print("QHGError: %s"%(q))
        except Exception as e:
            print("Error: %s"%(e))
        #-- end try
    #-- end if
else:
    print("%s ['-r'|'-a'} <qdf-file> <data-file>"%argv[0])
    print("where")
    print("  qdf-file   qdf file to be modified (if <qdf-file> doesn't exist, it will be created)")
    print("  data-file  text file containng the data for the pie (format: see below)")
    print("  -r         replace the qdf file's 'PiePlots' group with the data")
    print("  -a         append new data to exisiting 'PiePlots' group")
    print("data file format:")
    print("  data-file       ::= <header> <data-line>*")
    print("  header          ::= <dataset-line><numpies-line><numvals-line><valnames-line>")
    print("  dataset-line    ::= \"DATASET=\"<dataset-name>")
    print("  numpies-line    ::= \"NUM_PIES=\"<umber-of-pies>")
    print("  numvals-line    ::= \"NUM_VALS=\"<number-of-values>")
    print("  valnames_line   ::= \"VAL_NAMES=\"<names-string>")
    print("  dataset-name     : name of data set (visible in VisIt)")
    print("  number-of-pies   : number of pies to be plotted (should be equal to the number of data lines)")
    print("  number-of-values : number of values to be displayed by each pie")
    print("  names-string     : string consisting of value names concatenated by ';'")
    print("  data-line        : <position><normal><value>*")
    print("  position         : <pos_x><pos_y><pos_z> (position of pie)")
    print("  normal           : <norm_x><norm_y><norm_z> (normal of pie)")
    print("  value            : data value (there should be <number-of-values> items)")
    
    
    
#-- end main
