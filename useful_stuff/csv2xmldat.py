#!/usr/bin/python


import sys
import re


#-- name of the changeable attributes for the various groups
#-- there will be a set for 'sapiens' and a set for 'neander',
#-- but here we omit these suffices.
#--
attr_spc_names={'NPersZHybBirthDeathRel': ["HybBirthDeathRel_hybminprob"],
                                           

                'LocEnv':      ["NPPCap_K_max_",
                                "NPPCap_K_min_",
                                "NPPCap_NPP_max_",
                                "NPPCap_NPP_min_",
                                "NPPCap_coastal_factor_",
                                "NPPCap_coastal_max_latitude_",
                                "NPPCap_coastal_min_latitude_",
                                "NPPCap_water_factor_"],

                'PrivParamMix':["NPersHybBirthDeathRel_b0_" ,
                                "NPersHybBirthDeathRel_d0_" ,
                                "NPersHybBirthDeathRel_theta_" ,
                                "NPersHybBirthDeathRel_other_" ,
                                "NPersHybBirthDeathRel_this_" ,
                                "Fertility_min_age_" ,
                                "Fertility_max_age_" ,
                                "Fertility_interbirth_" ,
                                "NPersWeightedMove_" ,
                                "OAD_max_age_" ,
                                "OAD_uncertainty_" ,
                                "PrivParamMix_mode"],

                'Navigate':    ["Navigate_decay",
                                "Navigate_dist0",
                                "Navigate_min_dens",
                                "Navigate_bridge_prob",
                                "Navigate_prob0"]
                }

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
# the entry 'species' (0:sapiens, 1:neander) is needed to choose the values to be used for HybVal1 and HybVal2
SPECIES_INDEX=dat_min_entries.index("Species")

SEP=';'
DEF_WIDTH = 5
DEF_WHAT  = "xml+dat"

#-----------------------------------------------------------------------------
#-- collect vals
#--
#--  from the head line of the csv and a given line, create a dictionary
#--  associating attribute names with attribute values
#--
def collect_vals(csv_headers, csv_line):
    namval=None
    vals = csv_line.split(SEP)
    if len(vals) == len(csv_headers):
        namval = {}
        print("vals:%s"%vals)
        for i in range(len(vals)):
            print("namval[%s] <- %s"%(csv_headers[i].strip(), vals[i].strip()))
            namval[csv_headers[i].strip()] = vals[i].strip()
        #-- end for
    else:
        print("%d names, but %d values"%(len(csv_headers),len(vals)))
        print(vals)
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
#-- check_known
#--
#--  find all csv header entries that do not appear as a xml param name.
#--  Unknow headers in csv_ignore don't cause errors
#--
def check_known(csv_headers, xml_names, csv_ignores):
    headers_ok = True
    temp = []
    for h in csv_headers:
        if not h in xml_names:
            if not h in csv_ignores:
                headers_ok = False
            #-- end if
        #-- end if
    #-- end for

    #return headers_ok
    return True
#-- end def


#-----------------------------------------------------------------------------
#-- create_xml
#--
def create_xml(xml_template, namval, fout_xml):
    fxml_template=open(xml_template, 'r')

    # loop through the lines of the xml-template
    for line in fxml_template:
        #print("line [%s]:"%line)      
   
        # we need to remember if a line has been modified or not
        bModified = False

        # loop through group names 
        for n in attr_spc_names:
                    
            # loop through attribute name prefixes
            for x in attr_spc_names[n]:
                        
                # attribute names ending with '_' need a species suffix
                if (x.endswith('_')):
                    #print("  entry [%s]:"%x)      
                    #loop through species
                    for species in ['sapiens', 'neander']:
                        fullname = x + species
                        if  (fullname in namval):
                            m = re.match('(.*)(%s" *value=")(.*)(" */>)'%(fullname), line)
                            #print('    pattern [(.*)(%s" *value=")(.*)(" */>)]'%(fullname))
                            if not m is None:
                                #print("    got a match [%s%s%s%s\n"%(m[1], m[2], namval[fullname], m[4]))
                                fout_xml.write("%s%s%s%s\n"%(m[1], m[2], namval[fullname], m[4]))
                                bModified = True
                            #-- end if
                        #-- end if
                    #-- end for
                else:
                    # species independent attributes
                    if  (x in namval):
                        #print("x:%s. [%s]"%(x, line))
                        m = re.match('(.*)(%s" *value=")(.*)(" */>)'%x, line)
                        if not m is None:
                            #print("x:%s"%x)
                            # originally, ali gave log(hybminprob), but evenly spaced logs are very unevenly spaced values.
                            #if x == "HybBirthDeathRel_hybminprob":
                            #    fout_xml.write("%s%s%s%s\n"%(m[1], m[2], 10**float(namval[x]), m[4]))
                            #else:
                            fout_xml.write("%s%s%s%s\n"%(m[1], m[2], namval[x], m[4]))
                            #-- end if
                            bModified = True
                        #-- end if
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
#-- create_dat
#--
#-- we expect the template line to have the values
#--   Longitude Latitude LifeState AgentID BirthTime Gender Age LastBirth Species
#-- 
def create_dat(dat_template, namval, approach, fout_dat):

    fdat_template=open(dat_template, 'r')

    # loop through the lines of the dat-template
    for line in fdat_template:
        if line.startswith('#'):
            header = line.strip('# \n').split(SEP)
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
                vals0 = line.split(SEP)

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

