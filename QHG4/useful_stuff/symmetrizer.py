#!/usr/bin/python

from sys import argv

#-----------------------------------------------------------------------------
#-- InitSymmetrizer
#--
class InitSymmetrizer:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    #--  arr_desc: list of strings "group_name/array_name"
    #-- 
    def __init__(self, xml_file, dat_file):
        self.xml_file = xml_file
        self.dat_file = dat_file
        self.xml_outlines = []
        self.dat_outlines = []
        self.has_error = False
    #-- end def


    #-----------------------------------------------------------------------------
    #-- symmetrize_xml
    #--   go through lines :
    #--     if line contains "_neander" then
    #--       do nothing
    #--     if line contains "_sapiens" then
    #--       append line to outlines
    #--       replace "_sapiens" with _neander in line
    #--       append modified line to outlines
    #--     if line contains neither "_neander" nor "_sapiens" then
    #--       append line to outlines
    #--
    def symmetrize_xml(self):
        f = open(self.xml_file, "rt")
        all_lines = f.read()
        lines = all_lines.split('\n')
        self.xml_outlines = []

        for line in lines:
            pos = line.find("_sapiens")
            if pos > 0 :
                self.xml_outlines.append(line)
                # special case 1
                line_n = line.replace("_sapiens", "_neander")
                if "other" in line_n:
                    line_n = line_n.replace('value="neander"', 'value="sapiens"')
                #-- end if
                if "this" in line_n:
                    line_n = line_n.replace('value="sapiens"', 'value="neander"')
                #-- end if

                self.xml_outlines.append(line_n)
            else:
                pos = line.find("_neander")
                if pos < 0 :
                    self.xml_outlines.append(line)
                #-- end if
            #-- end if
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- symmetrize_dat
    #--   The "tail fields" are the fields following GenHybF.
    #--   we replace the tail fields of all lines with the
    #--   tail fields of the first sapiens line.
    #--   
    def symmetrize_dat(self):
        pos_GenHybM =  8
        pos_GenHybF =  9
        pos_rest    = 10

        # read lines and split them into list of list of fields
        f = open(self.dat_file, "rt")
        all_lines = f.read()
        lines = all_lines.split('\n')
        all_parts=[line.split(";") for line in lines]

        #save the tail fields of the first sapiens line
        tail = []
        for parts in all_parts:
            if (len(parts) > pos_GenHybF):
                if len(parts[0])>0:
                    if not parts[0][0].startswith("#"):
                        if (len(parts) > pos_GenHybF) and (int(parts[pos_GenHybM]) == 0) and (int(parts[pos_GenHybF]) == 0):
                            # have sapiens line
                            tail = [parts[i] for i in range(pos_rest, len(parts))]
                            break
                        #-- end if
                    #-- end if
                #--end if
            #--end if
        #-- end for

        # now replace the tail fields in all line with tail
        if len(tail) > 0:
            tail_line = ";".join(tail)
            for parts in all_parts:
                if (len(parts) > pos_GenHybF):
                    if len(parts[0])>0:
                        if parts[0][0].startswith("#"):
                            self.dat_outlines.append(";".join(parts))
                        elif (int(parts[pos_GenHybM]) in [0,1]) and (int(parts[pos_GenHybF]) in [0, 1]) :
                            # pure sapiens, neander, pure neander or
                            # first-generation hybrid (which should not really appear in the init file
                            self.dat_outlines.append(";".join(parts[0:pos_GenHybF]) + ";" + tail_line)
                        else:
                            print("invalid line: [%s]"%(";".join(parts)))
                            self.has_error = True
                        #-- end if
                    else:
                        print("empty first part: [%s]"%(";".join(parts)))
                        self.has_error = True
                        
                    #-- end if
                else:
                    if len(parts) > 1:
                        print("lineshas not enough parts: [%s]"%(";".join(parts)))
                        self.has_error = True
                    else:
                        self.dat_outlines.append('')
                    #-- end if
                #-- end if
            #-- end for
        else:
            # either there are non sapiens lines, or something did go wrong
            print("either there are non sapiens lines, or something did go wrong")
            self.has_error = True
        #-- end if
    #-- end def

    
            
    #-----------------------------------------------------------------------------
    #-- symmetrize_all
    #--  symmetrize xml and dat, and if ok write output files
    #--
    def symmetrize_all(self, out_body):
        if self.xml_file != '':
            self.symmetrize_xml()
            if not self.has_error:
                print("xml symmetrization ok")
            else:
                print("xml symmetrization failed")
            #-- end if
        #-- end if
        if not self.has_error:

            if self.dat_file != '':
                self.symmetrize_dat()
                if not self.has_error:
                    print("dat symmetrization ok")
                else:
                    print("dat symmetrization failed")
                #-- end if
            #-- end if
        #-- end if
        
        if not self.has_error:
            if self.xml_file != '':
                fxml = open(out_body+".xml", "wt")
                self.xml_outlines=map(lambda x:x+'\n', self.xml_outlines)
                fxml.writelines(self.xml_outlines)
                fxml.close()
                print("xml file written   ok")
            #-- end if
            
            if self.dat_file != '':
                fdat = open(out_body+".dat", "wt")
                self.dat_outlines=map(lambda x:x+'\n', self.dat_outlines)
                fdat.writelines(self.dat_outlines)
                fdat.close()
                print("dat file written   ok")
            #-- end if
        #-- end if
    #-- end def

#-- end class

#-----------------------------------------------------------------------------
#-- usage
#--
def usage():
    print()
    print("%s - symmetrizing xml and dat files"%argv[0])
    print("usage:")
    print("  %s [-x <xml_file>] [-d <dat_file>] -o <out_body>"%argv[0])
    print("where")
    print("  xml_file  input xml file")
    print("  dat_file  input dat file")
    print("  out_body  body for output file")

#-- end if

if len(argv) > 2:
    if (len(argv)%2) == 1:
        xml_file = ''
        dat_file = ''
        out_body = ''
        
        for i in range(1,len(argv)-1,2):
            if argv[i] == '-x':
                xml_file = argv[i+1]
            elif argv[i] == '-d':
                dat_file = argv[i+1]
            elif argv[i] == '-o':
                out_body = argv[i+1]
            else:
                print("unknown option '%s'"%argv[i])
            #-- end if
        #-- end for

        if out_body != '':
            isy = InitSymmetrizer(xml_file, dat_file)
            isy.symmetrize_all(out_body)
        else:
            print("the argument '-o <out-body>' is required")
            usage()
        #-- end if
    else:
        print("there should be an even number of arguments")
        usage()
    #-- end if
else:
    usage()
#-- end if
