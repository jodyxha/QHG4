#!/usr/bin/python
from sys import stderr, argv, exit
import numpy as np
import h5py


import attr_tools
from QHGError import QHGError
import colors
import PopAttrs

#COL_RED = "\x1B[31m"
#COL_OFF = "\x1B[0m"

#varcols=[colors.OFF, colors.BGHIRED, colors.BGHIGREEN, colors.BGHIYELLOW, colors.BGHICYAN, colors.BGHIPURPLE, colors.BGCYAN, colors.BGYELLOW, colors.BGPURPLE, colors.BGHIBLUE]
varcols=[colors.OFF,
         colors.TBGHIRED,
         colors.TBGHIGREEN,
         colors.TBGHIYELLOW,
         colors.TBGHICYAN,
         colors.TBGHIPURPLE,
         colors.TBGCYAN,
         colors.TBGYELLOW,
         colors.TBGPURPLE,
         colors.TBGHIBLUE,
         colors.TBGRED,
         colors.TBGGREEN,
         colors.TBGBLUE,
         ]


HILITE_NONE         = 'none'
HILITE_SIMPLE       = 'simple'
HILITE_FILE_GROUP   = 'file_group'
HILITE_DIFF_GROUP   = 'diff_group'

hilites=[HILITE_NONE, HILITE_SIMPLE, HILITE_FILE_GROUP, HILITE_DIFF_GROUP]

SHORTNAME_NONE  = 'none'
SHORTNAME_PATH  = 'path'
SHORTNAME_ALIAS = 'alias'
shortnames=[SHORTNAME_NONE,  SHORTNAME_PATH, SHORTNAME_ALIAS]

alias_pattern = 'qdf_%02d'

paramshort = {
    'g': 'global',
    'a': 'action',
    'd': 'diffonly',
    'n': 'numagents',
    's': 'shortnames',
    'q': 'equalsize',
    'e': 'showempty',
    'h': 'hilite',
}
             
hiliteshort = {
    'n': HILITE_NONE,
    's': HILITE_SIMPLE,
    'f': HILITE_FILE_GROUP,
    'd': HILITE_DIFF_GROUP,
}

shortnameshort = {
    'n': SHORTNAME_NONE,
    'p': SHORTNAME_PATH,
    'a': SHORTNAME_ALIAS,
}



