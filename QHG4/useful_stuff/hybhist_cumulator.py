#!/usr/bin/python

from sys import argv, stderr
import os
import h5py
import numpy as np
from QHGError import QHGError

# symbolic name for the dimension indexes
SIM  = 0
STEP = 1
LOC  = 2

dim_names=["sim", "step", "loc", "hist"]

# location of hybridization in hdf data
HYB_POSITION = 3


class HybHistCumulator:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, hdf_file, num_bins, vmin, vmax, hyb_index):
        self.hdf_file = hdf_file
        if os.path.exists(hdf_file):
            try:
                self.hdf_data = h5py.File(hdf_file, "r")
            except Exception as e:
                raise QHGError("%s" % (e))
            #-- end try
        else:
            raise QHGError("file %s does not exist"%hdf_file)
        #-- end if

        self.hyb_index = hyb_index
        if num_bins > 0:
            self.num_bins = num_bins
        else:
            raise QHGError("numbins must be positive")
        #-- end if

        if (vmin < vmax):
            self.vmin     = vmin
            self.vmax     = vmax
        else:
            raise QHGError("vmin must be strictly less than vmax")
        #-- end if

        self.num_dims = 0
        
        # the hybridisation values read from the hdf file
        self.vals = {}

        # list of maps associating an index with a name
        self.name2Idx = []

        # list of list of names
        self.namelist = []

        # add along dimension i?
        self.abAdd   = []
        # fix dimension i?
        self.abFixed = []

        self.locvals    = []
        self.index_defs = []
        # now collect the values
        self.collect_vals()
    #-- end def


    #-----------------------------------------------------------------------------
    #-- make_histo
    #-- create a histogram with numbins bins from the values in vals
    #--
    def make_histo(self, vals):
        hist=None
        hist = [0]*self.num_bins
        for v in vals:
            bin = self.num_bins*(v - self.vmin)/(self.vmax - self.vmin)
            if (bin <= 0):
                bin=0
            elif (bin >= self.num_bins):
                bin = self.num_bins-1
            #-- end if
            bin=int(bin)
            hist[bin] = hist[bin]+1
        #-- end for
        return hist
    #-- end def


    #-----------------------------------------------------------------------------
    #-- collect_vals_rec
    #--  recursive call to collect data
    #--
    def collect_vals_rec(self, hdf_sub,  cur_vals, out_indexes, iLevel, max_level):

        iCur = 0
        if (len(self.name2Idx) == iLevel):
            self.name2Idx.append({})
            self.namelist.append([])
        #-- end if

        for nam in hdf_sub:
            cur_vals.append([])
            # add names and their indexes exactly *once*
            if  not nam in self.name2Idx[iLevel]:
                self.name2Idx[iLevel][nam] = iCur
                self.namelist[iLevel].append(nam)
            #-- end if
            out_indexes2 = out_indexes + "[%d]"%iCur

            #if nam in hdf_sub:
            if iLevel < max_level:
                self.collect_vals_rec(hdf_sub[nam], cur_vals[iCur], out_indexes2, iLevel+1, max_level)
            else:
            
                hvals_all =  np.array(hdf_sub[nam])

                if (len(hvals_all) > 0):
                    # hyb_index is the index of hybridization
                    hvals = [val[self.hyb_index] for val in hvals_all]
                    hist = self.make_histo(hvals)
                else:
                    hist = self.num_bins*[0]
                #-- end if
                                
                cur_vals[iCur] = hist

            #-- end if
            iCur = iCur + 1

        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- collect_vals
    #--
    def collect_vals(self):
        stderr.write("collecting values for [%f,%f]...\n"%(self.vmin, self.vmax))

        self.vals = []
        
        self.name2Idx = []
        self.namelist = []

        # we expect exactly 3 dimensions above the histos, so 2 is the maximimum dimension
        self.collect_vals_rec(self.hdf_data, self.vals, "", 0, 2)
        stderr.write("done!\n")

        self.num_dims = len(self.name2Idx)
        stderr.write("Have %d dimensions\n"%self.num_dims)

    #-- end def

 
    #-----------------------------------------------------------------------------
    #-- show_names
    #--
    def show_names(self, which=None):
        r =  range(len(self.namelist))
        if not which is None:
            r = range(which, which+1)
        #-- end if
        for i in r:
            print("%d (%s):"%(i,dim_names[i]))
            for s in self.namelist[i]:
                print("  %s"% s)
            #-- end for
        #-- end for
    #-- end def
    
        
    #-----------------------------------------------------------------------------
    #-- cumulate_histos
    #--
    def cumulate_histos(self, index_defs):
        iResult = -1
        self.index_defs = index_defs
        if (len(self.index_defs) == self.num_dims):
            self.abAdd   = self.num_dims*[False]
            self.abFixed = self.num_dims*[False]

            sType = ""
            for i in range(self.num_dims):
                sType = sType + "[%s]"%self.index_defs[i]
            #-- end for
            print("# index_def: "+sType)
            print("# binrange;  [%f,%f]"%(self.vmin, self.vmax))
            print("# binsize;   %f"%((self.vmax - self.vmin)/self.num_bins))
            
            ranges = self.determine_ranges(self.index_defs)

            self.locvals=np.array(self.vals)

            # first do the fixed dimensions
            s1 = list(self.locvals.shape)
            s0 = len(s1)*[0]
            for dim in range(self.num_dims):
                if self.abFixed[dim]:
                    #print("fixing %d to %s"%(dim, index_defs[dim]))
                    s0[dim] = int(self.name2Idx[dim][self.index_defs[dim]])
                    s1[dim] = s0[dim]+1
                #-- end if
            #-- end for

            # slicing for 'unknown' number of dimensions
            sArgs = ','.join(["s0[%d]:s1[%d]"%(i,i) for i in range(self.num_dims)])
            self.locvals = eval("self.locvals["+sArgs+"]")
            # instead of:
            #locvals = locvals[s0[0]:s1[0],s0[1]:s1[1],s0[2]:s1[2],s0[3]:s1[3],]
            #print("locvals shape after fixing: %s"%str(locvals.shape))

            # now do the summing where needed
            for dim in range(self.num_dims):
                s1 = list(self.locvals.shape)
                s0 = len(s1)*[0]
                if self.abAdd[dim]:

                    #print("adding along %d"%(dim))
                    s1a = s1.copy()

                    s1a[dim] = 1
                    sum = np.zeros(s1a)
                    for c in range(s1[dim]):
                        s0[dim] = c
                        s1[dim] = c+1
                        # again slicing for 'unknown' number of dimensions
                        sArgs = ','.join(["s0[%d]:s1[%d]"%(i,i) for i in range(self.num_dims)])
                        sum = eval("sum + self.locvals[" + sArgs +"]")
                        #sum = "sum + locvals[s0[0]:s1[0],s0[1]:s1[1],s0[2]:s1[2],s0[3]:s1[3]]
                    #-- end for
                    self.locvals = sum
                    #print("locvals shape after sum (%d): %s"%(dim, str(locvals.shape)))
                #-- end if
            #--end for
            #print("...finis... %s"%str(locvals.shape))

            stderr.write("locvals shape after cumulating: %s\n"%(str(self.locvals.shape)))
            iResult =  0
            #sOut = self.format_result(self.locvals, index_defs, True)
            
        else:
            print("bad index def:  [%s] (len(dims):%d"%(self.index_defs, self.num_dims)) 
            #sOut = ""
        #-- end if
        return iResult
    #-- end def


    #-----------------------------------------------------------------------------
    #-- determine_ranges
    #--  if the definition is an asterisk or a plus, the range is the full range
    #--  if the dfinition is a constant name, the range consists of that copnstant alone
    #--
    def determine_ranges(self, index_defs):
        stderr.write("determining ranges for %d dimensions\n"%self.num_dims)
        ranges       = [ [] for x in range(self.num_dims)]

        for dim in range(self.num_dims):
            if (index_defs[dim] == '*'):
                ranges[dim] = range(0,len(self.name2Idx[dim]))
            elif (index_defs[dim] == '+'):
                ranges[dim] = range(0,len(self.name2Idx[dim]))
                self.abAdd[dim] = True
            else:
                if (index_defs[dim] in self.name2Idx[dim]):
                    ranges[dim] = [self.name2Idx[dim][index_defs[dim]]]
                    self.abFixed[dim] = True
                else:
                    stderr.write("unknown name [%s]"%defs[dim])
                #-- end if
            #-- end if
        #-- end for
        
        return ranges
    #-- end def

        
    #-----------------------------------------------------------------------------
    #-- format_result_rec
    #--
    def format_result_rec(self, hall, defs, i, args, indent, bcsv, sOut, sPrefix):
        for iCur in range(hall.shape[i]):
            sNewField = ""
            if self.abAdd[i]:
                if bcsv:
                    sNewField = "[sum(%s)]"%dim_names[i]
                else:
                    print("%s[sum(%s)]: "%(indent, dim_names[i]))
                #-- end if
            else:
                if self.abFixed[i]:
                    if bcsv:
                       sNewField = defs[i] 
                    else:
                        print("%s[%s]: "%(indent, defs[i]))
                    #-- end if
                else:
                    if bcsv:
                        sNewField = self.namelist[i][iCur]
                    else:
                        print("%s[%s]: "%(indent, self.namelist[i][iCur]))
                    #-- end if
                #-- end if
            #-- end if
            
            args2 = args+"[%d]"%iCur

            if (i < len(defs)-1):
                sOut = self.format_result_rec(hall, defs, i+1, args2, indent+"  ", bcsv, sOut, sPrefix +";"+sNewField)
            else:
                if bcsv:
                    s = ""
                    sep=''
                    for k in range(len(eval("hall%s"%args2))):
                        s = s + sep + str(eval("int(hall%s[k])"%args2))
                        sep=';'
                    #-- end for
                    # print("%s"%s)
                    sOut = sOut + sPrefix + ";" + sNewField + ";" + s + "\n"
                    #sOut = sOut + self.hdf_file + ";" + s + "\n"

                else:
                    print("  %s%s"%(indent,eval("hall%s"%args2)))
                #-- end if
            #-- end if

        #-- end for
        return sOut
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- format_result
    #--
    def format_result(self, hall, defs, bcsv):
        sHeader="# file_name;" + ";".join(dim_names)
        sOut = sHeader+"\n"
        sOut = self.format_result_rec(hall, defs, 0, "", "", bcsv, sOut, self.hdf_file)
        return sOut
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_output
    #--
    def get_output(self, bcsv):
        return self.format_result(self.locvals, self.index_defs, bcsv)
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- get_dimensions
    #--  *** not used currently
    def get_dimensions(self, hall):
        # get dimensionality of result array
        bOK = True
        k=hall
        dims = []
        iDim = 0
        while bOK:
            print("dim %s(%d): %d"%(dim_names[iDim], iDim, len(k)))
            dims.append(len(k))
            iDim = iDim + 1
            k = k[0]
            bOK =  isinstance(k, list)
        #-- end while
        print("hall:%s"%hall)
        return dims
    #-- end if



    #-----------------------------------------------------------------------------
    #-- show_structure
    #--   *** not used currently
    def show_structure(self):
        
        print("have %d simulations"%len(self.vals))
        for iSim in range(len(self.vals)):
            print("sim '%s'(%d): %d steps"%(self.namelist[SIM][iSim], iSim, len(self.vals[iSim])))
            for iStep in range(len(self.vals[iSim])):
                print("  step '%s'(%d): %d locs"%(self.namelist[STEP][iStep], iStep, len(self.vals[iSim][iStep])))
                for iLoc in range(len(self.vals[iSim][iStep])):
                    print("    loc '%s'(%d): %d hists"%(self.namelist[LOC][iLoc], iLoc, len(self.vals[iSim][iStep][iLoc])))
                #-- end for
            #--end for
        #-- end for
    #-- end def
    

    #-----------------------------------------------------------------------------
    #-- check_index_def
    #--
    def check_index_def(self, sDef):
        defs = None
        bOK = False
        a = sDef.split(':')
        if (len(a) == 3):
            i = 0
            bOK = True
            while bOK and (i < 3):
                
                if (a[i] != '*') and (a[i] != '+') and not a[i] in self.namelist[i]:
                    stderr.write("[%s] is not a valid name for dimension %d, n"%(a[i], i))
                    stderr.write("%s\n"%self.namelist[i])
                    bOK = False
                else:
                    i = i+1
                #-- end if
            #-- end while
        else:
            print("Expected '<sim_name>;<step_name>:<loc_name>' but got [%s]"%sDef)
        #-- end if
        if (bOK) :
            defs = a
        #-- end if
       
        return defs
    #-- end def
        
