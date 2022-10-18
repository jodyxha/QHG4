#!/usr/bin/python


from sys import argv, stdout
import os.path
import glob
import subprocess
from shutil import copyfile
import PCOCreator

MODE_CLOUD  = "cloud"
MODE_TRIOPS = "triops"
MODE_KRAKEN = "kraken"
MODE_NUDI   = "nudi"
modes=[MODE_CLOUD, MODE_TRIOPS, MODE_KRAKEN, MODE_NUDI]

CLOUD_QHGDIR   = "/home/centos/qhg_data/QHG3"
KRAKEN_QHGDIR  = "/data/jw_simulations/qhgtests/src/QHG3"
TRIOPS_QHGDIR  = "/home/jody/progs/QHG3/trunk"
NUDI_QHGDIR    = "/home/jody/progs/QHG3"
KRAKEN_BASE    = "/data/jw_simulations/qhgtests"

DEF_POP_PATTERN  = "ooa_pop-Sapiens_ooa__###000.qdf"
DEF_STAT_PATTERN = "ooa_SGCVNM_###000.qdf"
DEF_OUT_NAME     = "analysis#"

COL_RED   = "\033[0;31m"
COL_BLUE  = "\033[0;34m" 
COL_OFF   = "\033[0m"

known_dists      = {'genes':['default'], 'phenes':['dist-avg', 'dist-cross', 'dist-euclid', 'default']}
known_analyses   = {'genes':['pco'], 'phenes':['pco', 'pca-cov', 'pca-cor']}
                 
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
#-- readParams
#--  paramdefs: dictionary opt => [name, default]
#--             
def readParams(param_defs):
    i = 1
    results = {}
    
    keys = param_defs.keys()
    
    # set default values
    for k in keys:
        results[param_defs[k][0]] = param_defs[k][1]
    #-- end for
    
    while i < len(argv):
        if argv[i] in keys:
            if (i+1) < len(argv):
                results[paramdefs[argv[i]][0]] = argv[i+1]
                i = i + 2
            else:
                raise ParamError("readParams", "expected value for [%s]" % (argv[i]))
            #-- end if
        else:
            raise ParamError("readParams", "unknown parameter [%s]" % (argv[i]))
        #-- end if
    #-- end while

    for k in paramdefs:
        if param_defs[k][1] is None:
            if (not param_defs[k][0] in results.keys()) or (results[param_defs[k][0]] is None):
                raise ParamError("readParams", "required parameter missing [%s]" % (param_defs[k][0]))
            #-- end if
        #-- end if
    #-- end for
        
    return results;
    
#-- end def
    

#-------------------------------------------------------------------------
#-- splitActions
#--  exepect string of form <seq-type>:<dist-meth>[,<seq-type>:<dist-meth>]*
#--  returns dictionary: seq_type => list of dist methods
#--
def splitActions(action_string):
    actions = {}
    els = action_string.split(',')

    legal_seqs = list(known_dists.keys())
    for el in els:
        a = el.split(':')
        if a[0] in legal_seqs:
            
            if (len(a) == 1):
                a.append('default')
            #-- end if
            for x in a[1:]:
                if (x in known_dists[a[0]]):
                    if a[0] in actions:
                        actions[a[0]].append(x)
                    else:
                        actions[a[0]] = [x]
                    #-- end if
                else:
                    raise ParamError("splitActions", "illegal dist method for '%s': [%s]" % (a[0], x))
                #-- end if
            #-- end for
        else:
            raise ParamError("splitActions", "unknown seqtype [%s]" % (a[0],))
        #-- end if
    #-- end for
    return actions

#-- end def