#-----------------------------------------------------------------------------
#-- AttrComparator
#--
class AttrComparator:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdfs):
        self.qdfs = qdfs
        self.hdf_attrs = {} #map filename =>( map pop => array of map (group_name)=>(hdf attribute))
        self.pop_group_attrs = {}#map (filename,sps) =>( array of map (group_name)=>(list[attr_name, atztr-val]))    #-- end def
        self.max_name_width = 0 #  max_name_width
        self.max_val_widths  = {} # map (filename, spc) => max_val_width
        self.full_attrs = {} # map group_name => 
        self.header_entries = []
        self.size_entries   = []
        self.global_group = None
        
    #-----------------------------------------------------------------------------
    #-- collect_attrs
    #--  collect all attributes of all groups of all files in a
    #--    map (filename => (map species => map (group_name =>(list[attr_name, attr-val])))
    #--
    def collect_attrs(self):

        self.header_entries = ['Attribute']
        self.size_entries = ['#Agents']

        for f0 in self.qdfs:
            singlepop = ''
            if f0.find(':') >= 0:
                f, singlepop = f0.split(':')
            else:
                f = f0
            #-- end if
            stderr.write("Opening %s [%s]\n" % (f, singlepop))
            pa = PopAttrs.PopAttrs(f)
            if (singlepop != ''):
                pop_list = [singlepop]
                #stderr.write("got list: %s\n" % pop_list)
            else:
                pop_list = pa.get_pop_names()
                if not pop_list:
                    raise QHGError("[collect_attrs] No [Populations] group in %s"%f)
                #else:
                    #stderr.write("got list: %s\n" % pop_list)
                #-- end if
            #--end if

            self.hdf_attrs[f] = pa.get_all_attrs(pop_list)
            
            for pop in pop_list:
                num = pa.get_num_agents(pop)
                self.size_entries.append(num)
                self.header_entries.append(f0)
            #-- end for
                            
            
        #-- end for
        print("[collect_entries] header: %s"%self.header_entries)
    #-- end def


    #-----------------------------------------------------------------------------
    #-- reshape_attrs
    #--   create a  map (file_name,spc_name) => (map group_name => [[att_name,att_val, ...])
    #--
    def reshape_attrs(self):
        stderr.write("[reshape_attrs]\n")

        for file_name in self.hdf_attrs:
            species = {}
            for spc_name in self.hdf_attrs[file_name]:
                groups = {}
                for group_name in self.hdf_attrs[file_name][spc_name]:
                    group_attrs = []
                    for a in self.hdf_attrs[file_name][spc_name][group_name]:
                        v = self.hdf_attrs[file_name][spc_name][group_name][a][0]
                        if isinstance(v,bytes):
                            v = v.decode()
                        else:
                            v = str(v)
                        #-- end if
                        group_attrs.append([a, v])
                    #-- end for
                    groups[group_name] = group_attrs
                #-- end for
                #species[spc_name] = groups
                self.pop_group_attrs[(file_name, spc_name)] = groups
            #-- end for
        #-- end for
    #-- end def


    #-------------------------------------------------------------------------
    #-- find_max_widths
    #--   find the lengths of the longest name and longest value
    #--
    def find_max_widths(self):
        name_width = 0
        max_name = ""

        if len(self.header_entries[0]) > name_width:
            name_width = len(self.header_entries[0])
        #-- end if

        if len(self.size_entries[0]) > name_width:
            name_width = len(self.size_entries[0])
        #-- end if
        
        for file_name in self.pop_group_attrs:
            val_width  = 0
            max_val = ""
            for group_name in self.pop_group_attrs[file_name]:
                for a in self.pop_group_attrs[file_name][group_name]:
                    if (len(a[0]) > name_width):
                        name_width =  len(a[0])
                        max_name = a[0]
                        stderr.write("  g %s: name_width: %s -> %d\n"%(group_name, max_name, name_width))
                    #-- end if
                    if (len(str(a[1])) > val_width):
                        val_width =  len(str(a[1]))
                        max_val = str(a[1])
                        stderr.write("  g %s: name_width: %s -> %d\n"%(group_name, max_val, val_width))
                    #-- end if
                #-- end for
            #-- end for


            if (len(self.header_entries[0]) > name_width):
                name_width = len(self.header_entries[0])
                stderr.write("  i %d: name_width: %s -> %d\n"%(i,self.header_entries[i], name_width))
            #-- end if
            for i in range(1, len(self.header_entries)):
                if (len(self.header_entries[i]) > val_width):
                    val_width = len(self.header_entries[i])
                    stderr.write("  i %d; val_width: %s -> %d\n"%(i,self.header_entries[i], val_width))
                #-- end if
            #-- end for

                
            #print("max name [%s], L %d"%(max_name, name_width))
            #print("max val  [%s], L %d"%(max_val, val_width))

            if (name_width > self.max_name_width):
                self.max_name_width = name_width

            if file_name in self.max_val_widths: 
                if (val_width >  self.max_val_widths[file_name]):
                    self.max_val_widths[file_name]  = val_width
                #-- end if
            else:
                self.max_val_widths[file_name]  = val_width
            #-- end if
        #-- end for

        #s=""
        #for f in self.pop_group_attrs:
        #    s = s + " " + str(self.max_val_width[f])
        #-- end for
        #print(s)
        stderr.write("[find_max_widths] max name width %d, max val widths %s\n"%(self.max_name_width,  self.max_val_widths))
    #-- end def
                          

    #-------------------------------------------------------------------------
    #-- create_full_attr_list
    #--   creates a map group_name =>  [[att_name,att_val, ...]
    #--   containing *all* groups and attributes
    #--
    def create_full_attr_list(self):
        self.full_attrs = {}
        #stderr.write("[create_full_attr_list] pop_group_attrs: %s\n"%self.pop_group_attrs)
        for file_name in self.pop_group_attrs:
            #print (file_name)
            for group_name in self.pop_group_attrs[file_name]:
                #print(self.full_attrs)
                if (not group_name in self.full_attrs):
                    self.full_attrs[group_name] = []
                #-- end if
                for a in self.pop_group_attrs[file_name][group_name]:
                    if (not a[0] in self.full_attrs[group_name]):
                        self.full_attrs[group_name].append(a[0])
                    #-- end if
                
                #-- end for
            #-- end for
        #-- end for
        #
        for group_name in self.full_attrs:
            #print("g %s"%group_name)
            #print("ag %s"%self.full_attrs[group_name])
            self.full_attrs[group_name] = sorted(self.full_attrs[group_name])
        #-- end if
     
    #-- end def


    #-------------------------------------------------------------------------
    #-- pad_groups
    #--
    def pad_groups(self, file_name):
        keys = [x for x in self.full_attrs]
        for group_name in sorted(keys):
            #print("checking group [%s]"%group_name)
            if (not group_name in self.pop_group_attrs[file_name]):
                #print("group does  not exist")
                h= []
                for a in self.full_attrs[group_name]:
                    #print("adding empty  [%s,'']"%a)
                    h.append([a, ""])
                #-- end for
                self.pop_group_attrs[file_name][group_name] = h
            else:
                for a in self.full_attrs[group_name]:
                    bsearching = True
                    for b in self.pop_group_attrs[file_name][group_name]:
                        if (a == b[0]):
                            bsearching = False
                            break;
                        #-- end if
                    #-- end for
                    if (bsearching):
                        #print("adding [%s,''] to  self.pop_group_attrs[%s]"%(a, group_name))
                        self.pop_group_attrs[file_name][group_name].append([a, ""])
                    #-- end if
                #-- end for
            #-- end if
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- group_files
    #--
    def group_files(self, row_args):
        cols = [-1]*len(row_args)
        arg1 = row_args[1].strip()
        for i in range(1, len(row_args)):
            #c = variant_vals.index(row_args[i].strip())
            if (arg1 != row_args[i].strip()):
                cols[i] = i
            else:
                cols[i] = 0
            #-- end if
        #-- end for
        return cols
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- group_diffs
    #--
    def group_diffs(self, row_args):
        cols = [0,0]

        first = row_args[1].strip()
        found_vals =[first]
        found_cols = [0]
        ref = -1
        for i in range(2,len(row_args)):
            cur_arg = row_args[i].strip()
            if cur_arg != first:
                if cur_arg in found_vals:

                    for j in range(len(found_vals)):
                        if (found_vals[j] == cur_arg):
                            ref = found_cols[j]
                            break;
                        #-- end if
                    #-- end for
                else:
                    ref = i
                    found_vals.append(cur_arg)
                    found_cols.append(ref)
                #-- end if
            else:
                ref = 0
            #- end if
            ref = ref if ref >= 0 else i
            cols.append(ref)
        #-- end for
        return cols
    #-- end def 


    #-----------------------------------------------------------------------------
    #-- group_cols_old
    #--
    def group_cols_old(self, row_args):
        cols = [-1]*len(row_args)
        curcol = 0
        for icol in range(0, len(row_args)):
            if cols[icol] < 0:
                cols[icol] = curcol
                curcol = curcol + 1
            #-- end if

            for icol2 in range(icol+1, len(row_args)):
                if row_args[icol].strip() ==  row_args[icol2].strip():
                    if   cols[icol2] < 0:
                        cols[icol2] = cols[icol]
                    #-- end if
                #-- end if
            #-- end for
        #-- end for
        return cols
    #-- end def
    
    
    #-------------------------------------------------------------------------
    #-- create separator
    #--
    def create_separator(self):
        s = "+-"
        #print(1+self.max_name_width)
        s = s+(1+self.max_name_width)*"-" + "+"
        for file_name in self.pop_group_attrs:
            #print(4+self.max_val_widths[file_name])
            s = s+(2+self.max_val_widths[file_name])*"-" + "+"
        # -- end for
        return s
    #-- end def
    
    #-------------------------------------------------------------------------
    #-- collect_equalities
    #--
    def collect_equalities(self, group_name):

        all_equal = True
        
        for index in range (len(self.full_attrs[group_name])):
            v_orig = "_null_"
            for file_name in self.pop_group_attrs:
                s1 = sorted(self.pop_group_attrs[file_name][group_name])[index][1]
                if v_orig == "_null_":
                    v_orig = s1
                else:
                    if v_orig != s1:
                        
                        all_equal = False
                    #-- end if
                #-- end if
            #-- end for
        #-- end for
        return all_equal
    #-- end def
    
    
    #-------------------------------------------------------------------------
    #-- create_att_line
    #--
    def create_att_line(self, group, index, hilite_state, diff_only):
        v_orig = "_null_"

        s0 =  self.full_attrs[group][index]
        s0 = s0.ljust(self.max_name_width)
        att_vals = [s0]

        all_equal = True
        
        for file_name in self.pop_group_attrs:
            s1 = sorted(self.pop_group_attrs[file_name][group])[index][1]
            if v_orig == "_null_":
                v_orig = s1
            else:
                if v_orig != s1:
                    all_equal = False
                #-- end if
            #-- end if

            s1 = s1.ljust(self.max_val_widths[file_name])

            att_vals.append(s1)
        #-- end for

        if not(all_equal and diff_only):
            if (all_equal):
                #print(self.fmt % tuple(row_args))
                bHadOutput = True
            else:
                if hilite_state ==  HILITE_DIFF_GROUP:
                    cols = self.group_diffs(att_vals)
                    att_vals[0] =  colors.INVERTBW+att_vals[0]+colors.OFF
                    for i in range(1, len(att_vals)):
                        att_vals[i] = varcols[cols[i]]+att_vals[i]+colors.OFF
                    #-- end for

                elif hilite_state ==  HILITE_FILE_GROUP:
                    cols = self.group_files(att_vals)
                    att_vals[0] =  colors.INVERTBW+att_vals[0]+colors.OFF
                    for i in range(1, len(att_vals)):
                        att_vals[i] = varcols[cols[i]]+att_vals[i]+colors.OFF
                    #-- end for

               


                #-- end if
            #-- end if
        #-- end if
        s = ""
        if (not all_equal or not diff_only):
            # create the output line
            s = "| " 
            for s1 in att_vals:
                s = s + s1 + " | "
            # -- end for
            
            if hilite_state == HILITE_SIMPLE:
                s =  colors.TBGHIRED+s+colors.OFF
            #-- end if
        #-- end if        
        return s
    #-- end def


    #-------------------------------------------------------------------------
    #-- create_group_line
    #--
    def create_group_line(self, group):
        ss = self.create_separator()
        s = "| "
        s0 = group
        #print("maxn: %d, s0:[%s]:%d"%(self.max_name_width, s0, len(s0)))
        s0 = s0.ljust(len(ss)-4) + " |"
        s = s+s0
        return s
    #-- end def
    

    #-------------------------------------------------------------------------
    #-- create_simple_line
    #--
    def create_simple_line(self, line_data):
        bfirst = True

        s = "| "
        i = 1
        s = s + str(line_data[0]).ljust(1+self.max_name_width) + "| "
        for file_name in self.pop_group_attrs:
            s = s + str(line_data[i]).ljust(1+self.max_val_widths[file_name]) + "| "
            i = i + 1
        #-- end for
        return s
    #-- end def

    #-------------------------------------------------------------------------
    #-- show_all_attrs_init
    #--   shows attrs as found by colllect_attrs
    #--
    def show_all_attrs_init(self):
        for file_name in self.hdf_attrs:
            print("%s"%file_name)
            for spc_name in self.hdf_attrs[file_name]:
                print("    %s"%spc_name)
                for group_name in self.hdf_attrs[file_name][spc_name]:
                    print("        %s"%group_name)
                    for a in self.hdf_attrs[file_name][spc_name][group_name]:
                        print("                %s:%s"%(a,  self.hdf_attrs[file_name][spc_name][group_name][a][0]))
                #-- end for
            #-- end for
        #-- end for
    #-- end def


    #-------------------------------------------------------------------------
    #-- show_all_attrs
    #--   shows attrs from the reshaped map
    #--
    def show_all_attrs(self):
        for file_name in self.pop_group_attrs:
            print(file_name)
            print("%s - %s: max n %d, max v %d"%(file_name[0], file_name[1], self.max_name_width, self.max_val_width[file_name]))
            for group_name in self.pop_group_attrs[file_name]:
                print("        %s"%group_name)
                for a in self.pop_group_attrs[file_name][group_name]:
                    print("                %s:%s"%(a[0],a[1]))
                #-- end for
            #-- end for
        #-- end for
    #-- end def


    #-------------------------------------------------------------------------
    #-- show_full_attrs
    #--   shows attrs from the reshaped map
    #--
    def show_full_attrs(self):
        keys = [x for x in self.full_attrs]
        for group_name in sorted(keys):
            print("        %s"%group_name)
            for a in self.full_attrs[group_name]:
                print("                %s"%(a))
            #-- end for
        #-- end for
    #-- end def


    #-------------------------------------------------------------------------
    #-- show_nice
    #--   shows all attrs of all files in a table
    #--
    def show_nice(self, hilite, diff_only, show_global, show_empty, show_numagents):

        print(self.create_separator())
        print(self.create_simple_line(self.header_entries))
        if show_numagents:
            print(self.create_separator())
            print(self.create_simple_line(self.size_entries))
        #-- end if
        #stderr.write("full_attrs: %s\n"%self.full_attrs)
        keys = [x for x in self.full_attrs]
        # make sure 'global' comes first
        for group_name in sorted(keys, key=lambda x: 'AAA_'+x if x=='global' else x):

            all_equal = self.collect_equalities(group_name)
            # we only write group line if there are any attributes or if all lines are different or if diffonly is not set
            if (show_empty or (len(self.full_attrs[group_name]) > 0)) and not (all_equal and diff_only):

                # we only display the globals if 'show_global' is set
                if (group_name != 'global') or show_global:
                    print(self.create_separator())
                    print(self.create_group_line(group_name))
                    # only add a separator if the group has attributes
                    if (len(self.full_attrs[group_name]) > 0):
                        print(self.create_separator())
                    for i in range (len(self.full_attrs[group_name])):
                        s = self.create_att_line(group_name, i, hilite, diff_only)
                        if (s != ''):
                            print(s);
                        #-- end if
                    #-- end for
                #-- end if
            #-- end if
        #-- end for
        print(self.create_separator())
    #-- end def


