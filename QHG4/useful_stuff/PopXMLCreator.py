#!/usr/bin/python

from sys import argv
import os
import re
import glob
import copy

## '\w'       : alphanumeric and underscore
## '(? ... )' : group contents, but don't return it as a match

substitutes = {
    "<<expr>>" : r'[\w\*\->\(\)\.\[\]]*',
    "<<word>>" : r'\w+',
}

templates = {
    "PAT_INCLUDE"           : r'#include *"([^"]*)"',
    "PAT_CLASSNAME"         : r'^class *(<<word>>) *: public *(Action|Evaluator)<T>.*{',
    "PAT_ATTRNAMES"         : r'^const *static *std::string (ATTR_<<word>>) *= *"(<<word>>).*',
    "PAT_CONSTRUCTOR"       : r'^(<<word>>)::\1\([^\)]*\)',
    "PAT_DESTRUCTOR"        : r'^(<<word>>)::~\1\([^\)]*\)',
    "PAT_METHOD"            : r'^<<word>> +(<<word>>)::(<<word>>)\([^\)]*\)',
    "PAT_EVALINFO_DEF"      : r'^ *MultiEvaluator<<<word>>>::evaluatorinfos *(<<word>>);',
    "PAT_EVALINFO_ADD"      : r'^ *(<<word>>).push_back\(std::pair<std::string, *Evaluator<<<word>>>\*>\(("[^"]*"), *(<<expr>>)\)\);',
    "PAT_SINGLE_EVAL_DEF"   : r'^ *(?:SingleEvaluator<<<word>>> \*)?(<<word>>) *= *new  *SingleEvaluator *<<<word>>> *\(<<word>>, *<<expr>>, *"([^"]*)", *<<expr>>, *<<expr>>, *"([^"]*)", *[^\)]*\);',
    "PAT_SHARE_EVAL_DEF"    : r'^ *(?:ShareEvaluator<<<word>>> \*)?(<<word>>) *= *new ShareEvaluator *<<<word>>> *\( *<<expr>>, *<<expr>>, *"([^"]*)", *.*',
    "PAT_MULTI_EVAL_DEF"    : r'^ *(?:MultiEvaluator<<<word>>> *\*)?(<<expr>>) *= *new  *MultiEvaluator *<<<word>>> *\(<<word>>, *<<expr>>, *"([^"]*)", <<expr>>, *(<<expr>>)',
    "PAT_NORM_ACTION_DEF"   : r'^ *(?:<<word>> *<<<word>>> *\*)? *<<expr>> *= *new (<<word>>) *<<<word>>> *\(<<word>>, *<<expr>>, *"([^"]*)"',
}

