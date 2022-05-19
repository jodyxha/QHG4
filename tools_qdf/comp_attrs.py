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


HILITE_NONE   = 'none'
HILITE_SIMPLE = 'simple'
HILITE_DIFF0  = 'diff0'
HILITE_GROUP  = 'group'
HILITE_OLDGROUP  = 'oldgroup'
hilites=[HILITE_NONE, HILITE_SIMPLE, HILITE_DIFF0, HILITE_GROUP, HILITE_OLDGROUP]

SHORTNAME_NONE  = 'none'
SHORTNAME_PATH  = 'path'
SHORTNAME_ALIAS = 'alias'
shortnames=[SHORTNAME_NONE,  SHORTNAME_PATH, SHORTNAME_ALIAS]

alias_pattern = 'qdf_%02d'

#-----------------------------------------------------------------------------
#-- AttrComparator
#--
class AttrComparator:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdfs):
        self.qdfs = qdfs
        self.keys = []
        self.all_attrs   = {}
        self.attr_names  = []
        self.max_l_attrs = 0
        self.max_col_w   = {}
        self.num_agents  = {}
        self.alias_names = {}
    #-- end def


    #-----------------------------------------------------------------------------
    #-- collect_attrs
    #--
    def collect_attrs(self, shortname_state, do_equalsize):
        #for i in range(len(self.qdfs)):
        #    self.max_col_w[i] = 0
        #-- end for
        iCol = 0

        max_l_names = 0
        for f0 in self.qdfs:
            singlepop = ''
            if f0.find(':') >= 0:
                f, singlepop = f0.split(':')
            else:
                f = f0
            #-- end if
            stderr.write("Opening %s\n" % (f))
            pa = PopAttrs.PopAttrs(f)
            if (singlepop != ''):
                pop_list = [singlepop]
            else:
                pop_list = pa.get_pop_names()
                if not pop_list:
                    raise QHGError("[collect_attrs] No [Populations] group in %s"%f)
                else:
                    stderr.write("got list: %s\n" % pop_list)
                #-- end if
            #--end if
            i=0
            for pop in pop_list:
                if not iCol in self.max_col_w:
                    self.max_col_w[iCol] = 0
                #-- end if
                loc_attrs = {}
                vals      = {}
                popkey = "%s:%s" % (f, pop)
                stderr.write("popkey: [%s]\n" % popkey)
                self.keys.append(popkey)
                num = pa.get_num_agents(pop)
                self.num_agents[popkey] = str(num)

                loc_attrs = pa.get_flat_attrs(pop)
                
                for key in loc_attrs:
                   
                    if (type(loc_attrs[key]) == list) and (len(loc_attrs[key]) == 1):
                        v = loc_attrs[key][0]
                        if isinstance(v,bytes):
                            v = v.decode()
                        else:
                            v = str(v)
                        #-- endif
                        vals[key] = v
                        
                        self.attr_names.append(key);
                        
                        # find longest file name
                        fname = f
                        if (shortname_state == SHORTNAME_PATH):
                            fname = f.split('/')[-1]
                        elif  (shortname_state == SHORTNAME_ALIAS):
                            fname =  alias_pattern%i
                        #-- end if
                        i = i + 1
                        
                        if len(fname) > self.max_col_w[iCol]:
                            self.max_col_w[iCol] = len(fname)
                        #-- end if 
                
                        # find longest attribute name
                        if len(key) > self.max_l_attrs:
                            self.max_l_attrs = len(key)
                        #-- end if
                    
                        # find longest attribute value
                        if len(vals[key]) > self.max_col_w[iCol]:
                            self.max_col_w[iCol] = len(vals[key])
                        #-- end if
            
                    #-- end if
                #--end for
                stderr.write("finished attr loop for [%s]\n" % popkey)
                self.all_attrs[popkey] = vals
                iCol = iCol + 1
            #-- end for
            stderr.write("finished pop loop for [%s]\n" % f0)
        #-- end for
        stderr.write("finished file loop\n")

        if do_equalsize:
            iColW = max([self.max_col_w[x] for x in self.max_col_w])
            for x in self.max_col_w:
                 self.max_col_w[x] = iColW
            #-- end for
        #-- end if

        stderr.write("finished collect_attrs\n")
    #-- end def


    #-----------------------------------------------------------------------------
    #-- separate_globals
    #--   global attributes contain no '_'
    #-- 
    def separate_globals(self, formatted_keys, global_attrs, action_attrs):
        for key in formatted_keys:
            if (key.strip() == key.strip().split("_")[0]):
                global_attrs.append(key)
            else:
                action_attrs.append(key)
            #-- end if
        #-- end for    
    #-- end if

    
    #-----------------------------------------------------------------------------
    #-- pad_names
    #--   pad attribute names to max attribute length
    #--   and file names to max column width
    #--
    def pad_names(self, shortname_state):
        self.attr_names = sorted(self.attr_names)

        # the first column must be wide enough for longest atttribute name
        self.formatted_keys = {}
        for name in self.attr_names:
            self.formatted_keys[name] = name.ljust(self.max_l_attrs+1)
        #-- end for

        self.formatted_qdfs = {}
        i = 0
        for name in self.keys:
            show_name = name.split(':')[0]
            if (shortname_state == SHORTNAME_PATH):
                show_name = show_name.split('/')[-1]
            elif (shortname_state == SHORTNAME_ALIAS):
                show_name = alias_pattern%i
            #-- end if
            self.alias_names[show_name] = name
            self.formatted_qdfs[name] = show_name.ljust(self.max_col_w[i])+" "
            i = i + 1
        #-- end for
        
    #-- end def


    #-----------------------------------------------------------------------------
    #-- checkequality
    #--   check if the first item differs from any of the rest
    #--
    def checkequality(self, items):
        bEqual = True;
        x0 = items[1].strip()
        for x in items[2:]:
            if (x0 != x.strip()):
                bEqual = False
                break
            #-- end if
        #-- end for
        return bEqual
    #-- end def


    #-----------------------------------------------------------------------------
    #-- find_variants
    #--
    def find_variants(self, items):
        classes = [items[1].strip()]
       
        for x in items[2:]:
            y= x.strip()
            if (y not in classes):
                classes.append(y)
            #-- end if
        #-- end for
        return classes
    #-- end def

            
    #-----------------------------------------------------------------------------
    #-- diff0_cols
    #--
    def diff0_cols(self, row_args):
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
    #-- group_cols
    #--
    def group_cols(self, row_args):
        cols = [0, 0]
        for i in range(2,len(row_args)):
            ref = -1
            for j in range(0,i):
                if (row_args[i] == row_args[j]):
                    ref = j
                    break
                #-- end if
            #- end for
            ref = ref if ref >= 0 else i
            cols.append(ref-1)
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


    #-----------------------------------------------------------------------------
    #-- print_attr_rows
    #--
    def print_attr_rows(self, attrs, split_groups, hilite_state, diff_only):
        prevgroup = ""
       
        bHadOutput = False
        bNext = False
        for att in sorted(attrs):

            # if a new group starts (and splitgroup is set, and its not the first group)
            # then write a separator
            curgroup =  att.strip().split("_")[0]
            if (curgroup != prevgroup):
                if (bHadOutput and split_groups):
                    print(self.sep2)
                #-- end if
                bHadOutput = False
                prevgroup = curgroup
            #-- end if

            # print attributes if they exist in a file, or spaces otherwise
            row_args = [self.formatted_keys[att]]
            i = 0
            for f in self.keys:#self.formatted_qdfs:
                if (att in self.all_attrs[f]):
                    row_args.append(self.all_attrs[f][att].ljust(self.max_col_w[i]+1))
                else:
                    row_args.append(" "*(self.max_col_w[i]+1))
                #-- end if
                i = i + 1
            #-- end for

            # are there differences?
            all_equal = self.checkequality(row_args)

            # how many different values are there?
            variant_vals = self.find_variants(row_args)
            
            
            
            if not(all_equal and diff_only):
                if (all_equal):
                     print(self.fmt % tuple(row_args))
                     bHadOutput = True
                else:
                    if hilite_state ==  HILITE_GROUP:
                        cols = self.group_cols(row_args)
                        row_args[0] =  colors.INVERTBW+row_args[0]+colors.OFF
                        for i in range(1, len(row_args)):
                            row_args[i] = varcols[cols[i]]+row_args[i]+colors.OFF
                        #-- end for
                        
                    if hilite_state ==  HILITE_OLDGROUP:
                        cols = self.group_cols_old(row_args)
                        row_args[0] =  colors.INVERTBW+row_args[0]+colors.OFF
                        for i in range(1, len(row_args)):
                            row_args[i] = varcols[cols[i]]+row_args[i]+colors.OFF
                        #-- end for
                        
                    elif hilite_state ==  HILITE_SIMPLE:
                        print(colors.TBGHIRED+self.fmt % tuple(row_args)+colors.OFF)
                    else:
                        if hilite_state ==  HILITE_DIFF0:
                            cols = self.diff0_cols(row_args)
                            row_args[0] =  colors.INVERTBW+row_args[0]+colors.OFF
                            for i in range(1, len(row_args)):
                                row_args[i] = varcols[cols[i]]+row_args[i]+colors.OFF
                            #-- end for
                        #-- end if
                    #-- end if
                    # now print (with or without highlited row_args)
                    print(self.fmt % tuple(row_args))
                    bHadOutput = True
                #-- end if
            #-- end if
            
            bNext = True
        #-- end for
        # lower border
        if  bHadOutput:
            print(self.sep2)
        #-- end if

    #-- end def

    #-----------------------------------------------------------------------------
    #-- show_aliases
    #--
    def show_aliases(self):
        print('Aliases:')
        for n in self.alias_names:
            print("  %s : %s"%(n, self.alias_names[n]))
        #-- end for
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- display
    #--
    def display(self, show_globals, show_actions, hilite_state, diff_only, numagents, do_shortnames):
        
        if (self.max_l_attrs == 0):
            self.max_l_attrs = len("Attribute")
        #-- end if
        self.groups = set([x.split("_")[0] for x in self.attr_names])

        # build format string for "Attribute" and all file names (or name and all values)
        self.fmt    = "| %s"*(len(self.keys)+1) + "|";

        # build a separator string with equidistant '+' in a long string of '-'
        sep1 = "+"+"-"*(self.max_l_attrs+2)
        self.sep2 = sep1+ "+"
        for i in self.max_col_w:
           self.sep2 = self.sep2 + "-"*(self.max_col_w[i]+2) + "+"
        #-- end for

        # make "Attribute" the first 'file name'
        na = "Attribute".ljust(self.max_l_attrs+1)
        nn = [self.formatted_qdfs[x] for x in self.keys]
        nn.insert(0,na)

        xa = "#Agents".ljust(self.max_l_attrs+1)
        xn = [xa]
        i = 0
        for x in self.keys:
            xn.append(self.num_agents[x].ljust(1+self.max_col_w[i]))
            i = i + 1
            #xn = [self.num_agents[x].ljust(1+self.iColW) for x in self.keys]
        #xn.insert(0,xa)

       
        # upper border
        print(self.sep2)
        
        # "attribute" and file names
        print(self.fmt % tuple(nn))
        print(self.sep2)

        if (numagents):
            # "#Agents" and agent numbers
            print(self.fmt % tuple(xn))
            print(self.sep2)
        #-- end if
        
        global_attrs = []
        action_attrs = []
        self.separate_globals(self.formatted_keys, global_attrs, action_attrs)

        # print global attributes
        if show_globals:
            self.print_attr_rows(global_attrs, False,  hilite_state, diff_only)
        #-- end if
        
        # print action attributes
        if show_actions:
            self.print_attr_rows(action_attrs, True, hilite_state, diff_only)
        #-- end if
        
    #-- end def