#-----------------------------------------------------------------------------
#-- collect_compact_settings
#--
def collect_compact_settings(arg, params):
    stderr.write("arg is [%s]\n"%arg)
    onoffstates = {True:"on", False:"off"}
    iResult = 0
    s1 = arg
    i = 0
    while i < len(s1):
        
        c = s1[i]
        if c.lower() in paramshort:
            parname = paramshort[c.lower()]
            if ( c.lower() == 'h'):
                c2 = 'n'
                if c == 'H':
                    i = i+1
                    if (i >= len(s1)):
                        c2 = 'n'
                    else:
                        c2 = s1[i]
                    #-- end if
                #-- end if
                if c2 in hiliteshort:
                    params[parname] = hiliteshort[c2.lower()]
                    stderr.write("set hilite state [%s]\n" % (params[parname]))
                else:
                    iResult = -1
                    stderr.write("invalid hilite state [%s]. Should be one of dgns\n" % (c2))
                #-- end if
            elif (c.lower() == 's'):
                c2 = 'n'
                if c == 'S':
                    i = i+1
                    if (i >= len(s1)):
                        c2 = 'n'
                    else:
                        c2 = s1[i]
                    #-- end if
                #-- end if
                if c2 in shortnameshort:
                    params[parname] = shortnameshort[c2.lower()]
                else:
                    iResult = -1
                    stderr.write("invalid hilite state [%s]. Should be one of apn\n" % (c2))
                #-- end if
            else:
                params[parname] = c.isupper()
            #-- end if
        else:
            iResult = -1
            stderr.write("invalid state [%s]. Should be one of aAdDeEgGnNqQsShH\n" % (c))
            break;
        #-- end if
        i=i+1
    #-- end for
    return iResult