#-------------------------------------------------------------------------
#-- splitAnalyses
#--  exepect string of form <seq-type>:<ana-meth>[:<ana-meth>]*[,<seq-type>:<ana-meth>[:<ana-meth>]*]*
#--  returns dictionary: seq_type => list of dist methods
#--
def splitAnalyses(analyses_string):
    analyses = {}
    els = analyses_string.split(',')

    legal_seqs = list(known_analyses.keys())
    for el in els:
        a = el.split(':')
        if a[0] in legal_seqs:
            
            if (len(a) == 1):
                a.append('pco')
            #-- end if
            for x in a[1:]:
                if (x in known_analyses[a[0]]):
                    if a[0] in analyses:
                        analyses[a[0]].append(x)
                    else:
                        analyses[a[0]] = [x]
                    #-- end if
                else:
                    raise ParamError("splitAnalyses", "illegal analysis method for '%s': [%s]" % (a[0], x))
                #-- end if
            #-- end for
        else:
            raise ParamError("splitAnalyses", "unknown seqtype [%s]" % (a[0],))
        #-- end if
    #-- end for
    return analyses

#-- end def

#-------------------------------------------------------------------------
#-- usage
#--
def usage():
    print("")
    print("%s - creating PCOs for a simulation" % (argv[0]))
    print("Usage")
    print("  %s -m <mode> -d <data-dir> -t <times> -o <outdir>" % (argv[0]))
    print("       [-p <pop-pattern>] [-s <stat-pattern>] [-n <outname>]")
    print("       [-l <logdir>] [-z <zipstate>]")
    print("       [-x <seq-type-dists>] [-a <analysis>] [-r <samp-file>]")
    print("       [-c <clean-type>]")
    print("       [-b <block-range>]")
    print("       [-dm <distance-method>]")
    print("or\n")
    print("  %s -q <qhgdir> -d <data-dir> -l >logdir>" % (argv[0]))
    print("        -d <data-dir> -l <logdir> -t <times> -o <outdir>")
    print("       [-p <pop-pattern>] [-s <stat-pattern>] [-n <outname>]")
    print("       [-z <zipstate>] [-x <seq-type-dists>] [-a <analysis>] [-r <samp-file>]")
    print("       [-c <clean-type>]")
    print("       [-b <block-range>]")
    print("       [-dm <distance-method>]")
    print("where")
    print("  mode             machine name: 'triops' | 'kraken' | 'cloud'")
    print("  qhgdir           QHG root directory")
    print("  datadir          directory containing the [gzipped] pop and stat files")
    print("  times            quoted list of  3-digit times (representing multiples of 1000)")
    print("  outdir           top output directory (in which a subdirectory with simulation name will be created)")
    print("  pop-pattern      pattern for pop qdfs (use '#' as placeholder for numbers) (default  %s)" % (DEF_POP_PATTERN))
    print("  stat-pattern     prefix for stat qdfs (use '#' as placeholder for numbers) (default  %s)" % (DEF_STAT_PATTERN))
    print("  outname          prefix for output files in <outdir> (default %s)" % (DEF_OUT_NAME))
    print("                   if outname ends with '#', sequence type ('genes','phenes') and join type (join','sep') are appended")
    print("  logdir           directory containing the .log and .out files");
    print("                     default triops: the QHG3/app directory")
    print("                     default kraken: the qhgtest directory")
    print("                     default cloud:  the QHGMain output directory")
    print("  zipstate         1: gunzipping and gzipping required")
    print("  seq-type-dists   sequencetypes with distance method : 'genes' or 'phenes' (default: 'genes')")
    print("                      <seq-type>:<dm>[:<dm>][,<seq-type>[:<dm>]]")
    print("                   where") 
    print("                       seq_type  =  'genes' | 'phenes'") 
    print("                       dm: distance calculation method:")  
    print("                       'default':     (diploid genomes)  min of normal and crosswise Hamming distances between chromosomes\n");
    print("                       'dist-euclid': (haploid phenomes) euclidean distance between chromosomes\n");
    print("                       'dist-avg':    (diploid phenomes) euclidean distance between averages of chromosomes\n");
    print("                       'dist-cross':  (diploid phenomes) min of normal and crosswise Euclidean distances between chromosomes\n");
    print("  analyis          sequencetypes with analysis ")
    print("                      <seq-type>[:<am>]*[,<seq-type>[:<am>]*]")
    print("                   where") 
    print("                       seq_type  =  'genes' | 'phenes'") 
    print("                       am: analysis method:")  
    print("                       'pco':         (genomes)   pco of distance matrix\n");
    print("                       'pco':         (phenomes)  pco of distance matrix\n");
    print("                       'pca-cov':     (phenomes)  pca of covarianceeuclidean distance between chromosomes\n");
    print("                       'pca-cor':     (phenomes) euclidean distance between averages of chromosomes\n");
    print("  clean-type       '%s' or '%s' or '%s' (default: '%s')" % (PCOCreator.CLEAN_NONE, PCOCreator.CLEAN_GZIP, PCOCreator.CLEAN_RM, PCOCreator.CLEAN_NONE))
    print("  block-range      only use nucleotides in specified blocks.")
    print("                   Format: \"<first-block>:<num-blocks>\"")
    print("  samp-file        name of previously created sample file (default: None)")
    print("")
