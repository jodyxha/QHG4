#!/usr/bin/python

from sys import argv
import re
import glob
import copy

PAT_CLASSNAME    = r'^class *(\w*) *: public *Action<T>.*{'
PAT_CLASSNAME2   = r'^class *(\w*) *: public *Evaluator<T>.*{'
PAT_ATTRNAMES    = r'^const *static *std::string (ATTR_\w*) *= *"(\w*).*'

PAT_CONSTRUCTOR  = r'^(\w+)::\1\([^\)]*\)'
PAT_DESTRUCTOR   = r'^(\w+)::~\1\([^\)]*\)'
PAT_METHOD       = r'^\w+ +(\w+)::(\w+)\([^\)]*\)'
PAT_INCLUDE      = r'#include *"([^"]*)"'

PAT_EVALINFO_DEF     = r'^ *MultiEvaluator<\w*>::evaluatorinfos *(\w*);'
PAT_EVALINFO_ADD     = r'^ *(\w+).push_back\(std::pair<std::string, *Evaluator<\w*>\*>\(("[^"]*"), *([\w\*\->\(\)\.\[\]]*)\)\);'# could also have "pSing = new SingleEvaluator<jkgY>(...)"

# matches: pointer name, id, poly_name
PAT_SINGLE_EVAL_DEF  = r'^ *(?:SingleEvaluator<\w*> \*)?(\w*) *= *new  *SingleEvaluator *<\w*> *\(\w*, *[\w\*\->\(\)\.\[\]]*, *"([^"]*)", *[\w\*\->\(\)\.\[\]]*, *[\w\*\->\(\)\.\[\]]*, *("[^"]*"), *[^\)]*\);'

# matches: pointer name, id
PAT_SHARE_EVAL_DEF  = r'^ *(?:ShareEvaluator<\w*> \*)?(\w*) *= *new ShareEvaluator *<\w*> *\([\w\*\->\(\)\.\[\]]*, *[\w\*\->\(\)\.\[\]]*, *"([^"]*)", *.*'

# matches: pinter name, id, eval_info name
PAT_MULTI_EVAL_DEF  = r'^ *(?:MultiEvaluator<\w*> *\*)?([\w\*\->\(\)\.\[\]]*) *= *new  *MultiEvaluator *<\w*> *\(\w*, *[\w\*\->\(\)\.\[\]]*, *"([^"]*)", *[\w\*\->\(\)\.\[\]]*, *([\w\*\->\(\)\.\[\]]*)'

# matches: action type
PAT_NORM_ACTION_DEF  = r'^ *(?:\w+<\w+> *\*)?[\w\*\->\(\)\.\[\]]* *= *new (\w*) *<\w*> *\(.*'

INDENT = 4*" "

#-----------------------------------------------------------------------------
#--  class QHGError
#--
class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class


#-----------------------------------------------------------------------------
#-- class ParamError
#--
class ParamError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class