#-- end def


#-----------------------------------------------------------------------------
#-- collect_settings
#--  we expect
#--   <prop_name>:("on"|"off")[,<prop_name>:("on"|"off")]*
#--
def collect_settings(arg, params):
    states = ["on", "off"]
    iResult = 0

    if not ':' in arg:
        iResult = collect_compact_settings(arg, params)
    else:
        settings = arg.split(',')
        for setting in settings:
            if (':') in setting:
                (key,val) = setting.split(':')
                if (key in params):
                    if (key == 'hilite'):
                        if val in hilites:
                            params['hilite'] = val
                        else:
                            iResult = -1
                            stderr.write("invalid hilite state [%s]\n" % (val))
                            break;
                        #-- end of
                    elif (key == 'shortnames'):
                        if val in shortnames:
                            params['shortnames'] = val
                        else:
                            iResult = -1
                            stderr.write("invalid shortname state [%s]\n" % (val))
                            break;
                        #-- end if
                    elif (val in states):
                        params[key] = (val == "on")
                    else:
                        iResult = -1
                        stderr.write("invalid state [%s]\n" % (val))
                        break;
                    #-- end if
                else:
                    iResult = -1
                    stderr.write("unknown setting [%s]\n" % (key))
                    break;
                #-- end if
                
            else:
                iResult = -1
                stderr.write("no ':' in setting [%s]\n" % (setting))
                break;
            #-- end if
        #-- end for

    #-- end if
    stderr.write("params:%s\n"%(params))
    return iResult
