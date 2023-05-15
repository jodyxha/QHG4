#!/usr/bin/python

from sys import argv, stderr
import h5py
import numpy as np


#-----------------------------------------------------------------------------
#-- extract_ymt
#--   extract the hybridisationagent data values using the provided path
#--   element 0: cell id
#--   element 1: agent id
#--   element 2: gender (0: female, 1: male)
#--   element 3: hybridization
#--   element 4: Ychr   (0: sap, 1: nea)
#--   element 5: mtDNA  (0: sap, 1: nea)
#--
def extract_ymt(h, path):
    hvals = []

    
    ycounts = [0, 0];
    mtcounts = [0, 0];
    hyb_sum = 0;
    if (path in h):
        vals = np.array(h[path])
#        stderr.write("Path [%s] has %d values\n" %(path, len(vals)))
        if (vals.size > 0):
            for val in vals:
                        
                if val[2] == 1:
                    ycounts[val[4]] =  1 + ycounts[val[4]]
                else:
                    mtcounts[val[5]] = 1 + mtcounts[val[5]]
                #-- end if

                hyb_sum = hyb_sum + val[3]
            #-- end for
        else:
            pass
#            stderr.write("Path [%s] has no values\n" %(path))
        #-- end if
    else:
        stderr.write("Path [%s] does not exist\n" %(path))
    #-- end if
    return [mtcounts, ycounts, hyb_sum, vals.size]
#-- end def


#-----------------------------------------------------------------------------
#-- show_counts_loc
#--  get histogram for specified sim and step
#--
def show_counts_loc(h, sim, step, loc, mode, level, bHeader):
    ycounts  = [0, 0]
    mtcounts = [0, 0]

    path = "%s/%s/%s"%(sim,step,loc)
    r = extract_ymt(h, path)

    mtcounts = r[0]
    ycounts  = r[1]
    if r[3] > 0:
        avg_hyb = r[2]/r[3]
    else:
        avg_hyb = -1
    #-- end if
    
#    ycounts[0]  = ycounts[0]  + r[1][0]
#    ycounts[1]  = ycounts[1]  + r[1][1]
#    mtcounts[0] = mtcounts[0] + r[0][0]
#    mtcounts[1] = mtcounts[1] + r[0][1]

    if (bHeader):
        print("lo sim;step;loc;mtcounts_S;mtcounts_N;ycounts_S;ycounts_N;avg_hyb")
        
    if mode == 'det':
        
        print("%s;%s;%s;%d;%d;%d;%d;%f"%(sim, step, loc, mtcounts[0], mtcounts[1], ycounts[0], ycounts[1], avg_hyb))

                
    #-- end if
    return [mtcounts,ycounts, r[2], r[3]]
#-- end def
                                    

        
#-----------------------------------------------------------------------------
#-- show_counts_step
#--  get histogram for specified sim and step
#--
def show_counts_step(h, sim, step, mode, level, bHeader):
    ycounts  = [0, 0]
    mtcounts = [0, 0]
    hyb_sum   = 0
    hyb_count = 0
    
    if (bHeader):
        if mode == mode: #'acc':
            sHeader = "mtcounts_S;mtcounts_N;ycounts_S;ycounts_N;avg_hyb"
            if level == 'file':
                sHeader = sHeader;
            elif level == 'sim':
                sHeader = "step;" + sHeader;
            elif level == 'step':
                sHeader = "sim;step;" + sHeader;
            elif level == 'loc':
                sHeader = "sim;step;loc;" + sHeader
            #-- end if
            print(sHeader)
        
        bHeader = False
        
    if (level == 'step') and (mode == 'det'):
        mode = 'acc'
        
        
        
    for loc in h[sim][step].keys():
        r = show_counts_loc(h, sim, step, loc, mode, level, bHeader)
        ycounts[0]  = ycounts[0]  + r[1][0]
        ycounts[1]  = ycounts[1]  + r[1][1]
        mtcounts[0] = mtcounts[0] + r[0][0]
        mtcounts[1] = mtcounts[1] + r[0][1]
        hyb_sum     = hyb_sum   + r[2]
        hyb_count   = hyb_count + r[3]
    #-- end for
    
    if level == 'step':
        if hyb_count > 0:
            avg_hyb = hyb_sum/hyb_count
        else:
            avg_hyb = -1
        #-- end if
        print("%s;%s;%d;%d;%d;%d;%f"%(sim, step, mtcounts[0], mtcounts[1], ycounts[0], ycounts[1], avg_hyb))
    #-- end if
    
    return [mtcounts, ycounts, hyb_sum, hyb_count]
            
#-- end def
                                    
