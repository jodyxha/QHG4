#!/usr/bin/python

from sys import argv, stderr, exit
import numpy as np
import h5py
from QHGError import QHGError

POP_GROUP_NAME = 'Populations'

#-----------------------------------------------------------------------------
#-- np.dtype kinds and names used in doChange()
#--
typenames={'b':	'boolean',
           'i':	'(signed) integer',
           'u': 'unsigned integer',
           'f': 'floating-point',
           'c': 'complex-floating point',
           'm': 'timedelta',
           'M': 'datetime',
           'O': '(Python) object',
           'S': '(byte-)string',
           'a': '(byte-)string',
           'U': 'Unicode',
           'V': 'raw data (void)'}



#-----------------------------------------------------------------------------
#--  get_QDF_version 
#--    get QDF version ( v>=4: attrs grouped by actions)
#--
def get_QDF_version(hf, pop_name):
    qdf_ver = -1 
    if POP_GROUP_NAME in list(hf.keys()):
        gp =  hf[POP_GROUP_NAME]
        if pop_name in gp:
            gs=gp[pop_name]
            if "QDFVersion" in gs.attrs:
                qdf_ver = int(gs.attrs["QDFVersion"][0])
            else:
                qdf_ver = 3  # 'old' version
            #-- end if
        else:
            qdf_ver = -2 # no <pop_name> subgroup
        #-- end if
    else:
        qdf_ver = -1 # no  'Populations' group
    #-- end if
    return qdf_ver;
#--end def

 
#-----------------------------------------------------------------------------
#--  get_pop_names 
#--    get population names
#--
def get_pop_names(hf):
    pop_list = []
    if POP_GROUP_NAME in list(hf.keys()):
        gp= hf[POP_GROUP_NAME]
        pop_list = list(gp.keys())
    #-- end if
    return pop_list
#-- end def


#-----------------------------------------------------------------------------
#--  get_attrs 
#--    get attributes for a specific population
#--    if no population name is given, the first population is used
#--
def get_attrs(hf, pop_name):
    attr = None
    if POP_GROUP_NAME in list(hf.keys()):
        gp= hf[POP_GROUP_NAME]
        if (len(list(gp.keys())) > 0):
            if pop_name == '':
                attr=gp[list(gp.keys())[0]].attrs
            else:
                if pop_name in list(gp.keys()):
                    attr=gp[pop_name].attrs
                else:
                    raise QHGError("[get_attrs] No population ["+pop_name+"] found")
                #-- end if               
        else:
            raise QHGError("[get_attrs] No population subgroups found")
        #-- end if
    else:
        raise QHGError("[get_attrs] No group ['%s'] found" % POP_GROUP_NAME)
    #-- end if 
    return attr
#-- end def


#-----------------------------------------------------------------------------
#--  xxx_get_attrs4 
#--    get attributes for a specific population
#--    if no population name is given, the first population is used
#--
def get_attrs4(hf, pop_name):
    attr = {}
    if POP_GROUP_NAME in list(hf.keys()):
        gp= hf[POP_GROUP_NAME]
        if (len(list(gp.keys())) > 0):
            if pop_name == '':
                hs = gp[list(gp.keys())[0]]
            else:
                if pop_name in list(gp.keys()):
                    hs = gp[pop_name]
                else:
                    raise QHGError("[get_attrs] No population ["+pop_name+"] found")
                #-- end if
            attr = {**hs.attrs}
          
            for x in list(hs):
                if isinstance(hs[x], h5py.Group):
                    attr = { **attr, **(hs[x].attrs)}
                #-- end if
            #-- end for
        else:
            raise QHGError("[get_attrs] No population subgroups found")
        #-- end if
    else:
        raise QHGError("[get_attrs] No group ['%s'] found" % POP_GROUP_NAME)
    #-- end if 
    return attr
#-- end def


#-----------------------------------------------------------------------------
#--  get_num_agents 
#--    get attributes for a specific population
#--    if no population name is given, the first population is used
#--
def get_num_agents(hf, pop_name):
    num = -1
    if POP_GROUP_NAME in list(hf.keys()):
        gp= hf[POP_GROUP_NAME]
        if (len(list(gp.keys())) > 0):
            if pop_name == '':
                pop = gp[list(gp.keys())[0]]
            else:
                if pop_name in list(gp.keys()):
                    pop = gp[pop_name]
                else:
                    raise QHGError("[get_num_agents] No population [%s] found" %(pop_name))
                #-- end if
            #-- end if
            if ('AgentDataSet' in pop):
                num = len(pop['AgentDataSet'])
            else:
                raise QHGError("[get_num_agents] No dataset 'AgentDataSet' found in [%s]" % (pop_name))
        else:
            raise QHGError("[get_num_agents] No population subgroups found")
        #-- end if
    else:
        raise QHGError("[get_num_agents] No group [POP_GROUP_NAME] found")
    #-- end if 
    return num
#-- end def


#-----------------------------------------------------------------------------
#-- is_numeric_attr
#--
def is_numeric_attr(attr):
    k = attr.dtype.kind
    return k in  ['b', 'i', 'u', 'f']
#-- end def