#-- end def

#-----------------------------------------------------------------------------
#-- main
#--
#--   comp -d <prop>[,<prop>*] <qdf_file>*
#--   prop ::= "global:"<state> |"action:"<state> | "hilite:"<state> | 

# default settings
params = {}
params["global"]     = True
params["action"]     = True
params["diffonly"]   = False
params["numagents"]  = True
params["shortnames"] = False
params["equalsize"]  = False
params["showempty"]  = False
params["hilite"]     = HILITE_FILE_GROUP

if (len(argv) > 1):

    cur_index = 1
    iResult   = 0
    if argv[1] == '-d':
        if (len(argv) > 2):
            iResult = collect_settings(argv[2], params)
            cur_index = 3
        else:
            stderr.write("excpected settings after '-d'\n")
        #-- end if
    #-- end if

    if (iResult == 0):
        ac = AttrComparator(argv[cur_index:])
        try:
            ac.collect_attrs()
        except Exception as e:
            print(e)
        else:
            stderr.write("reshaping\n")
            ac.reshape_attrs()
            stderr.write("find max widths\n")
            ac.find_max_widths()
            #ac.show_all_attrs()
            stderr.write("create full list\n")
            ac.create_full_attr_list()
            #print("Full attrs:")
            #ac.show_full_attrs()
            stderr.write("pad groups\n")
            #ac.show_full_attrs()
            k = [f for f in ac.pop_group_attrs]
            for i in range(len(k)):
                stderr.write("g[%d]: %s\n"%(i, k[i]))
                ac.pad_groups(k[i])
            #-- end for
            #ac.show_all_attrs()   

            ac.show_nice(params["hilite"], params["diffonly"], params['global'],params['showempty'], params['numagents'])
            
        #-- end try
    else:
        stderr.write("Error in settings:[%s]\n" % (argv[2]))
    #-- end if