# constants neededd tocheck if all required parameters have been given
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
    separator  = SEP

    req = MSK_ALL
    while (len(sys.argv) > cur+1) and (sys.argv[cur].startswith("-")):
        if (sys.argv[cur] == "-w"):
            width = int(sys.argv[cur+1])
        elif (sys.argv[cur] == "-a"):
            what = sys.argv[cur+1]
        elif (sys.argv[cur] == "-x"):
            xml_template = sys.argv[cur+1]
            req = req & (~MSK_XML)
        elif (sys.argv[cur] == "-d"):
            dat_template = sys.argv[cur+1]
            req = req & (~MSK_DAT)
        elif (sys.argv[cur] == "-c"):
            csv_file = sys.argv[cur+1]
            req = req & (~MSK_CSV)
        elif (sys.argv[cur] == "-o"):
            out_prefix = sys.argv[cur+1]
        #-- end if
        cur = cur + 2
    #-- end while
    actions = what.split('+')
    bDoXML = ("xml" in actions)
    bDoDAT = ("dat" in actions)

        
    if (req == 0):

        print("xmlt:%s, datt:%s, csv:%s, out:%s"%(xml_template,dat_template, csv_file, out_prefix))
        fcsv = open(csv_file, "rt")
        header = fcsv.readline()
        print("header: [%s]"%header.strip())
        count = 0
        csv_ignore=[]
        iResultCode = 0
        
        if (SEP in header):
            # go through every line of the csv file
            csv_headers = header.split(SEP)
            multiples=[]
            if check_multiples(csv_headers, multiples):
                #if check_known(csv_headers, attr_spc_names, csv_ignore):
                if True:
                    sim_idx = csv_headers.index('sim') if 'sim' in  csv_headers else -1
                    for csv_line in fcsv:
                        if (csv_line.strip()) :
                            if (not SEP in csv_line):
                                print("WARNING: no [%s] in line [%s]"%(SEP, csv_line))
                                iResultCode = -1
                                break
                            #- end if
                            if sim_idx >= 0:
                                count = int(csv_line.split(SEP)[sim_idx])
                            #-- end if

                            print("----- collect_vals -----")
                            # collect the values for this line
                            namval = collect_vals(csv_headers, csv_line)
                            print("namval:\n%s"%namval)

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
                                create_xml(xml_template, namval, fout_xml)
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
                                create_dat(dat_template, namval, approach, fout_dat)
                                print("----- closing dat file -----")
                                if not out_prefix is None:
                                    fout_dat.close()
                                #-- end if
                            #-- end if
                        #-- end if
                        print("done with #%d"%count) 
                        count = count + 1
                    #-- end for
                else:
                    print("ERROR: bad csv structure!")
                    print("csv entries not appear in xml param names")
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
            print("WARNING: no [%s] in header"%SEP)
        #- end if

    else:
        print("%s - turn csv to xml+dat files"%sys.argv[0])
        print("usage:")
        print("  %s [-w <width>] [-a <what>] -x <xml-template> -d <dat-template> -c <csv-file> [-o <out-body>]"%sys.argv[0])
        print("where")
        print("  width:         number of digits in output name (0-padded); default:%d"%DEF_WIDTH)
        print("  what:          do xml only, dat only or both. format: ('xml'|'dat')['+'(xml|dat)'] default:%s"%DEF_WHAT)
        print("  xml-template:  an XML attribute file to be used as template")
        print("  dat-template:  an dat agent data file to be used as template (all values (including header) ';'-separated)")
        print("  csv-file:      a CSV file containing attribute values")
        print("  out-body;      name template for output files")
    #-- end if
    sys.exit(iResultCode)
#-- end if (main)
