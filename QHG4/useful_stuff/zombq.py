#!/usr/bin/python

import xml.etree.ElementTree as ET

def show_node(node, indent):
    for child in node:
        print("%s%s: %s"%(indent, child.tag, child.attrib))
        show_node(child, indent+"  ")


def find_data(root, path, indent=""):
    value = None
    for child in root:
        #print("%s%s"%(indent,child.tag))
        if (value is None) and ((child.tag == "module") or (child.tag == "param")):
            name = path[0].split(":")
            if len(name) == 1:
                name.append("")
            #- end if
            #print("%sdoing %s for %s (%s)"%(indent,child.attrib["name"], name[0], path))
            if (len(path) == 1):
                if (child.attrib["name"] == name[0]):
                    #print("oinko")
                    if (not "id" in child.attrib) or (child.attrib["id"] == name[1]):
                        value=child.attrib["value"]
                        #print(value)
                        return value
                    #-- end if
                #-- end if    
            else:
                if (child.attrib["name"] == name[0]):
                    value = find_data(child, path[1:], indent+"  ")
                #-- end if
            #-- end if    
        #-- end if
    #- emd for     
    return value
#- end def

def find_all_data(root, paths):
    values = {}
    for path in paths:
        #print("doing %s"%path)
        x = find_data(root,path,"")
        #print("x:%s"%x)
        if (not x is None):
            values["*".join(path)] = x
        #-- end if
    #-- end for
    return values
#-- end def


def set_data(root, path,value,indent=""):
    
    for child in root:
        #print("%s%s"%(indent,child.tag))
        if  ((child.tag == "module") or (child.tag == "param")):
            name = path[0].split(":")
            if len(name) == 1:
                name.append("")
            #- end if
            #print("%sdoing %s for %s (%s)"%(indent,child.attrib["name"], name[0], path))
            if (len(path) == 1):
                if (child.attrib["name"] == name[0]):
                    #print("%soinks"%indent)
                    if (not "id" in child.attrib) or (child.attrib["id"] == name[1]):
                        #print("%swuff: %s"%(indent, value))
                        child.set("value", value)
                        #print(value)
                        return
                    else:
                        pass 
                        #print("%scomparing id to %s"%(ndent, name[1])) 
                    #-- end if
                #-- end if    
            else:
                if (child.attrib["name"] == name[0]):
                    value = set_data(child, path[1:], value,indent+"  ")
                #-- end if
            #-- end if    
        #-- end if
    #- emd for     
    return
#- end def

print("tree")
tree = ET.parse('/home/jody/progs/QHG4/tutorial_data/xmldat/tut_EnvironCapAlt.xml')
root = tree.getroot()


show_node(root,"")
p1 = ["MultiEvaluator:NPP+Alt", "SingleEvaluator:Alt", "AltPref"]
p2 = ["NPPCapacity", "NPPCap_K_max"]

x=find_data(root,p1)
print("value at %s: %s"%(p1,x))
print("changing value at %s"%p1)
set_data(root, ["MultiEvaluator:NPP+Alt", "SingleEvaluator:Alt", "AltPref"], "-0.2 0.0 0.2 0.02 2500 2.0 2002 2 3002 -9992")
x=find_data(root,["MultiEvaluator:NPP+Alt", "SingleEvaluator:Alt", "AltPref"])
print("value at %s: %s"%(p1,x))

#show_node(root,"")
print("extracting path-alues for %s and %s"%(p1, p2)) 
v = find_all_data(root,[p1, p2])
for w in v:
  print("%s : %s"%(w.split("*"), v[w]))