#-----------------------------------------------------------------------------
#--  class PopXNLCreator
#--
class PopXNLCreator:

    #-----------------------------------------------------------------------------
    #--  constructor
    #--
    def __init__(self, action_dir, pop_cpp):
        self.all_actions  = {}
        self.norm_actions = []
        self.evaluators   = {}
        self.multis       = {}

        # do all the collecting
        self.process_pop_ctor(action_dir, pop_cpp)
        
    #-- edf
                              
    #-----------------------------------------------------------------------------
    #--  collect_all_actions
    #--    sets self.all_actions to  a map actionNamr => list of attribute name
    #--
    def collect_all_actions(self, action_dir):
        self.all_actions = {}
        attr_names = []
       
        hfiles = glob.glob(action_dir + '/*.h')
        action_name = ""
    
        for f in hfiles:
            try:
                fIn = open(f, "rt")
                for line in fIn:
                    m = re.search(PAT_CLASSNAME, line)
                    if (not m is None):
                        action_name = m.groups()[0]
                        self.all_actions[action_name] = attr_names
                        attr_names=[]
                        break
                    #-- end if
                    m = re.search(PAT_CLASSNAME2, line)
                    if (not m is None):
                        action_name = m.groups()[0]
                        self.all_actions[action_name] = attr_names
                        attr_names=[]
                        #eval_names.append(action_name)
                        break
                    #-- end if
                    m = re.search(PAT_ATTRNAMES, line)
                    if (not m is None):
                        attr_name  = m.groups()[0]
                        attr_value = m.groups()[1]
                        if (attr_name.count("_") > 2) and (attr_value != ""):
                            attr_names.append(attr_value)
                        #-- end if
                    #-- end if
                #-- end for
                fIn.close()
                if action_name == "":
                    print("Error: no action name found")
                #-- end if
            except IOError as e:
                raise QHGError("problem getting coords:\n%s"%str(e))
            #-- end try

        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_evalinfo_def
    #--   find a eval info definition
    #--
    def get_evalinfo_def(self, line):
  
        line_read = False
        m = re.search(PAT_EVALINFO_DEF, line)
        if not m is None:
            cureval=m.groups()[0]
            if not cureval in self.eval_infos:
                #print("get_evalinfo_def cureval [%s]"%(cureval))

                self.eval_infos[cureval] = {}
                line_read = True
            else:
                raise QHGError("Mutiple eval infos [%s]"%cureval)
            #-- end if
        #-- end if
        return line_read
    #-- end def 


    #-----------------------------------------------------------------------------
    #-- get_evalinfo_add
    #--
    def get_evalinfo_add(self, line):
  
        line_read = False
        m = re.search(PAT_EVALINFO_ADD, line)
        if not m is None:
            g = m.groups()
            #print("match: %s:%s"%(g[0],g[2]))
            cureval=g[0]
            #print("get_evalinfo_add cureval [%s],name [%s]"%(cureval, g[2]))
            if cureval in self.eval_infos:
                self.eval_infos[cureval][g[2]] ={"weight":g[1]}
            else:
                raise QHGError("no eval info [%s] defined"%cureval)
            #-- end if
            line_read = True
        #-- end if
        return line_read
    #-- end def 


    #-----------------------------------------------------------------------------
    #-- get_evaluator
    #--   match line from the costructor to creation of evaluaotr
    #--
    def get_evaluator(self, line):
        line_read = False

        # check for SingleEvaluator
        if not line_read:
            m = re.search(PAT_SINGLE_EVAL_DEF, line)
            if not m is None:
                g=m.groups()
                cur_name = g[0]
                # name, id, polyname
                if not cur_name in self.evaluators:
                    self.evaluators[cur_name] = {"type":"SingleEvaluator", "id":g[1], "polyname":g[2]}
                    line_read = True
                else:
                    raise QHGError("Mutiple evaluator name [%s}"%cur_name)
                #-- end if
            #-- end if
        #-- end if

        # check for ShareEvaluator
        if not line_read:
            m = re.search(PAT_SHARE_EVAL_DEF, line)
            if not m is None:
                g = m.groups()
                cur_name = g[0]
                #print("shareeval cur_name [%s]"%cur_name)
                # name, id,
                if not cur_name in self.evaluators:
                    self.evaluators[cur_name] = {"type":"ShareEvaluator", "id":g[1]}
                    line_read = True
                else:
                    raise QHGError("Mutiple evaluator name [%s}"%cur_name)
                #-- end if
            #-- end if
        #-- end if

        # check for MultiEvaluator
        if not line_read:
            m = re.search(PAT_MULTI_EVAL_DEF, line)
            if not m is None:
                g = m.groups()
                cur_name = g[0]
                # name, id, evalinfo
                #print("multieval1 cur_name [%s]"%cur_name)
                if not cur_name in self.evaluators:
                    self.evaluators[cur_name] = {"type":"MultiEvaluator", "id":g[1], "evalinfo":g[2], "children":[]}
                    line_read = True
                else:
                    raise QHGError("Mutiple evaluator name [%s}"%cur_name)
                #-- end if
            #-- end if
        #-- end if

        return line_read
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_norm_action
    #--
    def get_norm_action(self, line):

        line_read = False

        # check for norm action
        if not line_read:
            m = re.search(PAT_NORM_ACTION_DEF, line)

            if not m is None:
                g = m.groups()
                cur_action = g[0]
                if (not "Evaluator" in cur_action) and (cur_action in self.all_actions):
                    self.norm_actions.append(cur_action)
                    line_read = True
                #-- end if
            #-- end if
        #-- end if
        return line_read
    #- end def

    
    #-----------------------------------------------------------------------------
    #-- get_ctor_lines
    #--   reads constructor lines searching for creation of evalinfos normal
    #--   actions and evaluators
    #--     
    def get_ctor_lines(self, pop_cpp):
        self.eval_infos = {}
        self.evaluators = {}
        self.norm_actions = []

        is_in_ctor = False
        try:
            fIn = open(pop_cpp, "rt")
            for line in fIn:
                bok = False
                if is_in_ctor:
                    m = re.search(PAT_DESTRUCTOR, line)
                    if not m is None:
                        is_in_ctor=False
                        bok = True
                    #-- end if
                    if not bok:
                        m = re.search(PAT_METHOD, line)
                        if not m is None:
                            is_in_ctor=False
                            bok = True
                        #-- end if
                    #-- end if
                    
                    if is_in_ctor:
                        #print("in ctor line [%s]"%line)

                        if not bok:
                            bok = self.get_evalinfo_def(line)
                        #-- end if
                        if not bok:
                            bok = self.get_evaluator(line)
                        #-- end if
                        if not bok:
                            bok = self.get_evalinfo_add(line)
                        #-- end if
                        if not bok:
                            bok = self.get_norm_action(line)
                        #-- end if
                    #-- end if
                else:
                    m = re.search(PAT_CONSTRUCTOR, line)
                    if not m is None:
                        class_name = m.groups()[0]
                        is_in_ctor = True
                    #-- end if
                #-- end if
            #-- end for
            fIn.close()
        except IOError as e:
            raise QHGError("Couldn't open pop file [%s}"%pop_cpp)
        #-- end try
        
    #-- end def


    #-----------------------------------------------------------------------------
    #-- fell_evalinfos
    #--
    def fill_evalinfos(self):
        for ei in self.eval_infos:
            for ev_name in self.eval_infos[ei]:
                if ev_name in self.evaluators:
                    self.eval_infos[ei][ev_name].update(self.evaluators[ev_name].copy())
                    del self.evaluators[ev_name]
                else:
                    raise QHGError("EvalInfos evaluaotr name [%s] is not an evaluator"%ev_name)
                #-- end if
            #-- end for
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- link_evaluators
    #--
    def link_evaluators(self):
        
        self.multis = {}
        # find multievaluators
        for e in self.evaluators:
            if self.evaluators[e]["type"] == "MultiEvaluator":
                self.multis[e] = self.evaluators[e]
            #-- end if
        #-- end for
        multilist = list(self.multis)

        while len(multilist) > 0:
            me = multilist[0]
            
            #multis[me]["children"] = []
            evi_name = self.multis[me]["evalinfo"]

            if evi_name in self.eval_infos:
                for pe in self.eval_infos[evi_name]:
                    if "evalinfo" in self.eval_infos[evi_name][pe]:
                        print("deleting evalinfo of %s in evalinfos:%s"%(pe,self.eval_infos[evi_name][pe])) 
                        del self.eval_infos[evi_name][pe]["evalinfo"]
                    #- end if    
                    self.multis[me]["children"].append(copy.deepcopy(self.eval_infos[evi_name][pe]))
                #-- end for
                del self.eval_infos[evi_name]
            else:
                raise QHGError("MultiEval's eval_info name doesn't exist [%s]"%evi_name)
            #-- end if
            
            # somewhere torards the end
            del multilist[0]
            print("deleting evalinfo of %s in multis"%me) 
            del self.multis[me]["evalinfo"]
        #-- end while
    #-- end def


    #-----------------------------------------------------------------------------
    #-- wrap_toplevel_evaluators
    #--
    def wrap_toplevel_evaluators(self):
        self.multis=[]
        for e in self.evaluators:
            evtype= self.evaluators[e]["type"]
            if  evtype == "MultiEvaluator":
                self.multis.append({"weight":"", 'type': 'MultiEvaluator', 'id': self.evaluators[e]['id'], 'children': self.evaluators[e]['children']})
            elif  evtype == "SingleEvaluator":
                self.multis.append({"weight":"", 'type': 'SingleEvaluator', 'id': self.evaluators[e]['id'], 'polyname': self.evaluators[e]['polyname']})
            elif  evtype == "ShareEvaluator":
                self.multis.append({"weight":"", 'type': 'ShareEvaluator', 'id': evaluators[e]['id']})
            else:
                raise QHGError("Unknown evaluator type [%s]"%e["type"])
            #-- end if
        #-- end for
        #return multis
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- process_pop_ctor
    #--
    def process_pop_ctor(self, action_dir, pop_cpp):
        self.collect_all_actions(action_dir)
        
        for action in self.all_actions:
            print("%s:"%action)
            for a in self.all_actions[action]:
                print("  %s"%a)
            #-- end for
        #-- end for
        
        #self.collect_includes(pop_cpp)
        self.get_ctor_lines(pop_cpp)

        self.fill_evalinfos()
        self.link_evaluators()
        self.wrap_toplevel_evaluators()
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_singleevaluator
    #--
    def write_singleevaluator(self, fOut, child, offset):
        fOut.write('\n%s<module name="SingleEvaluator" id="%s">\n'%(offset, child["id"]))

        if child["polyname"] != "":
            fOut.write('    %s<param name="%s" value="???0.0 0.0 1.0 1.0???" />\n'%(offset, child["polyname"]))
        #-- end if
        fOut.write("%s</module>\n"%(offset))    
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_shareevaluator
    #--
    def write_shareevaluator(self, fOut, child, offset):
        fOut.write('\n%s<module name="ShareEvaluator" id="%s">\n'%(offset, child["id"]))
        fOut.write('    %s<param name="ShareEvaluator_%s_arrayname" value="???arrayname???" />\n'%(offset, child["id"]))
        fOut.write('    %s<param name="ShareEvaluator_%s_polyname"  value="???polyname???" />\n'%(offset, child["id"]))
        fOut.write('%s</module>'%(offset))    
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_multievaluator
    #--
    def write_multievaluator(self, fOut, child, offset):
        fOut.write('\n%s<module name="MultiEvaluator" id="%s">\n'%(offset, child["id"]))
        pad_len = 0
        if (len(child["children"]) > 0):
            pad_len = 4 + max([len(a) for a in child["children"]])
        #-- end if
    
        for sub in child["children"]:
            fOut.write('    %s<param name=%s value="???0.0???" />\n'%(offset, (sub["weight"]).ljust(pad_len)))
        #- end for
        
        for sub in child["children"]:
    
            if sub["type"] == "SingleEvaluator":
                self.write_singleevaluator(fOut, sub, INDENT+offset)
            elif sub["type"] == "ShareEvaluator":
                self.write_shareevaluator(fOut, sub, INDENT+offset)
            elif sub["type"] == "MultiEvaluator":
                self.write_multievaluator(fOut, sub, INDENT+offset)
            else:
                raise QHGError("unknown evaluator type [%s]"%sub["type"])
            #-- end if
        #- end for
        fOut.write('%s</module>\n'%(offset))    
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_norm_action
    #--
    def write_norm_action(self, fOut, action, offset):

        pad_len = 0
        if (len(self.all_actions[action]) > 0):
            pad_len = 4 + max([len(a) for a in self.all_actions[action]])
        #-- end if
        fOut.write('\n%s<module name="%s" id="%s">\n'%(offset,action,""))
        for a in self.all_actions[action]:
            fOut.write('%s%s<param name=%s value="???0.0???" />\n'%(INDENT, offset, ('"'+a+'"').ljust(pad_len)))
        #- end for
        fOut.write('%s</module>\n'%offset)
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- write_priorities 
    #--
    def write_priorities(self, fOut, offset):
        pad_len1 = 0
        pad_len2 = 0
        if (len(self.norm_actions) > 0):
            pad_len1 = 4 + max([len(a) for a in self.norm_actions])
        #-- end if
        evnames = ['"%s[%s]"'%(self.evaluators[e]["type"], self.evaluators[e]["id"]) for e in self.evaluators]
        if (len(evnames) > 0):
            pad_len2 = 4 + max([len(a) for a in evnames])
        #-- end if
        pad_len =max(pad_len1,pad_len2)

        fOut.write('\n%s<priorities>\n'%offset)

        for a in self.norm_actions:
            fOut.write('%s%s<prio name=%s value="???0???" />\n'%(INDENT,offset, ('"' +a+'"').ljust(pad_len)))
        #-- end for

        for name in evnames:
            fOut.write('%s%s<prio name=%s value="???0???" />\n'%(INDENT,offset, name.ljust(pad_len)))
        #-- end for
    
        fOut.write('%s</priorities>\n'%offset)
    #-- end if


    #-----------------------------------------------------------------------------
    #-- write_xml
    #--
    def write_xml(self, class_name, species_name, output_file):
        try:
            fOut = open(output_file, "wt")

        
            fOut.write('<class name="%s" species_name="%s" species_id="104">\n'%(class_name, species_name))

            for action in self.norm_actions:
                self.write_norm_action(fOut, action, INDENT)
            #-- end for

            for x in self.multis:
                evtype= x["type"]
                if  evtype == "MultiEvaluator":
                    self.write_multievaluator(fOut, x, INDENT)
                elif  evtype == "SingleEvaluator":
                    self.write_singleevaluator(fOut, x, INDENT)
                elif  evtype == "ShareEvaluator":
                    self.write_shareevaluator(fOut, x, INDENT)
                else:
                    raise QHGError("Unknown evaluator type [%s]"%e["type"])
                #-- end if
            #- end for

            self.write_priorities(fOut, INDENT)
    
            fOut.write('</class>\n')

            fOut.close()
        
        except IOError as e:
            raise QHGError("Couldn't open output file [%s}"%output_file)
        #-- end try
    #-- end def