#-- end class


#-----------------------------------------------------------------------------
#-- interactive_loop
#--
def interactive_loop(hhc):
    CMD_HELP = "help"
    CMD_QUIT = "quit"
    CMD_NAME = "names"
    #TODO
    #CMD_RANGE = "range"
    #CMD_BINS  = "bins"
    
    bGoOn = True
    print("interactive mode")
    print("type 'q' to exit, 'h' for help")
    
    while bGoOn:
        s=input("Enter index definition: ")
        if CMD_QUIT.startswith(s):
            bGoOn = False
            
        elif CMD_HELP.startswith(s):
            print("possible inputs:")
            print("  'q'  or 'quit'    exit loop")
            print("  'h'  or 'help'    display this help")
            print("  'n'  or 'names'   show simulation, step and location names")
            print("  <index_def>       an index definition for histogram cumulation")
            print("where")
            print("  index_def ::= <sim_def>\":\"<step_def>\":\"<loc_def>")
            print("  sim_def   ::= <sim_name>  | \"*\" | \"+\"")
            print("  step_def  ::= <step_name> | \"*\" | \"+\"")
            print("  loc_def   ::= <loc_name>  | \"*\" | \"+\"")
            print("  sim_name  :   name of a simulation")
            print("  step_name :   name of a step")
            print("  loc_name  :   name of a location")
            print("  '*'       : show all entries along this dimension")
            print("  '+'       : add entries along this dimension")
            
        elif CMD_NAME.startswith(s):
            which = None
            a = s.split()
            if (len(a) == 2) and (a[1].isnumeric()):
                which = int(a[1])
                #-- end if
                hhc.show_names(which)
                
            elif len(s) > 0:
                indexdefs = hhc.check_index_def(s)
                if not indexdefs is None:
                    hhc.cumulate_histos(indexdefs)
                #-- end if
            #-- end if
        #-- end if
    #-- end wile