#-----------------------------------------------------------------------------
#-- cast_numeric
#--   convert a string into a number of the desired type
#--   b : bool
#--   i : integer
#--   u : unsigned
#--   f : float
#--
def cast_numeric(valstr, kind):
    valnum = None
    try:
        if kind == 'b':
            valnum = valstr.lower() in ("yes", "true", "t", "1")
        elif kind == 'i':
            valnum = int(valstr)
        elif kind == 'u':
            v = int(valstr)
            if (v > 0):
                valnum = v
            #-- end if
        elif kind == 'f':
            valnum = float(valstr)
        else:
            raise QHGError("[cast_numeric] Unknown type char ["+kind+"]")
        #-- end of
    except:
        pass # print("Conversion failed")
    #-- end try
    return valnum
#-- end def


#-----------------------------------------------------------------------------
#-- change_attr
#--
def change_attr(attrs, attr_names, attr_vals):
    #print("names: %s"%attr_names)
    #print("vals:  %s"%attr_vals)
    #print("attrs:  %s"%dict(attrs))
    if (len(attr_names) ==len(attr_vals)):
        for i in range(len(attr_names)):
            #print("Using attr_names[%d]:%s"%(i, attr_names[i]))
            vprev = attrs.get(attr_names[i])
            if (not vprev is None):
                # attribute must be numeric
                if (is_numeric_attr(vprev)):
                    # try to cast to vprev's general type
                    new_valc = cast_numeric(attr_vals[i], vprev.dtype.kind)
                    if (not new_valc is None):
                        new_valarr = np.array([new_valc])
                        # make sure types match
                        if (new_valarr.dtype.kind == vprev.dtype.kind):
                            attrs.modify(attr_names[i], new_valarr)
                        else:
                            if (new_valarr.dtype.kind == 'i') and (vprev.dtype.kind == 'u'):
                                attrs.modify(attr_names[i], new_valarr)
                            else:
                                raise QHGError("[change_attr] attribute ["+attr_names[i]+"] requires a value of type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+"), and not '"+new_valarr.dtype.kind+"'")
                            #-- end if
                        #-- end if
                    else:
                        raise QHGError("[change_attr] Couldn't cast ["+attr_vals[i]+"] to type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+")")
                    #-- end if
                else:
                    attrs.modify(attr_names[i], [attr_vals[i]])
                    #raise QHGError("[change_attr] attribute ["+attr_names[i]+"] is not numeric (type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+")")
                #-- end if
            else:
                raise QHGError("[change_attr] No attribute ["+attr_names[i]+"] found in population group")
            #-- end if
        #-- end for
        #print("successfully changed %d attribute%s"%(len(attr_names), "" if (len(attr_names)==1) else "s"))

    else:
        raise QHGError("[change_attr] Number of attribute names and values differ")
    #-- end if
#-- end def


#-----------------------------------------------------------------------------
#-- del_attr
#--
def del_attr(attrs, attr_names):

    for att in attr_names:
        vprev = attrs.get(att)
        if (not vprev is None):
            del attrs[att]
        else:
            raise QHGError("[del_attr] No attribute ["+att+"] found in attributes")
            #raise QHGError("[del_attr] No attribute ["+att+"] found in group ["+pop_name+"]")
        #-- end if
    #-- end for
    #print("successfully deleted %d attribute%s"%(len(attr_names), "" if (len(attr_names)==1) else "s"))

#-- end def


#-----------------------------------------------------------------------------
#-- add_attr
#--
def add_attr(attrs, attr_name, attr_val, val_type):
    val = np.array([attr_val],dtype=val_type)
    attrs.create(attr_name, val)
    print("successfully added attribute [%s]"%(attr_name) )

#-- end def


#-----------------------------------------------------------------------------
#-- show_attrs
#--
def show_attrs(attrs, attr_names, bAll, num):
    
    aa0={}
    aa1={}
    
    l0=0
    l1=0
    for x in attrs:
        if (type(attrs[x])==np.ndarray) and (len(attrs[x]) > 1):
            aa1[x]=attrs[x]
            if len(x) > l1:
                l1 = len(x)
            #-- end if
        else:   
            aa0[x]=attrs[x]
            if len(x) > l0:
                l0 = len(x)
            #-- end if
        #-- end if
                    
    #-- end for
    found = {}
    if (not attr_names):
        if (num > 0):
            print("#Agents".ljust(l0)+"\t[%d]"%num)
        #-- end if
        for n in sorted(aa0):
            print(n.ljust(l0)+"\t"+str(aa0[n]))
            
        #-- end for
    else:
        for attr in attr_names:
            if (attr in aa0):
                print(attr.ljust(l0)+"\t"+str(aa0[attr]))
                if attr not in found:
                    found[attr] = True
                #-- end if 
            else:
                if attr not in found:
                    found[attr] = False
                #-- end if 
            #-- end if
        #-- end for
    #-- end if
    
    if (bAll):
        if (not attr_names):
            for n in sorted(aa1):
                print(n.ljust(l1)+"\t"+str(aa1[n]).replace("\n", " "))
            #-- end for
        else:
            for attr in attr_names:
                if (attr in aa1):
                    print(attr.ljust(l0)+"\t"+str(aa1[attr]))
                    if (attr not in found) or (not found[attr])  :
                        found[attr] = True
                    #-- end if 
                else:
                    if attr not in found:
                        found[attr] = False
                    #-- end if 
                #-- end if 
            #-- end for 
        #-- end if
    #-- end if
    for key in found:
        if (not found[key]):
            print(key.ljust(l0)+"\t"+"<-- does not exist -->")
        #-- end if
    #-- end for
#-- end def