#-- end class

paramshort = {
    'g': 'global',
    'a': 'action',
    'd': 'diffonly',
    'n': 'numagents',
    's': 'shortnames',
    'e': 'equalsize',
    'h': 'hilite',
}
             
hiliteshort = {
    'n': HILITE_NONE,
    's': HILITE_SIMPLE,
    'd': HILITE_DIFF0,
    'g': HILITE_GROUP,
    'o': HILITE_OLDGROUP,
}

shortnameshort = {
    'n': SHORTNAME_NONE,
    'p': SHORTNAME_PATH,
    'a': SHORTNAME_ALIAS,
}

#-----------------------------------------------------------------------------
#-- collect_compact_settings
#--
def collect_compact_settings(arg, params):
    print("arg is [%s]"%arg)
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
            stderr.write("invalid state [%s]. Should be one of aAdDeEgGnNsShH\n" % (c))
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
params["hilite"]     = HILITE_DIFF0

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
            ac.collect_attrs(params['shortnames'], params['equalsize'])
        except Exception as e:
            print(e)
        else:
            ac.pad_names(params['shortnames'])
            ac.show_aliases()
            ac.display(params["global"], params["action"], params["hilite"], params["diffonly"], params['numagents'], params['shortnames'])
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
    print('                      \'d\' - diff0') 
    print('                      \'g\' - group') 
    print()
#-- end main
