#!/usr/bin/python

from sys import argv, stderr
import h5py
import numpy as np
import random

#-----------------------------------------------------------------------------
#-- median
#-- find median value
#--
def median(vals):
    if len(vals) > 2:
        sv = sorted(vals)
        return (sv[int(len(vals)/2)]+sv[int(1+len(vals)/2)])/2
    elif len(vals) > 1:
        return (vals[0]+vals[1])/2
    else:
        return vals[0]
    #-- end if
#-- end def


#-----------------------------------------------------------------------------
#-- make_histo
#-- create a histogram with numbins bins from the values in vals
#--
def make_histo(numbins, vals):
    hist=None
    vmin = min(vals)
    vmax = max(vals)
    stderr.write("%d values in [%d,%d]\n"%(len(vals),vmin, vmax))
    if (vmin <= vmax):
        vmin = 0
        vmax = 1
        hist = [0]*numbins
        for v in vals:
            bin = numbins*(v - vmin)/(vmax - vmin)
            if (bin <= 0):
                bin=0
            elif (bin >= numbins):
                bin = numbins-1
            #-- end if
            bin=int(bin)
            hist[bin] = hist[bin]+1
        #-- end for
    #-- end if
    return hist
#-- end def


#-----------------------------------------------------------------------------
#-- make_gauss_array
#-- (i don't remember what that's for)
#--
def make_gauss_arr(numvals, mu, sigma):
    gauss_arr=[]
    for i in range(numvals):
        gauss_arr.append(random.gauss(mu,sigma))
    #-- end for
    return gauss_arr
#-- end def


#-----------------------------------------------------------------------------
#-- extract_hyb
#--   extract the hybridisation values using the provided path
#--   (we assume h is a hdf file)
#--
def extract_hyb(h, path):
    hvals = []
    if (path in h):
        vals = np.array(h[path])
        if (vals.size > 0):
            for val in vals:
                hvals.append(val[2])
            #-- end for
        else:
            stderr.write("Path [%s] has no values\n" %(path))
        #-- end if
    else:
        stderr.write("Path [%s] does not exist\n" %(path))
    #-- end if
    return hvals
#-- end def
        

#-----------------------------------------------------------------------------
#-- extract_hist_from_path
#--
def extract_hist_from_path(h, path,  numbins):
    hist = None
    hvals = extract_hyb(h, path)
    if (len(hvals) > 0):
        hist = make_histo(numbins, hvals)
        #-- end if
    else:
        stderr.write("%s has %d values\n"%(path, len(hvals)))

        #print("Path [%s] does not exist on %s" %(path, hdf_file))
    #-- end if
    return hist
#-- end def


#-----------------------------------------------------------------------------
#-- extract_hist_from_values
#--
def extract_hist_from_values(hvals,  numbins):
    hist = None
    if (len(hvals) > 0):
        hist = make_histo(numbins, hvals)
        #-- end if
    else:
        stderr.write("%s has %d values\n"%(path, len(hvals)))

        #print("Path [%s] does not exist on %s" %(path, hdf_file))
    #-- end if
    return hist
#-- end def


#-----------------------------------------------------------------------------
#-- show_counts_file
#--  get histogram for all values in the file
#--
def show_counts_file(h, numbins):
    for sim in h.keys():
        show_counts_sim(h, sim, numbins);
    #-- end for
#-- end def


#-----------------------------------------------------------------------------
#-- show_counts_file
#--  get histogram for specified sim
#--
def show_counts_sim(h, sim, numbins):
    for step in h[sim].keys():
        show_counts_step(h, sim, step, numbins)
    #-- end for
#-- end def
                                                  

#-----------------------------------------------------------------------------
#-- show_counts_file
#--  get histogram for specified sim and step
#--
def show_counts_step(h, sim, step, numbins):
    for loc in h[sim][step].keys():
        show_counts_loc(h, sim, step, loc, numbins)
    #-- end for
#-- end def
                                                  

#-----------------------------------------------------------------------------
#-- show_counts_loc
#--  get histogram for specified sim, step, and loc
#--
def show_counts_loc(h, sim, step, loc, numbins):
    path = "%s/%s/%s"%(sim,step,loc)
    stderr.write("Extracting hist from [%s]\n"%path)
    hvals = extract_hyb(h,path)
    if len(hvals) > 0:
        hist = extract_hist_from_values(hvals, numbins)
    #-- end if
    if (len(hvals) > 0) and (not hist is None):
        sh=""
        for x in hist:
            sh = sh + ";%d"%x
        #-- end for
        print("%s;%s;%s;%d;%f;%f;%f%s"%(sim, step, loc, len(hvals), min(hvals), max(hvals), median(hvals), sh))
    else:
        sh = numbins*";-"
        print("%s;%s;%s;%d;-;-;-%s"%(sim, step, loc, len(hvals), sh))
    #-- end for
#-- end def

                                                   
sim      = ""
step     = ""
loc      = ""
#num_bin  = -1

if len(argv) > 1:
    if argv[1] == "-c":
        if len(argv) > 2:
            hdf_file = argv[2]
            h = h5py.File(hdf_file, "r")
            num_bin = 21
            if len(argv) > 3:
                num_bin = int(argv[3])
            #-- end if
            show_counts_file(h, num_bin)
            h.close()
        #-- end if
    else:
        hdf_file = argv[1]
        h = h5py.File(hdf_file, "r")
        stderr.write("arg 1:[%s]\n"%argv[1])
        if len(argv) > 2:
            stderr.write("arg 2:[%s]\n"%argv[2])
            if argv[2].isnumeric():
                show_counts_file(h, int(argv[2]));
            else:
                sim = argv[2]
                if len(argv) > 3:
                    stderr.write("arg 3:[%s]\n"%argv[3])
                    if argv[3].isnumeric():
                        show_counts_sim(h, sim, int(argv[3]));
                    else:
                        step = argv[3]
                        if len(argv) > 4:
                            stderr.write("arg 4:[%s]\n"%argv[4])
                            if argv[4].isnumeric():
                                show_counts_step(h, sim, step, int(argv[4]));
                            else:
                                loc = argv[4]
                                if len(argv) > 5:
                                    stderr.write("arg 5:[%s]\n"%argv[5])
                                    if argv[5].isnumeric():
                                        show_counts_loc(h, sim, step, int(argv[5]));
                                    else:
                                        stderr.write("expected numbins but got [%s]\n"%(argv[5]))
                                    #-- end if
                                else:
                                    stderr.write("locs in %s[%s/%s]:\n %s\n"%(hdf_file,sim,step, list(h[sim][step].keys())))
                                #-- end if
                            #-- end if    
                        else:
                            stderr.write("locs in %s[%s/%s]:\n %s \n"%(hdf_file,sim,step, list(h[sim][step].keys())))
                        #-- end if
                    #-- end if
                else:
                    stderr.write("steps in %s[%s]:\n %s\n"%(hdf_file,sim, list(h[sim].keys())))
                #-- end if
            #-- end if
        else:
            stderr.write("simulations in %s:\n %s\n"%(hdf_file, list(h.keys())))
        #-- end if
        h.close()
    #-- end if
else:
    stderr.write("%s <hdf_file> [<simulation> [<step> [<loc>]]] [numbins]\n"%(argv[0].split('/')[-1]))
    stderr.write("or\n")
    stderr.write("%s -c <hdf_file> <numbins>\n"%(argv[0].split('/')[-1]));
#-- end if 
