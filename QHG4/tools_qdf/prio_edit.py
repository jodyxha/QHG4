#!/usr/bin/python

from sys import argv
import h5py
import numpy
from QHGError import QHGError

#--------------------------------------------------------------------------
#-- listify
#--
def listify(map, dt):
    
    out = numpy.array([(m, int(map[m])) for m in map], dt)
    #for m in map:
    #    out.append((m, map[m]))
    #-- end for
    return out
#-- end def


#-----------------------------------------------------------------------------
#-- PrioEditor
#--
class PrioEditor:

    #--------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdf_file, species):
        try:
            self.qdf_file = qdf_file
            self.hfile    = h5py.File(qdf_file)
            if not species:
                species = self.find_first_spc()
                print("no species provided - using species [%s]"%species)
            #-- end if
            self.hgroup   = self.hfile["/Populations/"+species]
            self.hattrs   = self.hgroup.attrs
            temp    = self.hattrs.get("PrioInfo", None)
            if temp is None:
                self.prios = None
            else:
                self.dt = temp.dtype
                self.prios = dict(temp)
        #-- end if
        except Exception as e:
            raise QHGError("couldn't  initialize: %s\n" % e)
        #-- end try
    #-- end def


    #--------------------------------------------------------------------------
    #-- find_first_spc
    #--
    def find_first_spc(self):
        p = self.hfile["/Populations"]
        k = list(p.keys())
        if len(k) == 0:
            raise QHGError("no species found")
        #-- end if
        return k[0]
    #-- end def
    
    #--------------------------------------------------------------------------
    #-- list
    #--
    def list_prios(self):
        if (not self.prios is None):
            #print(self.prios)
            maxL = 0
            
            for key in self.prios:
                s = key.decode('utf-8')        
                maxL = max(maxL, len(s))
            #-- end for
        
            print("%s  %s"%("name".ljust(maxL+2), "priority"))
            print("-"*(maxL+2+8))
            for key in self.prios:
                print("%s  %3d"%(key.decode('utf-8').ljust(maxL+2), self.prios[key]))
            #-- end for
        else:
            print("No PrioInfo attr found")
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- delete_prio
    #--
    def delete_prio(self, name):
        if (not self.prios is None):
            k = self.prios
            name_b=name.encode()
            if name_b in k:
                del k[name_b]
                
                del self.hgroup.attrs['PrioInfo']
            
                self.hgroup.attrs.create('PrioInfo', listify(k, self.dt))
            else:
                print("unknown key: [%s]" % name)
            #-- end if
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- add_prio
    #--
    def add_prio(self, name, prio):
        #print(self.hattrs.get("PrioInfo"))
        #print(self.prios)
        if (not self.prios is None):
            k = self.prios
            name_b=name.encode()
            if not name_b in k:
                k[name_b] = prio

                del self.hgroup.attrs['PrioInfo']
                
                self.hgroup.attrs.create('PrioInfo', listify(k, self.dt))
            else:
                print("there already is a prio with name [%s]" % name)
            #-- end if
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- rename_prio
    #--
    def rename_prio(self, name, new_name):
        #print(self.hattrs.get("PrioInfo"))
        #print(self.prios)
        if (not self.prios is None):
            k = self.prios
            name_b=name.encode()
            if name_b in k:
                p = k[name_b]

                del k[name_b]
                name2_b = new_name.encode()
                k[name2_b] = p

                del self.hgroup.attrs['PrioInfo']
                
                self.hgroup.attrs.create('PrioInfo', listify(k, self.dt))
            else:
                print("there already is a prio with name [%s]" % name)
            #-- end if
        #-- end if
    #-- end def

    
    #--------------------------------------------------------------------------
    #-- change_prio
    #--
    def change_prio(self, name, new_prio):
        #print(self.hattrs.get("PrioInfo"))
        #print(self.prios)
        if (not self.prios is None):
            k = self.prios
            name_b=name.encode()
            if name_b in k:
                k[name_b] = new_prio

                del self.hgroup.attrs['PrioInfo']
                
                self.hgroup.attrs.create('PrioInfo', listify(k, self.dt))
            else:
                print("there already is a prio with name [%s]" % name)
            #-- end if
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- sort_prios
    #--
    def sort_prios(self):
        #print(self.hattrs.get("PrioInfo"))
        #print(self.prios)
        if (not self.prios is None):
           
            k0=[(x,self.prios[x]) for x in self.prios]
            k1 = sorted(k0,  key=lambda x: x[1])
            
            del self.hgroup.attrs['PrioInfo']
            self.hgroup.attrs.create('PrioInfo',  numpy.array(k1, self.dt))
           
        #-- end if
    #-- end def


    #--------------------------------------------------------------------------
    #-- save
    #--
    def save(self):
        self.hfile.flush()
        self.hfile.close()
    #-- end def

#-- end class                  


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':

    bOK =True
    if (len(argv) > 2):
        try:

            command = argv[1]
            a = argv[2].split(':')
            if (len(a) > 1) :
                qdf_file = a[0] 
                species  = a[1]
            else:
                qdf_file = argv[2] 
                species = ""
            #-- end if

            pe = PrioEditor(qdf_file, species)

            if (command == 'list'):
                pe.list_prios()
            elif (command == 'sort'):
                pe.sort_prios()
            else:
                if (len(argv) > 3):
                    prio_name = argv[3]
                    if (command == 'del'):
                        pe.delete_prio(prio_name)
                    else:
                        print("%d args: %s"%(len(argv),argv))
                        if (len(argv) > 4):
                            if (command == 'add'):
                                pe.add_prio(prio_name, argv[4])
                            elif (command == 'rename'):
                                pe.rename_prio(prio_name, argv[4])
                            elif (command == 'change'):
                                pe.change_prio(prio_name, argv[4])
                            else:
                                print("unknown command")
                                bOK = False
                            #-- end if
                        else:
                            print("not enough parameters provided 1")
                            bOK = False
                        #-- end if
                    #-- end if
                else:
                    print("not enough parameters provided 2")
                    bOK = False
                #-- end if
                if bOK:
                    pe.save()    
            #-- end if
        except Exception as e:
            print(e)
        #-- end try
    else:
        print("not enough parameters provided 3")
        bOK = False
    #-- end if

    if not bOK:
        print("%s -- Priorities editor" % argv[0])
        print("Usage:")
        print("  %s list   <qdf_file>[:<species>]"%argv[0])
        print("  %s del    <qdf_file>[:<species>] <prio_name>"%argv[0])
        print("  %s add    <qdf_file>[:<species>] <prio_name> <prio_val>"%argv[0])
        print("  %s rename <qdf_file>[:<species>] <prio_name> <prio_new_name>"%argv[0])
        print("  %s change <qdf_file>[:<species>] <prio_name> <prio_new_val>"%argv[0])
        print("  %s sort   <qdf_file>[:<species>]"%argv[0])
    #-- end if
#-- end main
