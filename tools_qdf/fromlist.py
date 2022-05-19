#!/usr/bin/python

from sys import argv
from shutil import copyfile
import h5py
import os

import PopAttrs
from QHGError import QHGError

SEP=';'

#----------------------------------------------------------------------
#-- QDFFromList
#--
class QDFFromList:

    #----------------------------------------------------------------------
    #-- constructor
    #--  
    def __init__(self, base_qdf, csv_file, prefix):
        self.base_qdf   = base_qdf
        self.pop_name   = ""
        self.prefix     = prefix
        self.csv_file   = csv_file
        if (self.prefix.find('###') >= 0):
            self.bReplace = True
        else:
            self.bReplace = False
        #-- end if
        self.count = 0
    #-- end def


    #----------------------------------------------------------------------
    #-- make_all
    #-  
    def make_all(self):
        self.lineno = 0
        fIn = open(self.csv_file, "rt")
        
        line = fIn.readline()
        self.lineno = self.lineno + 1
        self.attr_names = line.split(SEP)
        print("attrnames (%d)\n  %s\n" % (len(self.attr_names), self.attr_names))
        print("---------")
        
        line = fIn.readline()
        self.lineno = self.lineno + 1
        self.attr_short = line.split(SEP)
        print("attr_short (%d)\n  %s\n" % (len(self.attr_short), self.attr_short))
        print(self.attr_short)
        
        line = fIn.readline().strip()
        self.lineno = self.lineno + 1
        #temp_order = [int(x) for x in line.split(SEP)]
        self.attr_order = inv([int(x) for x in line.split(SEP)])
        print("attr_order (%d)\n  %s\n" % (len(self.attr_order), self.attr_order))
        print(self.attr_order)
        
        # read the data
        line = fIn.readline()
        self.lineno = self.lineno + 1
        while line:
            self.make_for_line(line)
            line = fIn.readline()
            self.lineno = self.lineno + 1
        #-- end while

        fIn.close()
    #-- end def

    
    #----------------------------------------------------------------------
    #-- make_for_line
    #-  
    def make_for_line(self, line):
       
        attr_vals = line.split(SEP)

        # build name (ignore fragment if order number is negative or format string is '-')
        if (self.bReplace) :
            qdf_name = self.prefix.replace('###', str(self.count).zfill(5))
        else:
            qdf_name = self.prefix
        #-- end if
        self.count = self.count + 1
        
        for i in range(len(attr_vals)):
            p = self.attr_order[i]
            if (p >= 0):
                short_format = self.attr_short[p].strip()
                if (short_format != '-'):
                    print("i:%d; order[i]:%d; attr:[%s]" % (i, p, short_format))
                    qdf_name = qdf_name + "_" + (short_format % (float(attr_vals[p])))
                #-- end if
            #-- end if
        #-- end for
        qdf_name = qdf_name + ".qdf"

        if (os.path.exists(qdf_name)):
            print("[%d] %s already exists" % (self.lineno, qdf_name))
        #-- end if
        print("Making copy [%s]" % (qdf_name))
        # make a copy of the base qdf
        copyfile(self.base_qdf, qdf_name)

        print("Changing attributes")
        # change attribute values in copy
        pa = PopAttrs.PopAttrs(qdf_name)

        attrs = pa.read_pop_attrs(self.pop_name)
        if not attrs is None:
            attr_names  = []
            attr_values = []
            for i in range(len(attr_vals)):
                if (self.attr_names[i] != 'prefix'):
                    attr_names.append(self.attr_names[i].strip())
                    attr_values.append(attr_vals[i].strip())
                #-- end if
            #-- end for

            pa.change_attrs(self.pop_name,  attr_names, attr_values)
            pa.flush()
            pa.close()
        else:
            printf("Problem opening attributes")
            #QHGException
        #-- end if
    #-- end def

#-- end class


#-----------------------------------------------------------------------------
#-- usage
#--
def inv(perm):
    inverse = [-1] * len(perm)
    for i, p in enumerate(perm):
        if (p >= 0):
            inverse[p] = i
        #-- end if
    return inverse
#-- end def

#-----------------------------------------------------------------------------
#-- usage
#--
def usage(app):
    print("%s - create qdf files from parameter list" % (app))
    print("Usage;")
    print("  %s <base_qdf> <csv_file> <prefix>" % (app))
    print("where")
    print("  base_qdf   base qdf file whose copies will be modified")
    print("  csv_file   csv file containing name, formats and values for the modifications")
    print("  prefix     prefix for output files. If it contains \"###\" this will be replaced by a count")
    print("")
    print("Format of csv_file: ")
    print("  csv_file     ::= <header> <value_line>*")
    print("  header       ::= <name_line> <NL> <format_line> <NL> <order_line> <NL>")
    print("  name_line    ::= <attr_name> [ \",\" <attr_name>]* <NL>")
    print("  format_line  ::= <frag_format> [ \",\" <frag_format>]* <NL>")
    print("  order_line   ::= <order_num> [ \",\" <order_num>]* <NL>")
    print("  value_line   ::= <value> [ \",\" <value>]* <NL>")
    print("  attr_name    : name of attribute")
    print("  frag_format  : format string to build output-name fragment for attribute")
    print("                 usually something like \"b%0.2f\", \"N%03d\" etc")
    print("                 To omit fragment, use '-'.")
    print("  order_num    : defines ordering of output-name fragments")
    print("                 To omit fragment, use -1") 
    print("  value        : attribute value to use")
    print("")
    print("%s will create 1 qdf file per value_line and create name of the form" % (app))
    print("    <prefix>_<frag_1>_...<frag_n>.qdf  ")
    print("where frag_i is the result of formatting value[order[i]]  with format[order[i]]")
    print("")
    print("Example of a csv file:")
    print("  Verhulst_b0, NPPCap_K_min, WeightedMoveProb")
    print("  b%.2f,       K%02d,        P%0.1f")
    print("  0,           2,            1")
    print("  0.19,        5,            0.1")
    print("  0.20,        6,            0.1")
    print("  0.22,        5,            0.3")
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 1):
        try:
            base_qdf = argv[1]
            csv_file = argv[2]
            prefix   = argv[3]

            vava = QDFFromList(base_qdf, csv_file, prefix)

            vava.make_all()
            print("+++ created %d qdfs from csv"%vava.count)
        except QHGError as qhge:
            print("Error: [%s]" % (qhge))
        #-- end try

    else:
        print("oh-oh")
        usage(argv[0])
    #-- end if
#-- end main
