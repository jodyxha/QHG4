#!/usr/bin/python


import sys
import os
import glob
import subprocess as sp
from shutil import copyfile
import gzip

KRAKEN_BASE    = "/data/jw_simulations/qhgtests"

DEF_POP_PREFIX  = "ooa_pop-Sapiens_ooa_"
DEF_STAT_PREFIX = "ooa_SGCVNM"
DEF_OUT_NAME    = "analysis"

COL_RED   = "\033[0;31m"
COL_BLUE  = "\033[0;34m" 
COL_OFF   = "\033[0m"

SEQ_GENE   =  "genes"
SEQ_PHENE  =  "phenes"
GEN_SAMP   =  "QDFGenomeSampler2"
GEN_MERG   =  "BinGeneMerge2"
GEN_DIST   =  "GeneDist2"

PHEN_SAMP  =  "QDFPhenomeSampler2"
PHEN_MERG  =  "BinPheneMerge2"
PHEN_DIST  =  "PheneDist2"

PROC_LOG   =  "PCOCreator_%s.log"
PROC_SEP   =  "-------------------------------\n"



seq_apps ={}
seq_apps[SEQ_GENE]  = {"samp":GEN_SAMP,  "merg":GEN_MERG,  "dist":GEN_DIST}
seq_apps[SEQ_PHENE] = {"samp":PHEN_SAMP, "merg":PHEN_MERG, "dist":PHEN_DIST}

BLOCK_SIZE = 17179869176

CLEAN_NONE = "keep_qdfs"
CLEAN_GZIP = "gzip_qdfs"
CLEAN_RM   = "rm_qdfs"

#-----------------------------------------------------------------------------
#--
#--
class QHGError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class

#-----------------------------------------------------------------------------
#--
#--
class ParamError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class


#-------------------------------------------------------------------------
#-- b2s
#--   bytes to string
#--   for python 2 use basestring instead of str
#--
def b2s(b):
    if isinstance(b, str):
        s = b
    else:
        s = "".join(map(chr,b))
    return s
 
    #return "".join(map(chr,b))
#-- end def



#-----------------------------------------------------------------------------
# gunzip_file
#  gunzip gz_file to out_file
#
def gunzip_file(gz_file, out_file):
    
    fIn = gzip.open(gz_file, "rb")
    fOut = open(out_file, "wb")
    while True:
        block = fIn.read(4294967294)
        if (len(block) > 0):
            x = fOut.write(block)
        else:
            print("done")
            break
        #-- end if
    #-- end while
    fIn.close()
    fOut.close()
#-- end def


#-----------------------------------------------------------------------------
# gzip_file
#  gzip in_file to gz_file
#
def gzip_file(in_file, gz_file):
    
    fIn = open(in_file, "rb")
    fOut = gzip.open(gz_file, "wb")
    while True:
        block = fIn.read(4294967294)
        if (len(block) > 0):
            x = fOut.write(block)
        else:
            print("done")
            break
        #-- end if
    #-- end while

    fIn.close()
    fOut.close()
#-- end def
    