# PAT_INCLUDE matches:         header file name
# PAT_CLASSNAME matches:       class name
# PAT_ATTRNAMES matches:       symbolic attribute name,real attribute name
# PAT_CONSTRUCTOR matches:     class name
# PAT_DESTRUCTOR matches:      class name
# PAT_METHOD matches:          class name, method name
# PAT_EVALINFO_DEF matches:    eval info name,weight name,evaluator name 
# PAT_EVALINFO_ADD matches:    eval info name, ID, 
# PAT_SINGLE_EVAL_DEF matches: pointer name, id, poly_name
# PAT_SHARE_EVAL_DEF matches:  pointer name, id
# PAT_MULTI_EVAL_DEF matches:  pointer name, id, eval_info name
# PAT_NORM_ACTION_DEF matches: action type, id


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
#--  class PopXMLCreator
#--
class PopXMLCreator:

    #-----------------------------------------------------------------------------
    #--  constructor
    #--
    def __init__(self, action_dir, pop_cpp):
        self.all_actions  = {}
        self.norm_actions = [] # 
        self.evaluators   = {}
        self.root_multis  = {}
        self.sub_multis   = {}
        self.patterns     = {}

        self.prepare_patterns()
        # do all the collecting
        self.process_pop_ctor(action_dir, pop_cpp)
        
    #-- edf


    #-----------------------------------------------------------------------------
    #--  prepare_patterns
    #--    in all pattern templates we have to replace the substrings "<<word>>" and
    #--    "<<expr>>" withtheir substitutes
    #-
    def prepare_patterns(self):
        self.patterns={}
        for t in templates:
            x = templates[t]
            for s in substitutes:
                x = x.replace(s, substitutes[s])
            #-- end for
            self.patterns[t] = x
        #-- end for

        # print("PAT MULTI:%s"%self.patterns["PAT_MULTI_EVAL_DEF"])
    #-- end def

                
    #-----------------------------------------------------------------------------
    #--  collect_all_actions
    #--    look at all h files in the directory and extracts class names and
    #--    attribute names and values.
    #--    sets self.all_actions to a map {actionName => list of attribute names}
    #--
    def collect_all_actions(self, action_dir):
        self.all_actions = {}
        attr_names = []

        if (os.path.exists(action_dir)):
            hfiles = glob.glob(action_dir + '/*.h')
            action_name = ""

            if (len(hfiles) > 0) :
                for f in hfiles:
                    try:
                        fIn = open(f, "rt")
                        for line in fIn:
                            
                            # class name of an action
                            m = re.search(self.patterns["PAT_CLASSNAME"], line)
                            if (not m is None):
                                action_name = m.groups()[0]
                                self.all_actions[action_name] = attr_names
                                attr_names=[]
                                break
                            #-- end if
                            
                            # attribute: symbolic name and real name (but only the real name is needed)
                            m = re.search(self.patterns["PAT_ATTRNAMES"], line)
                            if (not m is None):
                                attr_sym_name  = m.groups()[0]
                                attr_real_name = m.groups()[1]
                                if (attr_sym_name.count("_") > 2) and (attr_real_name != ""):
                                    attr_names.append(attr_real_name)
                                #-- end if
                            #-- end if

                        #-- end for
                        fIn.close()
                        if action_name == "":
                            print("Error: no action name found")
                        #-- end if
                    except IOError as e:
                        raise QHGError("couldn't open file %s"%f)
                    #-- end try
                #-- end for
            else:
                raise QHGError("the directory '%s' contain no h files"%action_dir)
            #-- end if
        else:
            raise QHGError("the directory '%s' does not exist"%action_dir)
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_evalinfo_def
    #--   find an eval info definition
    #--
    def get_evalinfo_def(self, line):
  
        line_read = False
        m = re.search(self.patterns["PAT_EVALINFO_DEF"], line)
        if not m is None:
            cureval=m.groups()[0]
            if not cureval in self.eval_infos:
                # print("get_evalinfo_def cureval [%s]"%(cureval))

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
        m = re.search(self.patterns["PAT_EVALINFO_ADD"], line)
        if not m is None:
            g = m.groups()
            # print("match: %s:%s"%(g[0],g[2]))
            cureval=g[0]
            # print("get_evalinfo_add cureval [%s],name [%s]"%(cureval, g[2]))
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
    #--   match line from the costructor to creation of one of the evaluators
    #--
    def get_evaluator(self, line):
        line_read = False

        # check for SingleEvaluator
        if not line_read:
            m = re.search(self.patterns["PAT_SINGLE_EVAL_DEF"], line)
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
            m = re.search(self.patterns["PAT_SHARE_EVAL_DEF"], line)
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
            m = re.search(self.patterns["PAT_MULTI_EVAL_DEF"], line)
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
            m = re.search(self.patterns["PAT_NORM_ACTION_DEF"], line)

            if not m is None:
                g = m.groups()
                cur_action = g[0]
                cur_id     = g[1]
                if (not "Evaluator" in cur_action) and (cur_action in self.all_actions):
                    self.norm_actions.append([cur_action, cur_id])
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
                    m = re.search(self.patterns["PAT_DESTRUCTOR"], line)
                    if not m is None:
                        is_in_ctor=False
                        bok = True
                    #-- end if
                    if not bok:
                        m = re.search(self.patterns["PAT_METHOD"], line)
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
                    m = re.search(self.patterns["PAT_CONSTRUCTOR"], line)
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
    #-- fill_evalinfos
    #--
    def fill_evalinfos(self):
        self.sub_multis = {}
                  
        for ei in self.eval_infos:
            for ev_name in self.eval_infos[ei]:
                if ev_name in self.evaluators:

                    self.eval_infos[ei][ev_name].update(self.evaluators[ev_name].copy())
                    if (self.evaluators[ev_name]["type"] == "MultiEvaluator"):
                        self.sub_multis[ev_name] = self.evaluators[ev_name].copy();
                    del self.evaluators[ev_name]
                else:
                    #raise QHGError("[fill_evalinfos] EvalInfos evaluator name [%s] is not an evaluator"%ev_name)
                    raise QHGErrort("[fill_evalinfos] EvalInfos evaluator name [%s] is not an evaluator or has already been used"%ev_name)
                #-- end if
            #-- end for
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- link_evaluators
    #--
    def link_evaluators(self):
        
        self.root_multis = {}
        # find root-level multievaluators
        for e in self.evaluators:
            if self.evaluators[e]["type"] == "MultiEvaluator":
                self.root_multis[e] = self.evaluators[e]
            #-- end if
        #-- end for

        self.all_multis = self.sub_multis
        self.all_multis.update(self.root_multis)
        multilist = list(self.all_multis) 

        while len(multilist) > 0:
            me = multilist[0]
            
            #multis[me]["children"] = []
            evi_name = self.all_multis[me]["evalinfo"]
            
            if evi_name in self.eval_infos:
                for pe in self.eval_infos[evi_name]:
                    if "evalinfo" in self.eval_infos[evi_name][pe]:
                        # print("deleting evalinfo of %s in evalinfos:%s"%(pe,self.eval_infos[evi_name][pe])) 
                        del self.eval_infos[evi_name][pe]["evalinfo"]
                    #- end if    
                    self.all_multis[me]["children"].append(copy.deepcopy(self.eval_infos[evi_name][pe]))
                #-- end for
                del self.eval_infos[evi_name]
            else:
                raise QHGError("MultiEval's eval_info name doesn't exist [%s]"%evi_name)
            #-- end if
            
            # somewhere torards the end
            del multilist[0]
            # print("deleting evalinfo of %s in multis"%me) 
            del self.all_multis[me]["evalinfo"]
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
                self.multis.append({"weight":"", 'type': 'ShareEvaluator', 'id':  self.evaluators[e]['id']})
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

        if False:
            for action in self.all_actions:
                print("%s:"%action)
                for a in self.all_actions[action]:
                    print("  %s"%a)
                #-- end for
            #-- end for
        #-- end if
        
        # self.collect_includes(pop_cpp)
        self.get_ctor_lines(pop_cpp)

        self.fill_evalinfos()
        self.link_evaluators()
        self.wrap_toplevel_evaluators()
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_singleevaluator
    #--
    def write_singleevaluator(self, fOut, child, offset):
        fOut.write('\n%s<module name="SingleEvaluator"%s>\n'%(offset, ' id="%s"'%child["id"] if (id != "") else ''))

        if child["polyname"] != "":
            fOut.write('    %s<param name="%s" value="???0.0 0.0 1.0 1.0???" />\n'%(offset, child["polyname"]))
        #-- end if
        fOut.write("%s</module>\n"%(offset))    
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_shareevaluator
    #--
    def write_shareevaluator(self, fOut, child, offset):
        
        fOut.write('\n%s<module name="ShareEvaluator"%s>\n'%(offset, ' id="%s'%child["id"] if (id != "") else ''))
        fOut.write('    %s<param name="ShareEvaluator_%s_arrayname" value="???arrayname???" />\n'%(offset, child["id"]))
        fOut.write('    %s<param name="ShareEvaluator_%s_polyname"  value="???polyname???" />\n'%(offset, child["id"]))
        fOut.write('%s</module>\n'%(offset))    
    #-- end def


    #-----------------------------------------------------------------------------
    #-- write_multievaluator
    #--
    def write_multievaluator(self, fOut, child, offset):
        fOut.write('\n%s<module name="MultiEvaluator"%s>\n'%(offset, ' id="%s"'%child["id"] if (id != "") else ''))
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
        if (len(self.all_actions[action[0]]) > 0):
            pad_len = 4 + max([len(a) for a in self.all_actions[action[0]]])
        #-- end if
        
        fOut.write('\n%s<module name="%s"%s>\n'%(offset,action[0],' id="%s"'%action[1] if ((action[1] != "") or not self.bNoEmptyID) else ''))
        for a in self.all_actions[action[0]]:
            fOut.write('%s%s<param name=%s value="???0.0???" />\n'%(INDENT, offset, ('"'+a+'"').ljust(pad_len)))
        #- end for
        fOut.write('%s</module>\n'%offset)
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- write_priorities 
    #--
    def write_priorities(self, fOut, offset):
        pad_len1 = 0
        # all normal actions get their id appended enclosed in square brackets
        acnames = ['"%s%s"'%(a[0], "[%s]"%a[1] if a[1] != "" else "") for a in self.norm_actions]
        if (len(acnames) > 0):
            pad_len1 = 4 + max([len(a) for a in acnames])
        #-- end if
        
        pad_len2 = 0
        # all evaluators get their id appended enclosed in square brackets
        evnames = ['"%s[%s]"'%(self.evaluators[e]["type"], self.evaluators[e]["id"]) for e in self.evaluators]
        if (len(evnames) > 0):
            pad_len2 = 4 + max([len(a) for a in evnames])
        #-- end if

        # need padding for nice output
        pad_len =max(pad_len1,pad_len2)

        fOut.write('\n%s<priorities>\n'%offset)

        for a in acnames:
            fOut.write('%s%s<prio name=%s value="???0???" />\n'%(INDENT,offset, a.ljust(pad_len)))
        #-- end for

        for name in evnames:
            fOut.write('%s%s<prio name=%s value="???0???" />\n'%(INDENT,offset, name.ljust(pad_len)))
        #-- end for
    
        fOut.write('%s</priorities>\n'%offset)
    #-- end if


    #-----------------------------------------------------------------------------
    #-- write_xml
    #--
    def write_xml(self, class_name, species_name, bNoEmptyID, output_file):
        self.bNoEmptyID = bNoEmptyID

        try:
            fOut = open(output_file, "wt")
        
            fOut.write('<class name="%s" species_name="%s" species_id="0">\n'%(class_name, species_name))

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