#-- end def 


#-----------------------------------------------------------------------------
#-- process_args
#--
def process_args(argv, argvals):
    cur = 1
    while (cur < len(argv)) :
        if argv[cur] == '-i':
            argvals["interact"] = True
            cur = cur+1
        elif argv[cur] == '-l':
            argvals["list"] = True
            cur = cur+1
        else:
            if (cur+1) < len(argv):
                if argv[cur] == '-f':
                    argvals["hdf_file"]   = argv[cur+1]
                # maybe check for existence
            
                elif argv[cur] == '-n':
                    if argv[cur+1].isdigit():
                        argvals["num_bins"] = int(argv[cur+1])
                    else:
                        raise QHGError("expected integer for num bins, not [%s]"%argv[cur+1])
                    #-- end if
            
                elif argv[cur] == '-d':
                    # format of index_defs will bechecked by HybHistCumulator 
                    argvals["index_defs"] = argv[cur+1]
            
                elif argv[cur] == '-r':
                    p = argv[cur+1].split(':')
                    if (len(p) == 2):
                        if p[0].replace('.','',1).isdigit() and p[1].replace('.','',1).isdigit():
                            argvals["v_min"] = float(p[0])
                            argvals["v_max"] = float(p[1])
                        else:
                            raise QHGError("expected floats for range, not [%s]"%argv[cur+1])
                        #-- end if
                    else:
                        raise QHGError("expected <vmin>:<vmax> instead of [%s]"%argv[cur+1])
                    #-- end if

                elif argv[cur] == '-o':
                    if (argv[cur+1] == "csv") or  (argv[cur+1] == "nice"):
                        argvals["out_mode"] = argv[cur+1]
                    else:
                        raise QHGError("outmode can only be 'csv' or 'nice', not [%s]"%argv[cur+1])
                    #-- end if

                elif argv[cur] == '-p':
                    if argv[cur+1].isdigit():
                        argvals["hyb_pos"] = int(argv[cur+1])
                    else:
                        raise QHGError("expected integer for hyb position, not [%s]"%argv[cur+1])
                    #-- end if
                else:
                    raise QHGError("unknown option [%s]"%argv[cur])
                #-- end if
                    
                cur = cur + 2
            else:
                raise QHGError("missing argument after [%s]"%argv[cur])
            #-- end if
        #-- end if
    #-- end while
    return argvals
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    
    argvals = {}
    argvals["hdf_file"]   = ""
    argvals["num_bins"]   = 0
    argvals["index_defs"] = ""
    argvals["v_min"]      = 0
    argvals["v_max"]      = 1
    argvals["out_mode"]   = "csv"
    argvals["interact"]   = False
    argvals["list"]       = False
    argvals["hyb_pos"]    = HYB_POSITION

    try:
        bOK = True
        argvals = process_args(argv, argvals)

        hdf_file   = argvals["hdf_file"]
        num_bins   = argvals["num_bins"]
        index_defs = argvals["index_defs"]
        v_min      = argvals["v_min"]
        v_max      = argvals["v_max"]
        hyb_index  = argvals["hyb_pos"]

        bcsv = (argvals["out_mode"] == 'csv')
        
        
        hhc = HybHistCumulator(hdf_file, num_bins, v_min, v_max, hyb_index)
        #hhc.show_names()
        #print()

        if argvals["interact"]:
            interactive_loop(hhc)
        elif argvals["list"]:
            hhc.show_names()
        else:
            indexdefs = hhc.check_index_def(index_defs)
            if  not indexdefs is None:
                iResult = hhc.cumulate_histos(indexdefs)
                if (iResult == 0):
                    sOut = hhc.get_output(bcsv)
                    print(sOut[0:-1])
                else:
                    print("there was some error")
                #-- end if
            #-- end if
        #-- end if
    except QHGError as e:
        print("QHGError: %s"%e)
        bOK = False
    #-- end try
       
    if not bOK:
        print("usage:")
        print("  %s -f <hdf_file> -n <num_bins> -d <indexdefs> [-r <vmin>:<vmax>] [-p <hyb_index>] [-o <outmode>]"%argv[0])
        print("  (to directly evaluate the index definition (outmode can be 'csv' or 'nice')")
        print("or")
        print("  %s -i -f <hdf_file> -n <num_bins>  [-r <vmin>:<vmax>]  [-p <hyb_index>] [-o <outmode>]"%argv[0])
        print("  (to enter interactive mode)")
        print("or")
        print("  %s -l -f <hdf_file> "%argv[0])
        print("  (to list dimension names)")
        print("where")
        print("  hdf_file     hdf file to  analyze")
        print("  num_bins     number of (equally sized) bins for histogram")
        print("  index_defs   index definitions for reductions (see below)")
        print("  vmin         mininmal value (start of lowest bin)")
        print("  vmax         maxinmal value (end of highest bin)")
        print("  outmode      output mode ('csv' or 'nice')")

        print("  index_def ::= <sim_def>\":\"<step_def>\":\"<loc_def>")
        print("  sim_def   ::= <sim_name>  | \"*\" | \"+\"")
        print("  step_def  ::= <step_name> | \"*\" | \"+\"")
        print("  loc_def   ::= <loc_name>  | \"*\" | \"+\"")
        print("  sim_name  :   name of a simulation")
        print("  step_name :   name of a step")
        print("  loc_name  :   name of a location")
        print("  '*'       : show all entries along this dimension")
        print("  '+'       : add entries along this dimension")
#-- end if
    #print("TODO:")
    #print(" -- index defs as input params")
    #print(" -- cumulations seem to work correctly now (problem was that in python  list assignments are actually reference assignments)")
#-- end main

