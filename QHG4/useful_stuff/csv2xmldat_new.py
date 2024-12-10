#!/usr/bin/python


import sys
import re
from xml.dom import minidom

# entries to be appended to lines in dat file, with default values
dat_new_entries = {"NPersWeightedMove":           '0.0885986328125',
                   "OAD_max_age":                 '69.404296875',     
                   "OAD_uncertainty":             '0.1',                  
                   "Fertility_min_age":           '13.983398',            
                   "Fertility_max_age":           '44.94629',           
                   "Fertility_interbirth":        '2.2236328',      
                   "NPersHybBirthDeathRel_b0":    '0.2585693359375',
                   "NPersHybBirthDeathRel_d0":    '0.020107421875',
                   "NPersHybBirthDeathRel_theta": '0.2585693359375'}



# for ali approach:
#   HybVal1 = NeaFrac
#   HybVal2 = NeaSDev
# for zolli approach:
#   HybVal1 = GenHybM
#   HybVal2 = GenHybF
dat_hyb_entries = {'ali':   ['NeaFrac', 'NeaSDev'],
                   'zolli': ['GenHybM', 'GenHybF']}
species_names = ['sapiens', 'neander']
hyb_vals = {'ali':   {'neander': ['1', '0.02'], 'sapiens': ['0', '0.02']},
            'zolli': {'neander': ['1', '1'],    'sapiens': ['0', '0']}}


dat_min_entries = ["Longitude",
                   "Latitude",
                   "LifeState",
                   "AgentID",
                   "BirthTime",
                   "Gender",
                   "Age",
                   "LastBirth",
                   "Species"]

additional_entries = {"GenHybM":                     '1',
                      "GenHybF":                     '1',
                      "NPersWeightedMove":           '0.0885986328125',
                      "OAD_max_age":                 '69.404296875',     
                      "OAD_uncertainty":             '0.1',                  
                      "Fertility_min_age":           '13.983398',            
                      "Fertility_max_age":           '44.94629',           
                      "Fertility_interbirth":        '2.2236328',      
                      "NPersHybBirthDeathRel_b0":    '0.2585693359375',
                      "NPersHybBirthDeathRel_d0":    '0.020107421875',
                      "NPersHybBirthDeathRel_theta": '0.2585693359375'}




# the entry 'species' (0:sapiens, 1:neander) is needed to choose the values to be used for HybVal1 and HybVal2
SPECIES_INDEX=dat_min_entries.index("Species")

DEF_SEP   = ';'
DEF_WIDTH = 5
DEF_WHAT  = "xml+dat"

# tag names for xml
TAG_CLASS    = 'class'
TAG_MODULE   = 'module'
TAG_PARAM    = 'param'
TAG_PRIOLIST = 'priorities'
TAG_PRIOITEM = 'prio'

#-----------------------------------------------------------------------------
#-- getXMLParamNames vals
#--
#--  get all the param names for each module in each class.
#--  return as {class_name => {module_name => [paramlist] } }
#--
def getXMLParamNames(xml_file, xml_block_params):
   
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
    return class_contents
#-- end if


#-----------------------------------------------------------------------------
#-- collect vals
#--
#--  from the head line of the csv and a given line, create a dictionary
#--  associating attribute names with attribute values.
#--  The param names must be either in attr_names_list or in csv_ignore_params
#--  for the name to be added to namval
#--
def collect_vals(csv_headers, csv_line, attr_names_list, csv_ignore_params):
    namval=None
    bErr = False
    vals = csv_line.split(DEF_SEP)
    if len(vals) == len(csv_headers):
        namval = {}
        #print("vals:%s"%vals)
        for i in range(len(vals)):
            if csv_headers[i] in attr_names_list:
                print("namval[%s] <- %s"%(csv_headers[i].strip(), vals[i].strip()))
                namval[csv_headers[i].strip()] = vals[i].strip()
            else:
                if not csv_headers[i] in csv_ignore_params:
                    print("tried to set an unknown or blocked param [%s]"%csv_headers[i])
                    bErr = True
                #-- end if
            #-- end if
        #-- end for
    else:
        print("%d names, but %d values"%(len(csv_headers),len(vals)))
        print(vals)
    #-- end if
    if bErr:
        namval = None
    #-- end if
    return namval
