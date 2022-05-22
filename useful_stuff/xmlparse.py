#!/usr/bin/python

from sys import argv
from xml.dom import minidom

NAME_CLASS    = 'class'
NAME_MODULE   = 'module'
NAME_PARAM    = 'param'
NAME_PRIOLIST = 'priorities'
NAME_PRIOITEM = 'prio'

def getXMLParamNames(xml_file):
   
    class_contents = {}
    tree = minidom.parse(argv[1])

    classes = tree.getElementsByTagName(NAME_CLASS);
    
    for c in classes:
        attr_names = {}
        class_name = c.attributes['name'].value;
        
        modules = c.getElementsByTagName(NAME_MODULE)
        #print("found %d modules"%len(modules))
        for m in modules:
            module_name = m.attributes['name'].value
            param_list={}
            #print("module [%s]"%module_name)
            params=m.getElementsByTagName(NAME_PARAM)
            #print("found %d params"%len(params))
            for p in params:
                #print("param [%s]"%p.attributes['name'].value)
                n=p.attributes['name'].value
                v=p.attributes['value'].value
                param_list[n] = v
            #-- end for
            attr_names[module_name] = param_list
        #-- end for
        class_contents[class_name] = attr_names
    #-- end for
    return class_contents
#-- end if


if len(argv) > 1:

    class_contents = getXMLParamNames(argv[1])
    
    for c in class_contents:
        print("class %s:"%c)
        for m in class_contents[c]:
            print("  module %s:"%m)
            for t in class_contents[c][m]:
                print("    param %s:%s"%(t,class_contents[c][m][t]))
            #-- end for
        #-- end for
    #-- end for
    attr_names = class_contents[list(class_contents)[0]]
    for m in attr_names:
        print("%s:"%m)
        for t in attr_names[m]:
            print("  %s"%t)
        #-- end for
    #-- end for
    
else:
    tree = minidom.parse(argv[1])

    classes = tree.getElementsByTagName(NAME_CLASS);
    print("found %d classes"%len(classes))

    for c in classes:
        class_name = c.attributes['name'].value;
        print("classs [%s]"%class_name)
        
        modules = c.getElementsByTagName(NAME_MODULE)
        print("found %d modules"%len(modules))
        for m in modules:
            module_name = m.attributes['name'].value
            param_list=[]
            print("module [%s]"%module_name)
            params=m.getElementsByTagName(NAME_PARAM)
            print("found %d params"%len(params))
            for p in params:
                print("param [%s]"%p.attributes['name'].value)
                param_list.append(p.attributes['name'].value)
            #-- end for
            attr_names[module_name] = param_list
        #-- end for
        priolist = c.getElementsByTagName(NAME_PRIOLIST)
        print("found %d priolists"%len(priolist))
        for p in priolist:
            print("found priolist")
            prioitems = p.getElementsByTagName(NAME_PRIOITEM)
            print("found %d prioitems"%len(prioitems))
            for t in prioitems:
                print("prio [%s]"%t.attributes['name'].value)
            #-- end for
        #-- end for
    #-- end for

    for m in attr_names:
        print("%s:"%m)
        for t in attr_names[m]:
            print("  %s"%t)
        #-- end for
    #-- end for
#else:
#    print("%s <xml-file>"%argv[0])
#-- end if
         
