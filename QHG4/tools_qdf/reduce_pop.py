#!/usr/bin/python

from sys import argv
import h5py
import numpy
import shutil
import time
import random
from QHGError import QHGError

NAME_POP = "Populations"
NAME_AGS = "AgentDataSet"
NAME_PHE = "Phenome"
NAME_GEN = "Genome"


#-----------------------------------------------------------------------------
#-- QDFReducer
#--
class QDFReducer:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdf_file, pop_name):
        try:
            self.f = h5py.File(qdf_file, "r+")
        except Exception as e:
            raise QHGError(e)
        #--end try

        
        if NAME_POP in self.f:
            self.popgroup = self.f[NAME_POP]
            if (not pop_name) :
                pop_name = list(self.popgroup.keys())[0]
            #-- end if
            
            if pop_name in self.popgroup:
                self.pop = self.popgroup[pop_name]
                if NAME_AGS in self.pop:
                    self.ags =self.pop[NAME_AGS]
                
                else:
                    raise QHGError("No agent dataset [%s] in ppop group"%NAME_AGS)
                #-- end if

                if NAME_GEN in self.pop:
                    self.gen =self.pop[NAME_GEN]
                    self.nge = int(len(self.gen)/len(self.ags))
                else:
                    self.gen = None
                #-- end if

                if NAME_PHE in self.pop:
                    self.phe =self.pop[NAME_PHE]
                    self.nph = int(len(self.phe)/len(self.ags))
                else:
                    self.phe = None
                #-- end if

                self.ags2 = []
                self.gen2 = []
                self.phe2 = []
                
                self.save = None
                if (self.phe is None) and (self.gen is None):
                    self.save = self.save_ags
                elif (self.phe is None) and (not self.gen is None):
                    self.save = self.save_gen
                elif (not self.phe is None) and (self.gen is None):
                    self.save = self.save_phe
                else:
                    #                self.save = self.save_gen_phe
                    self.save = self.save_blarg
                #-- end if
            else:
                raise QHGError("Didn't find pop group [%s] in [%s]"% (pop_name, qdf_file))
            #-- end if
        else:
            raise QHGError("Didn't find group [%s] in [%s]"% (NAME_POP, qdf_file))
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- build_mask
    #-- 
    def build_mask(self, a, b):
        mask = []
        for i in range(0,b):
            mask.append(int(i*b/a)%b)
        #-- end for
        return set(mask)
    #-- end def


    #-----------------------------------------------------------------------------
    #-- save_ags
    #-- 
    def save_ags(self, i):
        self.ags2.append(self.ags[i])
    #-- end def


    #-----------------------------------------------------------------------------
    #-- save_gen
    #-- 
    def save_gen(self, i):
        self.ags2.append(self.ags[i])
        self.gen2.extend(self.gen[i*self.nge:(i+1)*self.nge])
    #-- end def


    #-----------------------------------------------------------------------------
    #-- save_phe
    #-- 
    def save_phe(self, i):
        self.ags2.append(self.ags[i])
        self.phe2.extend(self.phe[i*self.nph:(i+1)*self.nph])
    #-- end def


    #-----------------------------------------------------------------------------
    #-- save_gen_phe
    #-- 
    def save_gen_phe(self, i):
        self.ags2.append(self.ags[i])
        self.gen2.extend(self.gen[i*self.nge:(i+1)*self.nge])
        self.phe2.extend(self.phe[i*self.nph:(i+1)*self.nph])
    #-- end def
    

    #-----------------------------------------------------------------------------
    #-- save_blarg
    #-- 
    def save_blarg(self, i):
        self.ags2.append(self.ags[i])
        if not self.phe is None:
            self.phe2.extend(self.phe[i*self.nph:(i+1)*self.nph])
        #-- end if
        if not self.gen is None:
            self.gen2.extend(self.gen[i*self.nge:(i+1)*self.nge])
        #-- end if
    #-- end def
    

    #-----------------------------------------------------------------------------
    #-- decimate_kill_mod
    #-- 
    def decimate_kill_mod(self, p1, p2):
        mask = self.build_mask(p1, p2)
        for i in range(len(self.ags)):
            if  not (i%p2) in mask != 0:
                self.save(i)
            #-- end if
        #-- end for
    #-- end def

         
    #-----------------------------------------------------------------------------
    #-- decimate_save_mod
    #-- 
    def decimate_save_mod(self, p1, p2):
        mask = self.build_mask(p1, p2)
        for i in range(len(self.ags)):
            if (i%p2) in mask:
                self.save(i)
            #-- end if
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- decimate_rand
    #-- 
    def decimate_rand(self, prob):
        for i in range(len(self.ags)):
            if random.random() > prob:
                self.save(i)
            #-- end if
        #-- end for
    #-- end def
        

    #-----------------------------------------------------------------------------
    #-- save_changes
    #-- 
    def save_changes(self):
        del self.pop[NAME_AGS]
        self.pop[NAME_AGS] = numpy.array(self.ags2)

        if not self.phe is None:
            del self.pop[NAME_PHE]
            self.pop[NAME_PHE] = numpy.array(self.phe2)
        #-- end if
        
        if not self.gen is None:
            del self.pop[NAME_GEN]
            self.pop[NAME_GEN] = numpy.array(self.gen2)
        #-- end if

        self.f.flush()
        self.f.close()
    #-- end def


if len(argv) > 3:
    
    filepop  = argv[1]
    method   = argv[2]
    params   = argv[3].split('/')

    t0 = 0
    try:

        if len(params) == 2:
            param1 = int(params[0])
            param2 = int(params[1])
        else:
            raise QHGError("Expected expression of the form x/y")
        #-- end if
        
        parts = filepop.split(':')
        qdf_file = parts[0]
        if (len(parts) > 1):
            pop_name = parts[1]
        else:
            pop_name = ""
        #-- end if
    

        shutil.copyfile(qdf_file, qdf_file+".bak");
        
        t0 = time.clock()
        qd = QDFReducer(qdf_file, pop_name)

        if method == "kill_mod":
            qd.decimate_kill_mod(param1, param2)
        elif method == "save_mod":
            qd.decimate_save_mod(param1, param2)
        else:
            raise QHGError("unknown method:[%s]" % (method,))
        #-- end if
    except QHGError as qhge: 
        print("Error: [%s]" % (qhge))
    except ZeroDivisionError as zde:
        print("The parameter must be >= 1")
#    except Exception as e:
#        print("Error: [%s]" % (e))
    else:
        qd.save_changes()
    #-- end try
    t1 = time.clock()
    print("Used %f secs\n" % (t1-t0))
else:
    print("Reduce number of agents together with Phenomes and/or Genomes")
    print("Usage:")
    print("  %s <qdf-file>[:<pop-name>] <method> <param1>/<param2>" % argv[0])
    print("where")
    print("  qdffile   QDF file to modify")
    print("  popname   name of population's group. If omitted, the first population is used")
    print("  method    'kill_mod', 'save_mod'")
    print("  param1    integer number:")
    print("  param2    integer number:")
    print("methods:")
    print("    'kill_mod': kill <param1>/<param2> of all agents")
    print("    'save_mod': keep <param1>/<param2> of all agents")
    print("")
    print("A backup file named <qdfile>.bak wil be made")
#-- end if