#-- end def


#-----------------------------------------------------------------------------
#-- fill_namvals
#--
#--  add all xml param values which are not already set in namvals
#--
def fill_namvals(namvals, attr_names_flat):
    #for m in attr_names:
    # 
    #    pl = list(attr_names[m])
    #
    #    for p in pl:
    #        if not p in namvals:
    #            print("setting namvals[%s] = %s"%(p, attr_names[m][p]))
    #            namvals[p] = attr_names[m][p]
    #        #-- end if
    #    #-- end for
    #-- end for
    for p in attr_names_flat:
        if not p in namvals:
            print("setting namvals[%s] = %s"%(p, attr_names_flat[p]))
            namvals[p] = attr_names_flat[p]
        #-- end if
    return namval
#-- end def


#-----------------------------------------------------------------------------
#-- check_multiples
#--
#--  find all multiple entries in the from the head line of the csv and a given line, create a dictionary
#--  associating attribute names with attribute values
#--
def check_multiples(csv_headers, mutiples):
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
#-- create_xml
#--
def create_xml(xml_template, namval, attr_names, csv_ignore_params, fout_xml):
    fxml_template=open(xml_template, 'r')

    # loop through the lines of the xml-template
    for line in fxml_template:
        #print("line [%s]:"%line.strip())      
   
        # we need to remember if a line has been modified or not
        bModified = False

        # loop through mofule names 
        for n in attr_names:
                    
            # loop through attribute names
            for x in attr_names[n]:
                if not x in csv_ignore_params:
               
                    #print("x:%s. [%s]"%(x, line))
                    m = re.match('(.*)(%s" *value=")(.*)(" */>)'%x, line)
                    if not m is None:
                        #print("x:%s"%x)
                        # originally, ali gave log(hybminprob), but evenly spaced logs are very unevenly spaced values.
                        #if x == "HybBirthDeathRel_hybminprob":
                        #    fout_xml.write("%s%s%s%s\n"%(m[1], m[2], 10**float(namval[x]), m[4]))
                        #else:
                        fout_xml.write("%s%s%s%s\n"%(m[1], m[2], namval[x], m[4]))
                        bModified = True
                    #-- end if
                #-- end if

            #-- end for
        #-- end for
        # if we have not done any modification, return original line
        if not bModified:
            fout_xml.write(line)
        #-- end if
    #-- end for
    fxml_template.close()
#-- end def


#-----------------------------------------------------------------------------
#-- create_dat_old
#--
#-- we expect the template line to have the values
#--   Longitude Latitude LifeState AgentID BirthTime Gender Age LastBirth Species
#-- 
def create_dat_old(dat_template, namval, approach, fout_dat):

    fdat_template=open(dat_template, 'r')

    # loop through the lines of the dat-template
    for line in fdat_template:
        if line.startswith('#'):
            header = line.strip('# \n').split(DEF_SEP)
            if header == dat_min_entries:
                header.remove('Species')
                header.extend(dat_hyb_entries[approach])
                header.extend([x for x in dat_new_entries])
                fout_dat.write("#%s\n"%(';'.join(header)))
            else:
                print("Header inconsistent!")
                print("Expected: %s"%dat_min_entries)
                print("Got:      %s"%header)
                break
            #-- end if
        else:
            if (line.strip() != ''):
                # grab the values already there
                vals0 = line.split(DEF_SEP)

                species = species_names[int(vals0.pop(SPECIES_INDEX))]

                # add the ali/zolli-specific vars
                vals0.extend(hyb_vals[approach][species])

                # now the values for the hyb data
                for n in dat_new_entries:
                    n1 = n+"_"+species
                    if n1 in namval:
                        vals0.append(namval[n1])
                    else:
                        vals0.append(dat_new_entries[n])
                #-- end for
                fout_dat.write("%s\n"%(';'.join(vals0)))
            #-- end if
        #-- end if
    #-- end for

    fdat_template.close()

#-- end def

