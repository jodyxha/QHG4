#!/usr/bin/python

import sys
import os
import re
from xml.dom import minidom


dat_min_entries = ["Longitude",
                   "Latitude",
                   "LifeState",
                   "AgentID",
                   "BirthTime",
                   "Gender",
                   "Age",
                   "LastBirth",
                   "Species"]


# tag names for xml
TAG_CLASS    = 'class'
TAG_MODULE   = 'module'
TAG_PARAM    = 'param'
TAG_PRIOLIST = 'priorities'
TAG_PRIOITEM = 'prio'

DEF_SEP   = ';'
#-----------------------------------------------------------------------------
#-- QHGError
#--
class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class


#-----------------------------------------------------------------------------
#-- csv2xmldatbuilder
#--
class csv2xmldatbuilder:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, csv_file, xml_template, dat_template, xml_block_params, species):

        self.csv_file     = csv_file
        self.xml_template = xml_template
        self.dat_template = dat_template
        self.species      = species

        self.fcsv             = None
        self.name_values      = {}
        self.xml_params_flat  = {}
        self.csv_headers      = {}

        self.check_files_exist()
        self.create_csv_headers(self.csv_file)
        self.xml_params_flat  = self.get_xml_param_names(self.xml_template, xml_block_params)
    #-- end def


    #-----------------------------------------------------------------------------
    #-- check_files_exist
    #--
    def check_files_exist(self):
        bOK  = True
        errlist = []
        for f in [self.csv_file, self.xml_template, self.dat_template]:
            if os.path.exists(f):
                if not os.path.isfile(f):
                    errlist.append("  %s is not a file"%f)
                    bOK = False
                #-- end if
            else:
                errlist.append("  %s does not exist"%f)
                bOK = False
            #-- end if
        #-- end for
        if not bOK:
            raise QHGError("ERROR\n"+"\n".join(errlist))
        #-- end if
        return bOK
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- create_csv_headers
    #--
    def create_csv_headers(self, csv_file):
        try:
            self.fcsv = open(csv_file, "rt")
        except OSError as o:
            raise QHGError("OSError: %s"%o)
        #-- end try
        
        header = self.fcsv.readline()
        #print("header: [%s]"%header.strip())
        
        if (DEF_SEP in header):
            # go through every line of the csv file
            self.csv_headers = [x.strip() for x in header.split(DEF_SEP)]
            multiples=[]
            if self.check_multiples(self.csv_headers, multiples):
                iResultCode = 0
            else:
                sErr ="bad csv structure!\nFound duplicate entries for :"
                for h in multiples:
                    sErr = sErr + '\n  ' + h
                #-- end for
                iResultCode = -1
                raise QHGError(sErr)
            #-- end if
        else:
            iResultCode = -1
            raise QHGError("WARNING: no [%s] in header"%DEF_SEP)
        #- end if
        return iResultCode
    #-- end def


    #-----------------------------------------------------------------------------
    #-- check_multiples
    #--
    #--  find all multiple entries in the from the head line of the csv and a given line, create a dictionary
    #--  associating attribute names with attribute values
    #--
    def check_multiples(self, csv_headers, multiples):
        headers_ok = True
        temp = []
        for h in csv_headers:
            if h in temp:
                multiples.append(h)
                headers_ok = False
            else:
                temp.append(h)
            #-- end if
        #-- end for
    
        return headers_ok
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- get_xml_param_names
    #--
    def get_xml_param_names(self, xml_file, xml_block_params):

        self.xml_params_flat = {}
        class_contents = {}
        tree = minidom.parse(xml_file)
        
        classes = tree.getElementsByTagName(TAG_CLASS);
        
        for c in classes:
            attr_names = {}
            class_name = c.attributes['name'].value;
            
            modules = c.getElementsByTagName(TAG_MODULE)
            #print("found %d modules"%len(modules))
            for m in modules:
                module_name = m.attributes['name'].value
                param_list={}
                #print("module [%s]"%module_name)
                params=m.getElementsByTagName(TAG_PARAM)
                #print("found %d params"%len(params))
                for p in params:
                    #print("param [%s]"%p.attributes['name'].value)
                    n=p.attributes['name'].value
                    if not n in xml_block_params:
                        v=p.attributes['value'].value
                        param_list[n] = v
                    else:
                        print("param [%s] blocked!"%p.attributes['name'].value)
                    #-- end if
                 #-- end for
                attr_names[module_name] = param_list
            #-- end for
            class_contents[class_name] = attr_names
        #-- end for

        for c in class_contents:
            for m in class_contents[c]:
                for t in class_contents[c][m]:
                    #print("    param %s:%s"%(t,class_contents[c][m][t]))
                    self.xml_params_flat[t]=class_contents[c][m][t]
                #-- end for
            #-- end for
        #-- end for
        return self.xml_params_flat
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- get_csv_params
    #--
    def get_csv_params(self, csv_line, csv_ignore_params):
        self.name_values = None
        bErr = False
        sErr = ""
        vals = csv_line.split(DEF_SEP)
        #print("csvheaders:%s"%self.csv_headers)
        #print("vals:%s"%vals)
        if len(vals) == len(self.csv_headers):
            self.name_values = {}
            #print("vals:%s"%vals)
            for i in range(len(vals)):
                if self.csv_headers[i] in self.xml_params_flat:
                    #print("namval[%s] <- %s"%(self.csv_headers[i].strip(), vals[i].strip()))
                    self.name_values[self.csv_headers[i].strip()] = vals[i].strip()
                else:
                    if not self.csv_headers[i] in csv_ignore_params:
                        if sErr != "":
                            sErr = sErr + '\n'
                        #-- end of
                        sErr = sErr + "  tried to set an unknown or blocked param [%s]"%self.csv_headers[i]
                        #print("tried to set an unknown or blocked param [%s]"%self.csv_headers[i])
                        bErr = True
                    #-- end if
                #-- end if
            #-- end for
        else:
            print("%d names, but %d values"%(len(self.csv_headers),len(vals)))
            print(vals)
        #-- end if
        if bErr:
            self.name_values = None
            raise QHGError("Accessed blocked params:\n" + sErr)
        #-- end if
        return self.name_values
    #-- end def


    #-----------------------------------------------------------------------------
    #-- fill_name_values
    #--
    def fill_name_values(self):
        for p in self.xml_params_flat:
            if not p in self.name_values:
                #print("setting namvals[%s] = %s"%(p, self.xml_params_flat[p]))
                self.name_values[p] = self.xml_params_flat[p]
            #-- end if
        #-- end for
    #-- end def


    #-----------------------------------------------------------------------------
    #-- create_xml
    #--
    def create_xml(self, csv_ignore_params, fout_xml):
        fxml_template=open(self.xml_template, 'r')

        # loop through the lines of the xml-template
        for line in fxml_template:
            #print("line [%s]:"%line.strip())      

            # we need to remember if a line has been modified or not
            bModified = False

            #print("xml_params_flat: %s"%self.xml_params_flat)
            # loop through module names 
            for n in self.xml_params_flat:

                if not n in csv_ignore_params:
                        
                    #print("n:%s. [%s]"%(n, line))
                    m = re.match('(.*)(%s" *value=")(.*)(" */>)'%n, line)
                    if not m is None:
                        fout_xml.write("%s%s%s%s\n"%(m[1], m[2], self.name_values[n], m[4]))
                        bModified = True
                    #-- end if
                #-- end if
            
            #-- end for
        
            # if we have not done any modification, return original line
            if not bModified:
                fout_xml.write(line)
            #-- end if
        #-- end for
        fxml_template.close()
    #-- end def


    #-----------------------------------------------------------------------------
    #-- create_dat
    #--
    #-- we expect the template line to have the values
    #--   Longitude Latitude LifeState AgentID BirthTime Gender Age LastBirth Species
    #-- 
    def create_dat(self, additional_entries, fout_dat):

        fdat_template=open(self.dat_template, 'r')
        iSpeciesIndex = -1
        # loop through the lines of the dat-template
        for line in fdat_template:
            if line.startswith('#'):
                header = line.strip('# \n').split(DEF_SEP)
                # make sure the minimal equired params are present
                
                if header == dat_min_entries:
                    iSpeciesIndex = dat_min_entries.index("Species")

                    header.remove('Species')
                    header.extend(list(additional_entries))
                    fout_dat.write("#%s\n"%(';'.join(header)))
                else:
                    print("Header inconsistent!")
                    print("Expected: %s"%dat_min_entries)
                    print("Got:      %s"%header)
                    break
                #-- end if
            else:
                if (line.strip(" \n") != ''):
                    # grab the values already there
                    vals0 = line.strip('\n').split(DEF_SEP)

                    if (iSpeciesIndex >= 0):
                        vals0.pop(iSpeciesIndex)
                    #-- end if

                    # now the values for the hyb data
                    for n in additional_entries:
                        if n in self.name_values:
                            vals0.append(self.name_values[n])
                        else:
                            # try name with species suffix
                            n1 = n+"_"+self.species
                            if n1 in self.name_values:
                                vals0.append(self.name_values[n1])
                            else:
                                vals0.append(additional_entries[n])
                            #-- end if
                        #-- end if

                    #-- end for

                    fout_dat.write("%s\n"%(';'.join(vals0)))
                #-- end if
            #-- end if
        #-- end for

        fdat_template.close()

     #-- end def




    #-----------------------------------------------------------------------------
    #-- creation_loop
    #--
    def creation_loop(self,csv_ignore_params, additional_entries, out_prefix, bDoXML, bDoDAT, width):
        count = 0
        for csv_line in self.fcsv:
            if (csv_line.strip()) :
                if (not DEF_SEP in csv_line):
                    print("WARNING: no [%s] in line [%s]"%(DEF_SEP, csv_line))
                    iResultCode = -1
                    break
                #- end if

                print("----- collect_vals -----")
                # collect the values for this line
                self.get_csv_params(csv_line, csv_ignore_params)
                if not self.name_values is None:
                    self.fill_name_values()
                #-- end if
                #print("namval:\n%s"%self.name_values)
                if not self.name_values is None:
                    if (bDoXML):
                        if out_prefix is None:
                            fout_xml  = sys.stdout
                            sname_xml = "(stdout)"
                        else:    
                            sname_xml = "%s_%0*d.xml"%(out_prefix, width, count)
                            try:
                                fout_xml  = open(sname_xml, 'w')
                            except OSError as o:
                                raise QHGError("OSError: %s"%o)
                            #-- end if
                        #-- end if
                        print("----- create_xml [%s] -----"%(sname_xml))
                        # create an xml for this line (using the xml template) 
                        self.create_xml(csv_ignore_params, fout_xml)
                        print("----- closing xml file -----")
                        if not out_prefix is None:
                            fout_xml.close()
                        #-- end if
                    #-- end if

                    if (bDoDAT):
                        if out_prefix is None:
                            fout_dat  = sys.stdout
                            sname_dat = "(stdout)"
                        else:    
                            sname_dat = "%s_%0*d.dat"%(out_prefix, width, count)
                            try:
                                fout_dat  = open(sname_dat, 'w')
                            except OSError as o:
                                raise QHGError("OSError: %s"%o)
                            #-- end if
                        #-- end if
                        print("----- create_dat [%s] -----"%sname_dat)
                        # create an dat for this line (using the dat template) 
                        self.create_dat(additional_entries, fout_dat)
                        print("----- closing dat file -----")
                        if not out_prefix is None:
                            fout_dat.close()
                        #-- end if
                    #-- end if
                else:
                    print("bad param used")
                    iResultCode = -1 
                #-- end if
            #-- end if
            print("done with #%d"%count) 
            count = count + 1
        #-- end for
    #-- end def

#-- end class

    