else:
    print('e:')
    print('  %s [-d <prop>[,<prop>]] (<qdf_file>[:<pop>])*' % (argv[0]))
    print('or')
    print('  %s [-d <propshorts>] (<qdf_file>[:<pop>])*' % (argv[0]))
    print('where')
    print('  qdf_file   name of a qdf file') 
    print('  pop        name of species to show attributes of (if omitted, all populations in this file are shown)')
    print('  prop       display properties settings, with format')
    print('    prop  ::= "global:"<state>   | "action:"<state>    | "hilite:"<hilite_state>     | "diffonly:"<state> | ')
    print('              "variants:"<state> | "numagents:"<state> | "shortnames:"<short_state>        | "equalsize:"<state>')
    print('    state ::= "on" | "off"')
    print('Properties:')
    print('  global:       %-7s show "global" attributes not belonging to an action' % ("["+("on" if params["global"] else "off")+"]"))
    print('  action:       %-7s show "action" atributes' % ("["+("on" if params["action"] else "off")+"]"))
    print('  diffonly:     %-7s only show rows with different values' % ("["+("on" if params["diffonly"] else "off")+"]"))
    print('  numagents:    %-7s show number of agents in header line' % ("["+("on" if params["numagents"] else "off")+"]"))
    print('  short_state:  %-7s shorten names' % ("["+("on" if params["shortnames"] else "off")+"]"))
    print('                \'none\'   use names as given')
    print('                \'path\'   remove path')
    print('                \'alias\'  use short alias names: qdf_*')
    print('  equalsize:    %-7s same width for all columns' % ("["+("on" if params["shortnames"] else "off")+"]"))
    print('  hilite_state: %-7s use colors to hilite differences' % ("["+params['hilite']+"]"))
    print('                \'none\'    no hilighting')
    print('                \'simple\'  hilight entire line if it contains differences')
    print('                \'diff0\'   hilight entries differing from the first one')
    print('                \'group\'   hilight roups of equal entries with the same color')
    print('  propshorts:   a string consisting of characters of the following list.')
    print('                 \'G\' or \'g\' - turn on or off property \'global\'')
    print('                 \'A\' or \'a\' - turn on or off property \'action\'')
    print('                 \'D\' or \'d\' - turn on or off property \'diffonly\'')
    print('                 \'N\' or \'n\' - turn on or off property \'numagents\'')
    print('                 \'E\' or \'e\' - turn on or off property \'equalsize\'')
    print('                 \'S\' or \'s\' - turn on or off property \'shortnames\'')
    print('                   \'S\' must be folllowed by one of these characters:')
    print('                      \'n\' - none') 
    print('                      \'p\' - path') 
    print('                      \'a\' - alias') 
    print('                 \'H\' or \'h\' - turn on or off property \'hilite\'')
    print('                    \'H\' must be followed by on of these characters:')
    print('                      \'n\' - none') 
    print('                      \'s\' - simple') 
    print('                      \'d\' - diff grouping') 
    print('                      \'f\' - file grouping') 
    print()
#-- end main