#-----------------------------------------------------------------------------
#-- create_dat
#--
#-- we expect the template line to have the values
#--   Longitude Latitude LifeState AgentID BirthTime Gender Age LastBirth Species
#-- 
def create_dat(dat_template, namval, additional_namvals, fout_dat):

    fdat_template=open(dat_template, 'r')

    # loop through the lines of the dat-template
    for line in fdat_template:
        if line.startswith('#'):
            header = line.strip('# \n').split(DEF_SEP)
            if header == dat_min_entries:
                header.remove('Species')
                header.extend(list(additional_namvals))
                fout_dat.write("#%s\n"%(';'.join(header)))
            else:
                print("Header inconsistent!")
                print("Expected: %s"%dat_min_entries)
                print("Got:      %s"%header)
                break
            #-- end if
        else:
            if (line.strip() != ''):
                # grab the values already there
                vals0 = line.split(DEF_SEP)

                species = species_names[int(vals0.pop(SPECIES_INDEX))]

                # now the values for the hyb data
                for n in additional_namvals:
                    if n in namval:
                        vals0.append(namval[n])
                    else:
                        n1 = n+"_"+species
                        if n1 in namval:
                            vals0.append(namval[n1])
                        else:
                            vals0.append(additional_namvals[n])
                        #-- end if
                    #-- end if
                #-- end for
                fout_dat.write("%s\n"%(';'.join(vals0)))
            #-- end if
        #-- end if
    #-- end for

    fdat_template.close()

#-- end def

MSK_XML = 1
MSK_DAT = 2
MSK_CSV = 4
MSK_ALL = 7