#-----------------------------------------------------------------------------
#-- show_counts_sim
#--  get histogram for specified sim and step
#--
def show_counts_sim(h, sim, mode, level, bHeader):
    ycounts   = [0, 0]
    mtcounts  = [0, 0]
    hyb_sum   = 0
    hyb_count = 0
        
    if (bHeader):
        if mode == mode:#'acc':
            sHeader = "mtcounts_S;mtcounts_N;ycounts_S;ycounts_N;avg_hyb"
            if level == 'file':
                sHeader = sHeader;
            elif level == 'sim':
                sHeader = "sim;" + sHeader;
            elif level == 'step':
                sHeader = "sim;step;"+sHeader;
            elif level == 'loc':
                sHeader = "sim;step;loc;" + sHeader
            #-- end if
            print(sHeader)
        
        bHeader = False            

    if (level == 'sim') and (mode == 'det'):
        mode = 'acc'
        
    for step in h[sim].keys():
        r = show_counts_step(h, sim, step, mode, level, bHeader)
        ycounts[0]  = ycounts[0]  + r[1][0]
        ycounts[1]  = ycounts[1]  + r[1][1]
        mtcounts[0] = mtcounts[0] + r[0][0]
        mtcounts[1] = mtcounts[1] + r[0][1]
        hyb_sum     = hyb_sum   + r[2]
        hyb_count   = hyb_count + r[3]
    #-- end for
        
    if level == 'sim':
        if hyb_count > 0:
            avg_hyb = hyb_sum/hyb_count
        else:
            avg_hyb = -1
        #-- end if

        print("%s;%d;%d;%d;%d;%f"%(sim, mtcounts[0], mtcounts[1], ycounts[0], ycounts[1], avg_hyb))
    #-- end if

    return [mtcounts, ycounts, hyb_sum, hyb_count]
#-- end def

#-----------------------------------------------------------------------------
#-- show_counts_file
#--  get histogram for specified sim and step
#--
def show_counts_file(h, mode, level, bHeader):
    ycounts   = [0, 0]
    mtcounts  = [0, 0]
    hyb_sum   = 0
    hyb_count = 0

    if (bHeader):
        if mode == mode: #'acc':
            sHeader = "mtcounts_S;mtcounts_N;ycounts_S;ycounts_N;avg_hyb"
            if level == 'file':
                sHeader = sHeader;
            elif level == 'sim':
                sHeader = "sim;"+sHeader;
            elif level == 'step':
                sHeader = "sim;step;"+sHeader;
            elif level == 'loc':
                sHeader = "sim;step;loc;" + sHeader
            #-- end if
            print(sHeader)
        
        bHeader = False            

   
    if (level == 'file') and (mode == 'det'):
        mode = 'acc'
                             

        
    for sim in h.keys():
        r = show_counts_sim(h, sim, mode, level, bHeader)
        ycounts[0]  = ycounts[0]  + r[1][0]
        ycounts[1]  = ycounts[1]  + r[1][1]
        mtcounts[0] = mtcounts[0] + r[0][0]
        mtcounts[1] = mtcounts[1] + r[0][1]
        hyb_sum     = hyb_sum   + r[2]
        hyb_count   = hyb_count + r[3]
    #-- end for
        
    if level == 'file':
        if hyb_count > 0:
            avg_hyb = hyb_sum/hyb_count
        else:
            avg_hyb = -1
        #-- end if
        
        print("%d;%d;%d;%d;%f"%(mtcounts[0], mtcounts[1], ycounts[0], ycounts[1], avg_hyb))
    #-- end if
    
    return [mtcounts, ycounts, hyb_sum, hyb_count]
#-- end def


sim      = ""
step     = ""
loc      = ""
#num_bin  = -1

if len(argv) > 2:
    level = argv[1]
    stderr.write("level:[%s]\n"%level)
    if (level =="file") or (level == "sim") or (level == "step") or (level == "loc"):
        hdf_file = argv[2]
        h = h5py.File(hdf_file, "r")
        stderr.write("file:[%s]\n"%argv[2])
        if level == 'file':
            show_counts_file(h, 'acc', level, True)
        else:
            if len(argv) > 3:
                sim = argv[3];
                stderr.write("sim:[%s]\n"%sim)
                if level == 'sim':
                    show_counts_sim(h, sim, 'acc', level, True)
                else:
                    if len(argv) > 4:
                        step=argv[4]
                        stderr.write("step:[%s]\n"%step)
                        if level == 'step':
                            show_counts_step(h, sim, step, 'acc', level, True)
                        else:
                            if len(argv) > 5:
                                loc=argv[5]
                                stderr.write("loc:[%s]\n"%loc)
                                show_counts_loc(h, sim, step, loc, 'det', 'loc', True);
                            else:
                                show_counts_step(h, sim, step, 'det', level, True);
                            #-- end if
                        #-- end if
                    else:
                        show_counts_sim(h, sim, 'det', level, True)
                    #-- end if
                #-- end if
            else:
                show_counts_file(h, 'det', level, True)
            #-- end if
        #-- end if
    else:
        stderr.write("mode must be 'file', 'sim', 'step' or 'loc'\n")
    #-- end if
else:
    stderr.write("%s level <hdf_file> <path>\n"%(argv[0].split('/')[-1]))
    stderr.write("  path ::= [<simulation> [<step> [<loc>]]] \n")
    stderr.write(" level ::= 'file' | 'sim' | 'step' | 'loc'\n")
    
#-- end if 