#-- end class

-------------------------------------------------------------------------
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


# main
if len(argv) > 6:

    # check /home/jody/progs/QHG4/genes/pcomulti.py
    paramdefs = {
        "-a" : ["action_dir", None],
        "-p" : ["pop_cpp",    None],
        "-o" : ["output",     None],
        "-d" : ["dfaults",    ""],
    }
    
    results = readParams(paramdefs)
    showParams(results)

    
    
    action_dir  = paramdefs["action_dir"]
    pop_file    = paramdefs["pop_cpp"]
    output_file = paramdefs["output"]
    defaults    = paramdefs["default"}

    class_name = re.search("(\w+)", pop_file).groups()[0]

    pxc = PopXNLCreator(action_dir, pop_file)
    pxc.write_xml(class_name, "sapiens", output_file)
    
else:
    print("usage:")
    print(" %s -a <action_dir> -p <pop_cpp> -o <output_file> [-d <defaults>]"%argv[0])
#- end if

'''
@TODO:
    - xml reader for pop xmls (similar to ParaProvider/qhgXML
      https://docs.python.org/3/library/xml.etree.elementtree.html
    - analogon to MooduleComplex
    - convert PopXNLCreator data to MooduleComplex analogon?
    - read default xmls -> Module complexes
    - merge original MC with defaults
    - write final MC to XML

    
def show_node(node, indent):
    for child in node:
        print("%s%s: %s"%(indent, child.tag, child.attrib))
        show_node(child, indent+"  ")

>>> def find_data(root, path):
...     for child in root:
...         if (len(path) == 0):
...             value=child.attrib["value"]
...             print(value)
...         else:
...             if (child.attrib["name"] == path[0]):
...                 find_data(child, path[1:])
... 


/home/jody/envnavexp/dongo.xml
/home/jody/progs/QHG4/actions/EnvNavPop.cpp