#-- end def 


#-------------------------------------------------------------------------
#-- determineQHGDir
#--
def determineQHGDir(mode):
    if mode == MODE_CLOUD:
        qhgdir     = CLOUD_QHGDIR
        cloudstyle = True
    elif mode == MODE_KRAKEN:
        qhgdir     = KRAKEN_QHGDIR
        cloudstyle = False
    elif mode == MODE_TRIOPS:
        qhgdir     = TRIOPS_QHGDIR
        cloudstyle = False
    elif mode == MODE_NUDI:
        qhgdir     = NUDI_QHGDIR
        cloudstyle = False
    elif os.path.exists(mode):
        qhgdir     = mode
        cloudstyle = False
    #-- end if
    return qhgdir,cloudstyle
#-- end def


#-------------------------------------------------------------------------
#-- determineLogDir
#--
def determineLogDir(mode, qhgdir, datadir, logdir):
    if logdir == "":
        if mode == MODE_CLOUD:
            logdir = datadir.split("output")[0]
        elif mode == MODE_KRAKEN:
            logdir = KRAKEN_BASE
        elif mode == MODE_TRIOPS:
            logdir = qhgdir + "/app"
        elif mode == MODE_NUDI:
            logdir = qhgdir + "/app"
        #-- end if
    #-- end if
    return logdir
#-- end def


#-------------------------------------------------------------------------
#-- showParams
#--
def showParams(params, outstream=None):
    if outstream is None:
        outstream = stdout
    #--- end if
    
    for k in params:
        outstream.write("%-10s[%s]\n" % (k, params[k]))
    #-- end for
    
#-- end def


#-------------------------------------------------------------------------
#-- copyPCOLog
#--
def copyPCOLog(pco_logname, params, outdir):
    try:
        clog = open(outdir + '/' + pco_logname, 'w')
        showParams(params, clog)

        ilog = open(pco_logname, 'r')
        clog.write(ilog.read())
        ilog.close()
        clog.close()
    except Exception as e:
        print("Error copying %s to %s\n" % (outdir + '/' + pco_logname, pco_logname))
        print(str(e))
    #-- end try