if __name__ == '__main__':

    iResultCode = -1

    out_prefix = None
    width      = DEF_WIDTH
    what       = DEF_WHAT
    approach   = "zolli"
    cur        = 1
    xml_block_params  = []
    csv_ignore_params = []
    
    req = MSK_ALL
    while (len(sys.argv) > cur+1) and (sys.argv[cur].startswith("-")):
        if (sys.argv[cur] == "-w"):
            # width for numbering
            width = int(sys.argv[cur+1])
        elif (sys.argv[cur] == "-a"):
            # what oututs to create
            what = sys.argv[cur+1]
        elif (sys.argv[cur] == "-x"):
            # xml template file
            xml_template = sys.argv[cur+1]
            req = req & (~MSK_XML)
        elif (sys.argv[cur] == "-d"):
            # dat template file
            dat_template = sys.argv[cur+1]
            req = req & (~MSK_DAT)
        elif (sys.argv[cur] == "-c"):
            # csv file
            csv_file = sys.argv[cur+1]
            req = req & (~MSK_CSV)
        elif (sys.argv[cur] == "-o"):
            # output subdir
            out_prefix = sys.argv[cur+1]
        elif (sys.argv[cur] == "-b"):
            # xml block list
            xml_block_params = sys.argv[cur+1].split(':')
        elif (sys.argv[cur] == "-i"):
            # csv ignore list
            print("got arg[-i]:%s"%sys.argv[cur+1])
            csv_ignore_params = sys.argv[cur+1].split(':')
        #-- end if
        cur = cur + 2
    #-- end while
    actions = what.split('+')
    bDoXML = ("xml" in actions)
    bDoDAT = ("dat" in actions)

        
    if (req == 0):
        print("xml_block:[%s]"%xml_block_params)
        
        print("xmlt:%s, datt:%s, csv:%s, out:%s"%(xml_template,dat_template, csv_file, out_prefix))
        class_contents = getXMLParamNames(xml_template, xml_block_params)
        # to simplify things, we create a flat dictionary holding all param_value oairs
        attr_names_flat = {}
        print("xemelli")
        for c in class_contents:
            for m in class_contents[c]:
                for t in class_contents[c][m]:
                    #print("    param %s:%s"%(t,class_contents[c][m][t]))
                    attr_names_flat[t]=class_contents[c][m][t]
                #-- end for
            #-- end for
        #-- end for
        attr_names = class_contents[list(class_contents)[0]]
        
        fcsv = open(csv_file, "rt")
        header = fcsv.readline()
        print("header: [%s]"%header.strip())
        count = 0

        iResultCode = 0
        
        if (DEF_SEP in header):
            # go through every line of the csv file
            csv_headers = [x.strip() for x in header.split(DEF_SEP)]
            multiples=[]
            if check_multiples(csv_headers, multiples):
            
                sim_idx = csv_headers.index('sim') if 'sim' in  csv_headers else -1
                for csv_line in fcsv:
                    if (csv_line.strip()) :
                        if (not DEF_SEP in csv_line):
                            print("WARNING: no [%s] in line [%s]"%(DEF_SEP, csv_line))
                            iResultCode = -1
                            break
                        #- end if
                        if sim_idx >= 0:
                            count = int(csv_line.split(DEF_SEP)[sim_idx])
                        #-- end if

                        print("----- collect_vals -----")
                        # collect the values for this line
                        namval = collect_vals(csv_headers, csv_line, attr_names_flat, csv_ignore_params)
                        if not namval is None:
                            namval = fill_namvals(namval, attr_names_flat)
                        #-- end if
                        print("namval:\n%s"%namval)
                        if not namval is None:
                            if (bDoXML):
                                if out_prefix is None:
                                    fout_xml  = sys.stdout
                                    sname_xml = "(stdout)"
                                else:    
                                    sname_xml = "%s_%0*d.xml"%(out_prefix, width, count)
                                    fout_xml  = open(sname_xml, 'w')
                                #-- end if
                                print("----- create_xml [%s] -----"%(sname_xml))
                                # create an xml for this line (using the xml template) 
                                create_xml(xml_template, namval, attr_names, csv_ignore_params, fout_xml)
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
                                    fout_dat  = open(sname_dat, 'w')
                                #-- end if
                                print("----- create_dat [%s] -----"%sname_dat)
                                # create an dat for this line (using the dat template) 
                                create_dat(dat_template, namval, additional_entries, fout_dat)
                                #create_dat(dat_template, namval, approach, fout_dat)
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
            else:
                print("ERROR: bad csv structure!")
                print("Found duplicate entries for :")
                for h in multiples:
                    print("  %s"%h)
                #-- end for
                iResultCode = -1
            #-- end if
        else:
            iResultCode = -1
            print("WARNING: no [%s] in header"%DEF_SEP)
        #- end if

    else:
        appname = sys.argv[0].split('/')[-1]
        applen  = len(appname)
        print("%*s - turn csv to xml+dat files"%(applen, appname))
        print("usage:")
        print("  %s [-w <width>] [-a <what>] -x <xml-template> -d <dat-template> -c <csv-file>"%sys.argv[0])
        print("         [-o <out-body>] [-b <xml-block-list>] [-i <csv-ignore-list>]")
        print("where")
        print("  width:            number of digits in output name (0-padded); default:%d"%DEF_WIDTH)
        print("  what:             do xml only, dat only or both. format: ('xml'|'dat')['+'(xml|dat)'] default:%s"%DEF_WHAT)
        print("  xml-template:     an XML attribute file to be used as template")
        print("  dat-template:     an dat agent data file to be used as template (all values (including header) ';'-separated)")
        print("  csv-file:         a CSV file containing attribute values")
        print("  out-body:         name template for output files")
        print("  xml-block-list:   list of param names in the xml file which must not be modified")
        print("  csv-ignore-list:  list of param names in the csv file which must not be used")
        print()
        print("Example:")
        
        print("  %*s test.csv -w 5 -x xml_tmpl_nea.xml.xml  -d dat_tmpl_nea.dat -c test.csv -o test_body"%(applen, appname))
        print("  %*s -b NPPCap_K_min_sapiens:NPPCap_NPP_min_sapiens"%(applen, " "))
        print("  %*s -i csv_ignore=maternal_sdev_hyb_sapiens:paternal_sdev_hyb_sapiens:maternal_sdev_hyb_neander:paternal_sdev_hyb_neander:NPPCap_efficiency_neander:Navigate_decay"%(applen," "))
    #-- end if
   
    print("@@TODO Need to think about a generalisation of dat creation!!!!!")
    print("@@TODO Less hardwired code!!!!!")
    print("@@TODO Make csv2xmldat_new.py work for all files (hyb or not)!!!!!")
    sys.exit(iResultCode)
#-- end if (main)