#-----------------------------------------------------------------------------
#--
#--
class PCOCreator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qhgdir, datadir, pop_pat, stat_pat, zipped, logdir, times, outtop, outname, seq_type, cloudstyle=False):
        self.QHGdir      = qhgdir
        self.datadir     = datadir.rstrip('/')
        self.logdir      = logdir
        self.pop_pat     = pop_pat
        self.stat_pat    = stat_pat
        if ':' in times:
            self.times       = times.split(':')
        else:
            self.times       = times.split(' ')
        #-- end if
        self.zipped      = zipped
        self.outtop      = outtop
        self.outname     = outname
        self.cloudstyle  = cloudstyle

        self.logname     = PROC_LOG % (seq_type)
        self.proclogfile = open(self.logname, "wt")
        self.popfile  = self.datadir + '/' + self.pop_pat
        self.statfile = self.datadir + '/' + self.stat_pat

        i0Pop=self.pop_pat.find('#')                  
        i1Pop=self.pop_pat.rfind('#')                  
        n = i1Pop - i0Pop +1
        self.pat=""
        for i in range(n):
            self.pat = self.pat + '#'
            #-- end for
        print("pat for[%s]:  %d [%s]" %(self.pop_pat, n, self.pat))

        #-- 
        if self.cloudstyle:
            self.simname = self.datadir.split("output")[0].strip("/").split("/")[-1]
        else:
            self.simname = self.datadir.split("/")[-1]
        #-- end if
        print("datadir [%s], simname [%s]\n" % (self.datadir, self.simname))
        try:
            print("logdir: [%s]" % self.logdir)
            
            # check if required directories exist
            iResult = self.checkExistence()
            
            self.gridfile = self.QHGdir + "/resources/grids/EmptyGridSGCV_256.qdf"
            self.locfile  = self.QHGdir + "/genes/LocationListGridsRed.txt"

            self.set_outname(self.outname)
        except ParamError as e:
            raise e
        except Exception as e:
            raise QHGError("constructor", "File existence check failed. Exception: %s" % (e))
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- set_outname
    #--
    def set_outname(self, outname):
        self.outname = outname
        self.outdir= self.outtop + "/" + self.simname
        print("outdir [%s], outname  [%s]\n" %( str(self.outdir), str(self.outname)))
        self.outbody=self.outdir + "/" + self.outname
        print("new outbody: [%s]" % self.outbody)
    #-- end def


    #-------------------------------------------------------------------------
    #-- checkExistence
    #--
    def checkExistence(self):
    
        if os.path.exists(self.datadir) and os.path.isdir(self.datadir):

            self.suffix = ".gz" if self.zipped else ""
            self.laststat = self.statfile.replace(self.pat, self.times[-1])
            if os.path.exists(self.laststat+self.suffix) and os.path.isfile(self.laststat+self.suffix):
                exist_ok = True
                curqdf=""
                for t in self.times:
                    curqdf = self.popfile.replace(self.pat, t) + self.suffix                 

                    if not (os.path.exists(curqdf) and os.path.isfile(curqdf)):
                        exist_ok = False
                        break
                    #-- end if
                #-- end for
                        
                if exist_ok:
                    if os.path.exists(self.logdir) and os.path.isdir(self.logdir):
                        print("existence ok")
                    else:
                        print("aa8")
                        raise QHGError("checkExistence", "logdir [%s] does not exist or isn't a directory" % (self.logdir))
                    #-- end if
                    
                else:
                    raise QHGError("checkExistence", "popfile [%s] does not exist" % (curqdf))
                #-- end if
            else:
                raise QHGError("checkExistence", "statfile [%s] does not exist" % (self.laststat+self.suffix))
            #-- end if
            
        else:
            raise QHGError("checkExistence", "datadir [%s] does not exist or isn't a directory" % (self.datadir))
        #-- end if
    #-- end def



    #-------------------------------------------------------------------------
    #-- gunzipFiles
    #--
    def gunzipFiles(self):
        print("times: "+str(self.times))
        print("gunzipping")
        self.proclogfile.write("%sgunzipping\n" % (PROC_SEP))
        
        iResult = 0

        try:
            print("gunzipping [%s]" % (self.laststat+".gz"))
            gunzip_file(self.laststat+".gz",  self.laststat)
        except Exception as e:
            iResult = -1
            print("couldn't gunzip [%s][%s]" % (self.laststat+".gz", str(e)))
            self.proclogfile.write("couldn't gunzip [%s]\n" % (self.laststat+".gz"))
            raise QHGError("gunzipFiles", "couldn't gunzip [%s]" % (self.laststat+".gz"))
        #-- end try

        for t in self.times:
            try:
                curqdf = self.popfile.replace(self.pat, t)
                print("gunzipping [%s]" % (curqdf+".gz"))
                gunzip_file(curqdf+".gz", curqdf)
            except Exception as e:
                iResult = -1
                print("couldn't gunzip [%s][%s]" % (curqdf, str(e)))
                self.proclogfile.write("couldn't gunzip [%s]\n" % (self.laststat+".gz"))
                raise QHGError("gunzipFiles", "couldn't gunzip [%s]" % (self.laststat+".gz"))
                break
            #-- end try
        #-- end for

        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- clean_up
    #--
    def clean_up(self, clean_type):
        if (clean_type == CLEAN_NONE):
            print("no cleaning")
            pass
        elif (clean_type == CLEAN_GZIP):
            self.gzipFiles()
        elif (clean_type == CLEAN_RM):
            self.removeFiles()
        else:
            print("unknown clean type [%d]" % (clean_type))
            self.proclogfile.write("unknown clean type [%d]" % (clean_type))
        #-- end if
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- gzipFiles
    #--
    def gzipFiles(self):
        print("gzipping")
        self.proclogfile.write("%sgzipping\n" % (PROC_SEP))
        
        iResult = 0
        try:
            gzip_file(self.laststat,  self.laststat+".gz")
            for t in self.times:
                try:
                    curqdf = self.popfile.replace(self.pat, t)
                    gzip_file(curqdf, curqdf+".gz")
                except:
                    iResult = -1
                    self.proclogfile.write("couldn't gzip [%s]\n" % (curqdf))
                    raise QHGError("gzipFiles", "couldn't gzip [%s]" % (curqdf))
                    break
                #-- end try
            #-- end for
        except:
            iResult = -1
            self.proclogfile.write("couldn't gzip [%s]\n" % (self.laststat))
            raise QHGError("gzipFiles", "couldn't gzip [%s]" % (self.laststat))
        #-- end try   

        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- removeFiles
    #--
    def removeFiles(self):
        print("removing unzipped")
        self.proclogfile.write("%sremoving unzipped\n" % (PROC_SEP))
        
        iResult = 0
        # only delete qdf if there is a gz version of it
        gzfile = self.laststat+".gz"
        if (os.path.exists(gzfile) and os.path.isfile(gzfile)):
            os.remove(self.laststat)
        else:
            self.proclogfile.write("won't remove [%s] as there is no [%s]\n" % (self.laststat, gzfile))
        #-- end if
            
        for t in self.times:
            curqdf = self.popfile.replace(self.pat, t)
            gzfile = curqdf+".gz"
            if (os.path.exists(gzfile) and os.path.isfile(gzfile)):
                os.remove(curqdf)
            else:
                self.proclogfile.write("won't remove [%s] as there is no [%s]\n" % (curqdf, gzfile))
            #-- end if

        #-- end for
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- createOuputDir
    #--
    def createOuputDir(self):
        self.proclogfile.write("%screateOutputDir\n" % (PROC_SEP))

        if not os.path.exists(self.outtop):
            os.mkdir(self.outtop)
        else:
            if (os.path.isdir(self.outtop)):
                self.proclogfile.write("[%s] already exists\n" % (self.outtop))
            else:
                self.proclogfile.write("createOuputDir failed: [%s] is not a directory\n" % (self.outtop))
                raise QHGError("createOuputDir", "[%s] is not a directory" % (self.outtop))
            #-- end if
        #-- end if

        if not os.path.exists(self.outdir):
            os.mkdir(self.outdir)
        else:
            if (os.path.isdir(self.outdir)):
                print("[%s] already exists" % (self.outdir))
            else:
                self.proclogfile.write("createOuputDir failed: [%s] is not a directory\n" % (self.outdir))
                raise QHGError("createOuputDir", "[%s] is not a directory" % (self.outdir))
            #-- end if
            
        #-- end if
        
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doQDFSampler
    #--
    def doQDFSampler(self, seq_type, samp_in, block_range):
        app_name = seq_apps[seq_type]["samp"]
       
        for t in self.times:

            print("%s for [%s]" % (app_name, t))
            self.proclogfile.write("%s%s for [%s]\n" % (PROC_SEP, app_name, t))
            
            curpop = self.popfile.replace(self.pat, t)
            curout = self.outbody + "_" + t
            if (samp_in != ""):
                sampline = "--read-samp=%s_%s.smp" % (samp_in, t)
            else:
                sampline = ""
            #-- end if
            if (block_range != ""):
                rangeline = "--set-range=%s" % (block_range)
            else:
                rangeline = ""
            #-- end if

            
            self.proclogfile.write("outbody[%s]->[%s]\n" % (self.outbody, curout))
            command = [self.QHGdir + "/genes/" + app_name,
                       "-i", curpop,
                       "-s", "Sapiens_ooa",
                       "-o", curout,
                       "-f","bin:asc",
                       "--location-file="+self.locfile,
                       "--write-samp="+curout+".smp",
                       "-g", self.gridfile]#,
                       #"-q"]
            if (sampline != ""):
                command.append(sampline)
            #-- end if
            if (rangeline != ""):
                command.append(rangeline)
            #-- end if

            proc = sp.Popen(command,  stdout=sp.PIPE,  stderr=sp.PIPE)
            sOut, sError = proc.communicate()
            self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
            iResult = proc.returncode
            if (iResult == 0):
                print("+++ success +++")
            else:
                self.proclogfile.write("*** %s failed for [%s]:\n  %s\n error: %s\n" % (app_name, curpop, str(command), b2s(sError)))
                raise QHGError("doQDFSampler","%s failed for [%s]:\n  %s\n error: %s" % (app_name, curpop, str(command), b2s(sError)))
            #-- end if
            sys.stdout.flush()
        #-- end for
        return iResult
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doBinSeqMerge2
    #--
    def doBinSeqMerge2(self, seq_type):
        app_name = seq_apps[seq_type]["merg"]
        print(app_name)
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))
        
        infiles = []
        for t in self.times:
            infiles.append(self.outbody + "_" + t + ".bin")
        #-- end for
        command = [self.QHGdir + "/genes/" + app_name,
                   self.outbody+".all.bin"] + infiles
                
        proc = sp.Popen(command, stdout=sp.PIPE, stderr=sp.PIPE)
                    
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** %s failed:\n  %s\nerror: %s\n" % (app_name, str(command), b2s(sError)))
            raise QHGError("doBinSeqMerge2", "%s failed:\n  %s\nerror: %s" % (app_name, str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doAscSeqMerge2
    #--
    def doAscSeqMerge2(self, seq_type):
        iResult = 0
        app_name = "AscSeqMerge"
        print("AscMerge")
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))

        try:
            fOut = open(self.outbody+".all.asc", "wt")
            for t in self.times:
                temp_name = self.outbody + "_" + t + ".asc"
                fIn = open(temp_name)
                for line in fIn:
                    fOut.write(line)
                #-- end for
            #-- end for
            fOut.close()
        except Exception as e:    
            self.proclogfile.write("*** %s failed:\n  %s\n" % (app_name, e))
            raise QHGError("doAscSeqMerge2", "%s failed:\n  %s" % (app_name, e))
            sys.stdout.flush()
        #-- end try
        
        print("+++ success +++")
        return iResult
    #-- end def



    #-------------------------------------------------------------------------
    #-- doSeqDist_s
    #--
    def doSeqDist_s(self, seq_type, dist_method):
        self.doSeqDist(seq_type, dist_method, self.outbody + ".all.bin", self.outbody)
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doSeqDist
    #--
    def doSeqDist(self, seq_type, dist_method, bin_name, out_name):

        app_name = seq_apps[seq_type]["dist"]
        print(app_name)
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))
        
        command = [self.QHGdir+"/genes/" + app_name,
                   "-g", bin_name,
                   "-o", out_name,
                   "-m", self.laststat,
                   "-G", self.gridfile]
        if (dist_method):
            command.extend(["-d", dist_method])
        #-- end if
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** %s failed:\n  %s\nerror %s\n" % (app_name, str(command), b2s(sError)))
            raise QHGError("doSeqDist", "%s failed:\n  %s\nerror %s" % (app_name, str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doNullDist
    #--
    def doNullDist(self, bin_name, out_name):

        app_name = "NullDist2"
        print(app_name)
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))
        
        command = [self.QHGdir+"/genes/" + app_name,
                   "-p", bin_name,
                   "-o", out_name,
                   "-m", self.laststat,
                   "-G", self.gridfile]
        #-- end if
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** %s failed:\n  %s\nerror %s\n" % (app_name, str(command), b2s(sError)))
            raise QHGError("doSeqDist", "%s failed:\n  %s\nerror %s" % (app_name, str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def



    #-------------------------------------------------------------------------
    #-- doCalcPCO_s
    #--
    def doCalcPCO_s(self):
        return self.doCalcPCOS(self.outbody)
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doCalcPCO
    #--
    def doCalcPCO(self, outbody):
        print("CalcPCO")
        self.proclogfile.write("%sCalcPCO\n" % (PROC_SEP))
        
        command = [self.QHGdir+"/genes/calcpco_R.sh",
                   outbody+".full.mat",
                   outbody+".full.pco.temp"]

        self.proclogfile.write("%s\n" % (str(command)))
                                  
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** CalcPCO failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doCalcPCO", "CalcPCO failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doCalcPCA
    #--
    def doCalcPCA(self, analysis, inbody, outbody):
        print("CalcPCA[%s]" % analysis)
        self.proclogfile.write("%sCalcPCA[%s]\n" % (PROC_SEP, analysis))

        
        command = [self.QHGdir+"/genes/calcpca_R.sh",
                   inbody+".all.asc",
                   outbody+".full."+analysis+".temp",
                   analysis]

        self.proclogfile.write("%s\n" % (str(command)))
                                  
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** CalcPCA failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doCalcPCA", "CalcPCA failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doColumnMerger_s
    #--
    def doColumnMerger_s(self, item):
        return self.doColumnMerger(self.outbody, item)
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doColumnMerger
    #--
    def doColumnMerger(self, outbody, item):
        print("ColumnMerger")
        self.proclogfile.write("%sColumnMerger\n" %(PROC_SEP))
    
        
        
        command = ["python", self.QHGdir+"/genes/ColumnMerger.py",
                   outbody+".tagdists",
                   "all",
                   outbody+".full."+item,
                   "all",
                   outbody+"."+item+".tab"]

        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** ColumnMerger failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doColumnMerger", "ColumnMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doTableMerger
    #--
    def doTableMerger(self, remove_comment):
        print("TableMerger")
        self.proclogfile.write("%sTableMerger\n" %(PROC_SEP))

        iResult = 0
        try:
            fOut = open(self.outbody+".tab", "wt")
            has_header = False
            for t in self.times:
                temp_name = self.outbody + "_" + t
                fIn = open(temp_name + ".tab")
                for line in fIn:
                    # only write first comment
                    if line.startswith('#'):
                        if not has_header:
                            has_header = True
                            if (remove_comment):
                                fOut.write(line[1:])
                            else:
                                fOut.write(line)
                            #-- end if
                        #-- end if   
                    else:   
                        fOut.write(line)
                    #-- end if
                #-- end for
                fIn.close()
            #-- end for
            fOut.close()
        except Exception as e:
            iResult = -1
            self.proclogfile.write("*** TableMerger failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doTableMerger", "TableMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        else:
            print("+++ success +++")
        #-- end try
        
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doRenameAscs
    #--    (unused?)
    #--
    def doRenameAscs(self):
        print("doRenameAscs")
        self.proclogfile.write("%sdoRenameAsc\n" %(PROC_SEP))

        iResult = 0
        try:
            for t in self.times:
                temp_name = self.outbody + "_" + t
                self.proclogfile.write("copying %s to %s\n" %(temp_name + ".asc", temp_name + ".full.pco.temp"))
                copyfile(temp_name + ".asc", temp_name + ".full.pco.temp")
            #-- end for
        except Exception as e:
            iResult = -1
            self.proclogfile.write("*** TableMerger failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doTableMerger", "TableMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        else:
            print("+++ success +++")
        #-- end try
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doPCOHeaders
    #--
    def doPCOHeaders(self, item):
        print("doPCOHeaders")
        self.proclogfile.write("%sdoPCOHeaders\n" %(PROC_SEP))

        # count columns (make sure allrows have same number of cols)
        iC = 0
        in_name  = "%s.full.%s.temp" % (self.outbody, item)
        out_name = "%s.full.%s" %   (self.outbody, item)
        print("Opening [%s]" % (in_name))
        fTemp = open(in_name, "rt")
        for x in fTemp:
            iC1 = len(x.split())
            if iC == 0:
                iC = iC1
            elif iC != iC1:
                self.proclogfile.write("*** doPCOHeaders failed:\n  number of columns not constant")
                raise QHGError("doPCOHeaders", "doPCOHeaders failed:\n   number of columns not constant")
            #-- end if
        #-- end for
        fTemp.close()

        sHeader = ""
        for i in range(1, iC+1):
            sHeader = sHeader + "%s%d " % (item, i)
        #-- end for
        iC = 0
        fOut = open(out_name, "wt")
        fOut.write("%s\n" % (sHeader))
        fIn =  open(in_name, "rt")
        for line in fIn:
            fOut.write(line)
        #-- end for

        fIn.close()
        fOut.close()
        
         
    #-- end def


    #-------------------------------------------------------------------------
    #-- doPCOHeadersMulti
    #--
    def doPCOHeadersMulti(self, item):
        print("doPCOHeadersMulti")
        self.proclogfile.write("%sdoPCOHeadersMulti\n" %(PROC_SEP))
        # count columns
        iC = 0
        for t in self.times:
            temp_name = self.outbody + "_" + t
            fTemp = open(temp_name + ".full.pco.temp", "rt")
            for x in fTemp:
                iC1 = len(x.split())
                if iC == 0:
                    iC = iC1
                elif iC != iC1:
                    self.proclogfile.write("*** doPCOHeaders failed:\n  number of columns not constant")
                    raise QHGError("doPCOHeaders", "doPCOHeaders failed:\n   number of columns not constant")
                #-- end if
            
            #-- end for
            fTemp.close()
        #-- end for
        self.proclogfile.write("  found num columns: %d\n" % (iC))

        sHeader = ""
        for i in range(1, iC+1):
            sHeader = sHeader + "%s%d " % (item, i)
        #-- end for
        iC = 0
        for t in self.times:
            temp_name = self.outbody + "_" + t
            fOut = open(temp_name + ".full.pco", "wt")
            fOut.write("%s\n" % (sHeader))
            fIn =  open(temp_name + ".full.pco.temp", "rt")
            for line in fIn:
                fOut.write(line)
            #-- end for
            fIn.close()
            fOut.close()
        #-- end for
        
         
    #-- end def


    #-------------------------------------------------------------------------
    #-- doCleanIntermediates
    #--
    def doCleanIntermediates(self):
        print("doCleanIntermediates")
        self.proclogfile.write("%sdoCleanIntermediates\n" %(PROC_SEP))

        iResult = 0
        try:
            for t in self.times:
                temp_name = self.outbody + "_" + t
                for f in [".full.pco.temp",".full.pco",".full.mat",".bin",".tab",".tagdists"]:
                    cur_name = temp_name + f
                    self.proclogfile.write("  checking [%s]\n" % (cur_name))
                    if os.path.exists(cur_name):
                        self.proclogfile.write("  removing [%s]\n" % (cur_name))
                        os.remove(cur_name)
                    #-- end if
                #-- end for
            #-- end for
            
        except Exception as e:
            iResult = -1
            self.proclogfile.write("*** doCleanIntermediates failed:\nerror %s\n" % (str(e)))
            raise QHGError("doCleanIntermediates", "doCleanIntermediates failed:\nerror %s" % (str(e)))
        else:
            print("+++ success +++")
        #-- end try
        
        sys.stdout.flush()
        return iResult
    #-- end def

          
    #-------------------------------------------------------------------------
    #-- doAttributes
    #--
    def doAttributes(self):
        print("Attributes")
        self.proclogfile.write("%sAttributes\n" % (PROC_SEP))
        attrfile = open(self.outbody + ".attr", "wt")
        curpop = self.popfile.replace(self.pat,  self.times[-1])
        command = [self.QHGdir+"/useful_stuff/show_attr",
                   "all",
                   curpop]

        proc = sp.Popen(command,  stdout=attrfile, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        attrfile.close()
        self.proclogfile.write("command %s\n" % (str(command)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** doAttributes failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doAttributes", "ColumnMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if

        sys.stdout.flush()
        return iResult
    #-- end def

        
    #-------------------------------------------------------------------------
    #-- doArrivals
    #--
    def doArrivals(self):
        print("arrival times")
        self.proclogfile.write("%sarrival times\n" % (PROC_SEP))
        
        arrfile = open(self.outbody + ".arr", "wt")
        command = [self.QHGdir+"/genes/ArrivalCheck",
                    "-g", self.laststat,
                    "-l", self.locfile,
                    "-n"]
        proc = sp.Popen(command,  stdout=arrfile, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        arrfile.close()
        self.proclogfile.write("command %s\n" % (str(command)))

        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** doArrivals failed:\n  %s\n" % (str(command)))
            raise QHGError("doArrivals", "doArrivals failed:\n  %s" % (str(command)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def

        
    #-------------------------------------------------------------------------
    #-- doCopyLogs
    #--   in our cloud setup, the name of log and out are not known,
    #--   but they are the only ones in the log dir
    #--
    def doCopyLogs(self):
        print("CopyLogs")
        self.proclogfile.write("%sCopyLogs\n" % (PROC_SEP))
        
        if self.cloudstyle:
            logfile = glob.glob(self.logdir + "/*.log")[0]
            outfile = glob.glob(self.logdir + "/*.out")[0]
        else:
            logfile = self.logdir + "/" + self.simname + ".log"
            outfile = self.logdir + "/" + self.simname + ".out"
        #-- end if
        logtarg = logfile.split("/")[-1]
        outtarg = outfile.split("/")[-1]
        self.proclogfile.write("copying [%s] to [%s]\n" % (logfile,  self.outdir+"/"+logtarg))
        self.proclogfile.write("copying [%s] to [%s]\n" % (outfile,  self.outdir+"/"+outtarg))
        try:
            copyfile(logfile, self.outdir+"/"+logtarg)
            copyfile(outfile, self.outdir+"/"+outtarg)
        except Exception as e:
            self.proclogfile.write("*** doCopyLogs failed:\n  %s\n" % (str(e)))
            raise QHGError("doCopyLogs", "doCopyLogs failed:\n  %s" % (str(e)))
        else:
            print("+++ success +++")
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- doProcess
    #--
    def doProcess(self, seq_type):
        self.createOuputDir()
        
        if self.zipped:
                self.gunzipFiles()
            #-- end if

        self.doQDFSampler(seq_type)

        self.doBinSeqMerge2(seq_type)
        
        self.doSeqDist(seq_type)

        self.doCalcPCO_s()

        self.doColumnMerger_s()

        self.doAttributes()

        self.doArrivals()

        self.doCopyLogs()

        if self.zipped:
            self.gzipFiles()
        #-- end if
    #-- end def


    #-------------------------------------------------------------------------
    #-- __del__
    #--
    def __del__(self):
        self.proclogfile.write("ending PCOCreator\n")
        self.proclogfile.close()
        print("Outputs written to [%s]\n" % (self.logname))
    #-- end def
    
#-- end class