#-- end def

   
#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    try:
        if (len(argv) <= 8):
            usage()
        else:
            paramdefs = {
                "-m" : ["mode", None],
                "-q" : ["qhqdir", ""],
                "-d" : ["datadir", None],
                "-p" : ["pop_pat", DEF_POP_PATTERN],
                "-s" : ["stat_pat", DEF_STAT_PATTERN],
                "-z" : ["zipped", False],
                "-l" : ["logdir", ""],
                "-t" : ["times", None],
                "-o" : ["outdir", None],
                "-n" : ["outname", DEF_OUT_NAME],
                "-x" : ["actions", "genes"],
                "-r" : ["in_samp", ""],
                "-c" : ["clean_type", PCOCreator.CLEAN_NONE],
                "-b" : ["block_range", ""],
                "-a" : ["analyses", ""],
                "-u" : ["no_pco", False]
                }

            results = readParams(paramdefs)
            showParams(results)
            
            if not results["mode"] is None:
                qhgdir,cloudstyle = determineQHGDir(results["mode"])
                print("Have qhgdir [%s], cloudstyle:%s" % (qhgdir, str(cloudstyle)))
                logdir = determineLogDir(results["mode"], qhgdir, results["datadir"], results["logdir"])
                print("Have logdir [%s]" % (logdir))
            else:
                if (not results["qhgdir"] is None) and (not results["logdir"]  is None):
                    qhgdir = results["qhgdir"]
                    logdir = results["logdir"]
                else:
                    raise ParamError("Either -m  or both -q and -l must be specified")
                #-- end if
            #-- end of

            actions  = splitActions(results["actions"])
            analyses = splitAnalyses(results["analyses"])
            
            pcoc = PCOCreator.PCOCreator(qhgdir,
                                         results["datadir"],
                                         results["pop_pat"],
                                         results["stat_pat"],
                                         int(results["zipped"]),
                                         logdir,
                                         results["times"],
                                         results["outdir"],
                                         results["outname"],
                                         results["actions"],
                                         cloudstyle)

                
            pcoc.createOuputDir()
            
            print("PCOCreator thinks zipped is " + str(pcoc.zipped))
            if pcoc.zipped:
                pcoc.gunzipFiles()
            #-- end if

            print("*** have actions:"+str(actions))
            
            for seq_type in actions:
                print("methods for %s: %s" % (seq_type, str(actions[seq_type])))
                sout =  results["outname"]
                if sout[-1] == '#':
                    pcoc.set_outname(sout.replace('#', ("_%s")%(seq_type,)))
                else:
                    pcoc.set_outname(sout)
                #- end if    
                topbody = pcoc.outbody


                pcoc.doQDFSampler(seq_type, results["in_samp"], results["block_range"])
                    
                pcoc.doBinSeqMerge2(seq_type)
                pcoc.doAscSeqMerge2(seq_type)

                for analysis in analyses[seq_type]:
                    if analysis == 'pco':
                        for dist_meth in actions[seq_type]:
                            print("doing method [%s]" % (dist_meth,))
                            inbody = topbody+".all.bin"
                            sout =  results["outname"]
                            if sout[-1] == '#':
                                pcoc.set_outname(sout.replace('#', ("_%s_%s")%(seq_type,dist_meth)))
                            else:
                                pcoc.set_outname(sout)
                            #- end if    
                            
                            pcoc.doSeqDist(seq_type, dist_meth, inbody, pcoc.outbody)
                                
                            pcoc.doCalcPCO(pcoc.outbody)
                
                            pcoc.doPCOHeaders(analysis)
                                
                            pcoc.doColumnMerger_s(analysis)
                        #-- end for
                    elif  (analysis == 'pca-cov') or (analysis == 'pca-cor'):
                        sout =  results["outname"]
                        if sout[-1] == '#':
                            pcoc.set_outname(sout.replace('#', ("_%s_%s")%(seq_type,"hamm")))
                        else:
                            pcoc.set_outname(sout)  
                            #- end if
                            
                        #pcoc.doCalcPCA(analysis, pcoc.outbody)
                        pcoc.doCalcPCA(analysis, topbody, pcoc.outbody)
                        inbody = topbody+".all.bin"
                        pcoc.doNullDist( inbody, pcoc.outbody)

                        pcoc.doPCOHeaders(analysis)
                                
                        pcoc.doColumnMerger_s(analysis)
                    else:
                        raise QHGError("main", "unknown anlysis [%s] (actually should not happen" % (analysis))
                    #-- end if

                #-- end for
            #-- end for
                
            sout =  results["outname"]
            if sout[-1] == '#':
                pcoc.set_outname(sout.replace('#', ""))
            else:
                pcoc.set_outname(sout)
            #- end if    

            pcoc.doCopyLogs()

            pcoc.doAttributes()
            
            pcoc.doArrivals()
                
            #if pcoc.zipped:
            pcoc.clean_up(results["clean_type"])
            #-- end if
            
            logname_c = pcoc.logname
            outdir_c  =  pcoc.outdir
            del pcoc
            copyPCOLog(logname_c, results, outdir_c)
        #-- end if    
                
    except QHGError as e:
        print("\n%sQHGError: %s%s" % (COL_RED, e, COL_OFF))
    except ParamError as e:
        print("\n%sParamError: %s%s" % (COL_RED, e, COL_OFF))
        print(COL_BLUE)
        usage()
        print(COL_OFF);
#    except  Exception as e:
#        print("Exception: %s" % (e))
#        raise(e)
    #-- end try

#-- end if
        
