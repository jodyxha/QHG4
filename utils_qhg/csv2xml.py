#!/usr/bin/python


import sys
import re


#-- name of the changeable attributes for the various groups
#-- there will be a set for 'sapiens' and a set for 'neander',
#-- but here we omit these suffices.
#--
attr_spc_names={'LocEnv':      ["NPPCap_K_max_",
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


#-----------------------------------------------------------------------------
#-- collect vals
#--
#--  from the head line of the csv and a given line, create a dictionary
#--  associating attribute names with attribute values
#--
def collect_vals(csv_headers, csv_line):
    namval={}
    vals = csv_line.split(SEP)
    if len(vals) == len(csv_headers):
        namval = {}
        for i in range(len(vals)):
            namval[csv_headers[i].strip()] = vals[i].strip()
        #-- end for
    else:
        namval=None
        print("%d names, but %d values"%(len(csv_headers),len(vals)))
        print(vals)
    #-- end if
    return namval
#-- end def


#-----------------------------------------------------------------------------
#-- create_xml
#--
def create_xml(xml_template, namval, fout_xml):
    fxml_template=open(xml_template, 'r')

    # loop through the lines of the xml-template
    for line in fxml_template:
        m = re.match('(.*)value="(.*)(" */>)', line)
        if not m is None:
                        
            # we need to remember if a line has been modified or not
            bModified = False

            # loop through group names 
            for n in attr_spc_names:

                # loop through attribute name prefixes
                for x in attr_spc_names[n]:
                    
                    # attribute names ending with '_' need a species suffix
                    if (x.endswith('_')):
                            
                        #loop through species
                        for species in ['sapiens', 'neander']:
                            fullname = x + species
                            print("fullname [%s]"%fullname)
                            if  (fullname in namval):
                                m = re.match('(.*)(%s" *value=")(.*)(" */>)'%(fullname), line)
                                if not m is None:
                                    fout_xml.write("%s%s%s%s\n"%(m[1], m[2], namval[fullname], m[4]))
                                    bModified = True
                                    print("did match [%s] to line [%s]"%('(.*)(%s" *value=")(.*)(" */>)'%(fullname), line))
                                else:
                                    print("did NOT match [%s] to line [%s]"%('(.*)(%s" *value=")(.*)(" */>)'%(fullname), line))
                                #-- end if
                            else:
                                print("fullname [%s] not in namval %s"%(fullname,namval))
                            #-- end if
                            if bModified:
                                print("matched [%s]"%fullname)
                            else:
                                print("[%s] matched to nothing"%fullname)
                            #-- end if
                        #-- end for
                    else:
                        # species independent attributes
                        if  (x in namval):
                            m = re.match('(.*)(%s" *value=")(.*)(" */>)'%x, line)
                            if not m is None:
                                fout_xml.write("%s%s%s%s\n"%(m[1], m[2], namval[x], m[4]))
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


if __name__ == '__main__':

    out_prefix = None
    if len(sys.argv) > 4:
        xml_template = sys.argv[1]
        dat_template = sys.argv[2]
        csv_file = sys.argv[3]
        approach = sys.argv[4]
        if len(sys.argv) > 5:
            out_prefix = sys.argv[5]
        #-- end if
        print("xmlt:%s, datt:%s, csv:%s, out:%s"%(xml_template,dat_template, csv_file, out_prefix))
        fcsv = open(csv_file, "rt")
        header = fcsv.readline()
        print("header: [%s]"%header.strip())
        
        count = 0
        # go through every line of the csv file
        csv_headers = header.split(SEP)
        sim_idx = csv_headers.index('sim') if 'sim' in  csv_headers else -1
        for csv_line in fcsv:
            if sim_idx >= 0:
                count = int(csv_line.split(SEP)[sim_idx])
             #-- end if
            if out_prefix is None:
                fout_xml = sys.stdout
                fout_dat = sys.stdout
            else:
                sname_xml = "%s_%05d.xml"%(out_prefix, count)
                fout_xml  = open(sname_xml, 'w')
                sname_dat = "%s_%05d.dat"%(out_prefix, count)
                fout_dat  = open(sname_dat, 'w')
            #-- end if
            
            # collect the values for this line
            namval = collect_vals(csv_headers, csv_line)
            print(namval)
            
            # create an xml for this line (using the xml template) 
            create_xml(xml_template, namval, fout_xml)
            
            # create an dat for this line (using the dat template) 
            create_dat(dat_template, namval, approach, fout_dat)

            # close files if out put was not stdout
            if not out_prefix is None:
                fout_xml.close()
                fout_dat.close()
            #-- end if
                
            print("done with #%d"%count) 
            count = count + 1
        #-- end for

    else:
        print("%s - turn csv to xml+dat files"%sys.argv[0])
        print("usage:")
        print("  %s <xml-template> <dat-template> <csv-file> <approach> [<out-body>]"%sys.argv[0])
        print("where")
        print("  xml-template:  an XML attribute file to be used as template")
        print("  dat-template:  an dat agent data file to be used as template (all values (including header) ';'-separated)")
        print("  csv-file:      a CSV file containing attribute values")
        print("  approach;      'ali' or 'zolli'")
        print("  out-body;      name template for output files")
    #-- end if    
#-- end if (main)
