#!/usr/bin/python

from sys import argv
import os
import re

from PopXMLCreator import PopXMLCreator,QHGError

#-----------------------------------------------------------------------------
#--  ParamError
#--
class ParamError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class

#---------------------------------------------------------------------------
#-- read_params
#--  paramdefs: dictionary opt => [name, default]
#--             
def read_params(argv, param_defs):
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
#-- show_params
#--
def show_params(params):
    print("Params:")
    for k in params:
        print("  '%-10s[%s]'" % (k, params[k]))
    #-- end for
    
#-- end def

#-----------------------------------------------------------------------------
#-- usage
#--
def usage(app):
    print("%s - create an XM population file from a population cpp filre"%app)
    print("usage:")
    print("  %s -a <action_dir> -p <pop_cpp> -o <output_file> [-n <noemptyid>]"%app)
    print("where")
    print("  action_dir   directory containing the action source code") 
    print("  pop_cpp      cpp for the population the XML file has to be created for")
    print("  output_file  name of the output file")
    print("  noemptyid    hide empty ids in output (\"on\" or \"off\"")
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if len(argv) > 3:

    # check /home/jody/progs/QHG4/genes/pcomulti.py
    paramdefs = {
        "-a" : ["actiondir", None],
        "-p" : ["pop_cpp", None],
        "-o" : ["output", None],
        "-n" : ["noempty", "off"],
        "-s" : ["species", "sapiens"]
    }
    try:
        params = read_params(argv, paramdefs)
    except ParamError as e:
        print("\nParamError: %s" % (e))
        usage(argv[0])
    else:           
        show_params(params)

        action_dir  = params["actiondir" ]
        pop_file    = params["pop_cpp"]
        output_file = params["output"]
        no_empty    = params["noempty"].lower()=="on"
        species     = params["species"]


        class_name = re.search("(\w+)", os.path.basename(pop_file)).groups()[0]
        try:
            pxc = PopXMLCreator(action_dir, pop_file)
            pxc.write_xml(class_name, species, no_empty, output_file)
            print("The XML file '%s' for the population '%s' has been successfully written."%(output_file, class_name))
            print("In order to use the file '%s' inb QHG, remember to replace all values of the form '???0???' or '???0.0???' with the values you need."%output_file)
        except QHGError as e:
            print("QHG Error: %s"%e)
        #-- end try
    #-- end try
    
else:
    usage(argv[0])
#- end if
