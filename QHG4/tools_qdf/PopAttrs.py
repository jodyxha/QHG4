
import numpy
import h5py
from   QHGError import QHGError
import attr_tools

POP_GROUP_NAME = 'Populations'
DEF_GROUP_V3   = 'all'

#-----------------------------------------------------------------------------
#-- PopAttrs
#--
#-- tools for viewing, adding, modifiyng and deleting population attributes
#--
class PopAttrs:


    #-----------------------------------------------------------------------------
    #-- constructor
    #--    
    def __init__(self, qdf_file):
        self.qdf_file = qdf_file

        # all_attrs is a map of maps of maps:
        #    pop_names=> [qdf_ver, {group_name => {attr:name => attr_val}}]
        self.all_attrs   = {}
       
        self.pop_names   = []
        self.hf = None
        self.iNumAgents = 0
        try:
            self.hf = h5py.File(qdf_file, 'r+')
            self.pop_names = attr_tools.get_pop_names(self.hf)
            self.read_all_attrs()

            #@@TODO remove this - it's not nice
            #for p in self.all_attrs:
            #    self.all_attrs[p]['global'] = { **self.all_attrs[p]['global'], "file":[qdf_file]}
            #-- end for
        except Exception as e:
            raise  QHGError(e)
        #-- end try
    #-- end def


    #-----------------------------------------------------------------------------
    #--  flush
    #--    
    #--
    def flush(self):
        self.hf.flush()
    #-- end def


    #-----------------------------------------------------------------------------
    #--  close
    #--    
    #--
    def close(self):
        self.hf.close()
    #-- end def


    #-----------------------------------------------------------------------------
    #--  get_num_agents
    #--    
    #--
    def get_num_agents(self, pop_name):
        return attr_tools.get_num_agents(self.hf,  pop_name)
    #-- end def


    #-----------------------------------------------------------------------------
    #--  get_pop_names
    #--    
    #--
    def get_pop_names(self):
        return attr_tools.get_pop_names(self.hf)
    #-- end def

    
 
    #-----------------------------------------------------------------------------
    #--  read_pop_attrs
    #--    
    #--
    def read_pop_attrs(self, pop_name):
        self.read_all_attrs()
        if (pop_name == ''):
            pop_name = self.pop_names[0]
        #-- end if
        if (pop_name in list(self.all_attrs.keys())):
            return self.all_attrs[pop_name]
        else:
            raise QHGError("No [%s] not found in populations" % pop_name)
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #--  read_all_attrs
    #--    
    #--
    def read_all_attrs(self):
        for p in self.pop_names:

            cur_qdf_version =  attr_tools.get_QDF_version(self.hf, p)

            if (cur_qdf_version < 0):
                raise QHGError("Couldn't get QDF version for ['%p']" % p)
            else:
                attrs4 = self.read_attrs4(self.hf, p) 
                self.all_attrs[p] = attrs4
            #-- end if
        #-- end for
    #-- end def




    #-----------------------------------------------------------------------------
    #--  read_attrs4
    #--    
    #--
    def read_attrs4(self, hf, pop_name):
        attr4 = {}
        if POP_GROUP_NAME in list(hf.keys()):
            pop_group = hf[POP_GROUP_NAME]
            if (len(list(pop_group.keys())) > 0):
                if pop_name == '':
                    spc_group = pop_group[list(pop_group.keys())[0]]
                else:
                    if pop_name in list(pop_group.keys()):
                        spc_group = pop_group[pop_name]
                    else:
                        raise QHGError("[get_attrs] No population ["+pop_name+"] found")
                    #-- end if
                #-- end if
               
                attr4['global'] = spc_group.attrs
                if "PrioInfo" in  attr4['global']:
                    del attr4['global']["PrioInfo"]
                if "QDFVersion" in  attr4['global']:
                    del attr4['global']["QDFVersion"]

                for link in list(spc_group):
                    if isinstance(spc_group[link], h5py.Group):
                        #attr4 = { **attr4, **(spc_group[link].attrs)}
                        attr4[link] = spc_group[link].attrs
                    #-- end if
                #-- end for
            else:
                raise QHGError("[get_attrs] No population subgroups found")
            #-- end if
        else:
            raise QHGError("[get_attrs] No group ['%s'] found" % POP_GROUP_NAME)
        #-- end if 
        return attr4
    #-- end def

 

    #-----------------------------------------------------------------------------
    #--  listify_attrs
    #--    
    #--
    def listify_attrs(self, hattrs):
        new_attr = {}
        temp_attr = dict(hattrs)
        #print(temp_attr)
        
        for attr_name in temp_attr:
            new_val = []
            for v in temp_attr[attr_name]:
                if (isinstance(v, numpy.bytes_)):
                    new_val.append(v.decode('utf-8'))
                elif (isinstance(v, numpy.ndarray)):
                    # listify numpy arrays
                    new_val.append(list(v))
                else:
                    new_val.append(v)
                #-- end if
            #-- end for
            new_attr[attr_name] = new_val
        #-- end for
                    
        return new_attr
    #-- end def


    # probably unused
    #-----------------------------------------------------------------------------
    #--  get_attrs
    #--
    #def get_attrs(self, pop_name):
    #    
    #    if pop_name in self.all_attrs:
    #        clean_attrs = {}
    #        for g in self.all_attrs[pop_name][1]:
    #            listify_attrs[g] = self.listify_attrs(self.all_attrs[pop_name][1][g])
    #        #-- end for
    #        return clean_attrs
    #    else:
    #        raise QHGError("Name [%s] does not exist"%pop_name)
    #    #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #--  get_all_attrs (used by comp_attr.py)
    #--
    def get_all_attrs(self, pop_list):
        pop_attrs = {}
        for pop in pop_list:
            pop_attrs = { **pop_attrs, pop:self.all_attrs[pop]}

        #-- end for
        return pop_attrs
    #-- end def

    #-----------------------------------------------------------------------------
    #--  get_flat_attrs (used by comp_attr.py)
    #--
    def get_flat_attrs(self, pop_name):
        if pop_name in self.all_attrs:
            flatatt = {}
            for x in self.all_attrs[pop_name]:
                y = self.listify_attrs(self.all_attrs[pop_name][x])
                flatatt = {**flatatt, **y}
            #-- end for
            return flatatt
        else:
            raise QHGError("Name [%s] does not exist"%pop_name)
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #--  del_attrs
    #--
    def del_attrs(self, pop_name, attr_names):
        if pop_name == "":
            pop_name = self.pop_names[0]
            print("using population %s"%pop_name)
        #-- end if
        if pop_name in self.all_attrs:
            self.del_attrs4(self.all_attrs[pop_name], attr_names)
            print("successfully deleted %d attribute%s"%(len(attr_names), "" if (len(attr_names)==1) else "s"))
        else:
            raise QHGError("Name [%s] does not exist"%pop_name)
        #-- end if
    #-- end def
        

    #-----------------------------------------------------------------------------
    #--  del_attrs4
    #--    attr_names list of 'groupname/attname' or list of 'attname'
    #--    if no groupname is given we try to find the *unique* group containing
    #--    an attribute with this name. If no such group: fail
    #--
    def del_attrs4(self, attrs, attr_names):
        print("--- del_attrs4 ---")
        for  an in attr_names:
            aan = an.split('/')
            if len(aan) == 2:
                group   = aan[0]
                attname = aan[1]
                if group in attrs:
                    g_attrs = attrs[group]
                    attr_tools.del_attr(g_attrs, [attname])
                else:
                    raise QHGError("No group [%s] in attrs[%s]"%(group, dict(attrs)))
                #-- end if
            else:
                # find unique group:
                # collect all groups with an attribute of the given name
                attname = aan[0]
                cands = []
                for group in attrs:
                    if attname in attrs[group]:
                        cands.append(group)
                    #-- end if
                #-- end for
                
                # if there is only one candidate, use it
                if len(cands) == 1:
                    print("removing [%s] from unique group [%s]"%(attname, cands[0]))
                    g_attrs = attrs[cands[0]]
                    attr_tools.del_attr(g_attrs, [attname])
                else:
                    raise QHGError("No unique group found for attribute [%s]"%attname)
                #-- end if
            #-- end if
        #-- end for
        #print("successfully deleted %d attribute%s"%(len(attr_names), "" if (len(attr_names)==1) else "s"))
    #-- end def

      
    #-----------------------------------------------------------------------------
    #-- change_attrs
    #--
    def change_attrs(self, pop_name, attr_names, attr_vals):
        if pop_name == '':
            pop_name = self.pop_names[0]
            print("using population %s"%pop_name)
        #-- end if
        if pop_name in self.all_attrs:
            self.change_attrs4(self.all_attrs[pop_name], attr_names, attr_vals)
            print("successfully changed %d attribute%s: %s"%(len(attr_names), "" if (len(attr_names)==1) else "s", attr_names))
        else:
            raise QHGError("Name [%s] does not exist"%pop_name)
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #--  change_attrs4
    #--    attr_names list of 'groupname/attname' or list of 'attname'
    #--    if no groupname is given we try to find the *unique* group containing
    #--    an attribute with this name. If no such group: fail
    #--
    def change_attrs4(self, attrs, attr_names, attr_vals):
        print("--- change_attrs4 ---")
        #print("names: %s"%attr_names)
        #print("vals:  %s"%attr_vals)
        #print("attrs:  %s"%dict(attrs))

        if (len(attr_names) ==len(attr_vals)):
            for i in range(len(attr_names)):
                groupatts = attr_names[i].split('/')
                if len(groupatts) == 2:
                    group   = groupatts[0]
                    attname = groupatts[1]
                    if group in attrs:
                        #print("changing [%s] of group [%s]"%(attname,group))
                        g_attrs = attrs[group]
                        attr_tools.change_attr(g_attrs, [attname], [attr_vals[i]])
                    else:
                        raise QHGError("No group [%s] in attrs[%s]"%(group, dict(attrs)))
                    #-- end if
                else:
                    #TODO: find unique group
                    attname = groupatts[0]
                    cands = []
                    for group in attrs:
                        #print("attrs[%s]:  %s"%(group, dict(attrs[group])))
                        if attname in list(attrs[group].keys()):
                            cands.append(group)
                        #-- end if
                    #-- end for
                    if len(cands) == 1:
                        print("changing [%s] of unique group [%s]"%(attname, cands[0]))
                        g_attrs = attrs[cands[0]]
                        attr_tools.change_attr(g_attrs, [attname], [attr_vals[i]])
                    else:
                        raise QHGError("No unique group found for attribute [%s]"%attname)
                    #-- end if
                #-- end if
            #-- end if
            #print("successfully changed %d attribute%s: %s"%(len(attr_names), "" if (len(attr_names)==1) else "s", attr_names))
        else:
            raise QHGError("[change_attr] Number of attribute names and values differ")
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- add_attr
    #--
    def add_attr(self, pop_name, attr_name, attr_val, val_type):
        if pop_name == "":
            pop_name = self.pop_names[0]
            print("using population %s"%pop_name)
        #-- end if

        if pop_name in self.all_attrs:
            self.add_attr4(self.all_attrs[pop_name], attr_name, attr_val, val_type)
            print("successfully added attribute [%s]"%(attr_name))
        else:
            raise QHGError("Name [%s] does not exist"%pop_name)
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- add_attr4
    #--
    def add_attr4(self, attrs, attr_name, attr_val, val_type):
        aan = attr_name.split('/')
        if len(aan) == 2:
            group   = aan[0]
            attname = aan[1]
            if group in attrs:
                g_attrs = attrs[group]
                attr_tools.add_attr(g_attrs, attname, attr_val, val_type)
            else:
                raise QHGError("No group [%s] in attrs[%s]"%(group, dict(attrs)))
            #-- end if
        else:
            raise QHGError("v4 populations require 'subgroup/attr_name'")
        #-- end if
        #print("successfully added attribute [%s]"%(attname))
    #-- end def

    
    #-----------------------------------------------------------------------------
    #--  show_all
    #--
    def show_all(self):
        for pop_name in self.all_attrs:
            print("species [%s]"%(pop_name))

            for g in self.all_attrs[pop_name]:
                print("--group [%s]----------------"%g)
                ag =  self.all_attrs[pop_name][g]
                for a in ag:
                    print("    %s -> %s"%(a, ag[a]))
                #-- end for
            #-- end for
    #-- end def
        
     
    #-----------------------------------------------------------------------------
    #--  list_attrs
    #--
    def list_attrs(self):
        for p in self.all_attrs:
            print("species [%s] (v%d)"%(p))
            for g in self.all_attrs[p]:
                ag =  self.all_attrs[p][g]
                for a in ag:
                    print("    %s/%s [%s]"%(g, a, type(ag[a][0])))
                #-- end for
            #-- end for
        #-- end for
    #-- end def
 
    
