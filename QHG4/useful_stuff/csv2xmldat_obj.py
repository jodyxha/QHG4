#!/usr/bin/python


import sys
import re
from xml.dom import minidom
from csv2xmldatbuilder import csv2xmldatbuilder, QHGError

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

additional_entries = {
    "sapiens": {"GenHybM":                     '0',
                "GenHybF":                     '0',
                "NPersWeightedMove":           '0.0885986328125',
                "OAD_max_age":                 '69.404296875',     
                "OAD_uncertainty":             '0.1',                  
                "Fertility_min_age":           '13.983398',            
                "Fertility_max_age":           '44.94629',           
                "Fertility_interbirth":        '2.2236328',      
                "NPersHybBirthDeathRel_b0":    '0.2585693359375',
                "NPersHybBirthDeathRel_d0":    '0.020107421875',
                "NPersHybBirthDeathRel_theta": '0.2585693359375'
    },                
    "neander": {"GenHybM":                     '1',
                "GenHybF":                     '1',
                "NPersWeightedMove":           '0.0885986328125',
                "OAD_max_age":                 '69.404296875',     
                "OAD_uncertainty":             '0.1',                  
                "Fertility_min_age":           '13.983398',            
                "Fertility_max_age":           '44.94629',           
                "Fertility_interbirth":        '2.2236328',      
                "NPersHybBirthDeathRel_b0":    '0.2585693359375',
                "NPersHybBirthDeathRel_d0":    '0.020107421875',
                "NPersHybBirthDeathRel_theta": '0.2585693359375'
    },
}



# the entry 'species' (0:sapiens, 1:neander) is needed to choose the values to be used for HybVal1 and HybVal2
SPECIES_INDEX=dat_min_entries.index("Species")

DEF_SEP   = ';'
DEF_WIDTH = 5
DEF_WHAT  = "xml+dat"


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
    species    = "sapiens"
    
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
            csv_ignore_params = sys.argv[cur+1].split(':')
        elif (sys.argv[cur] == "-s"):
            # speies selector
            species = sys.argv[cur+1]
        #-- end if
        cur = cur + 2
    #-- end while
    actions = what.split('+')
    bDoXML = ("xml" in actions)
    bDoDAT = ("dat" in actions)

        
    if (req == 0):
        print("xml_block:[%s]"%xml_block_params)
        
        print("xmlt:%s, datt:%s, csv:%s, out:%s"%(xml_template,dat_template, csv_file, out_prefix))

        try: 
            c2xd =csv2xmldatbuilder(csv_file, xml_template, dat_template, xml_block_params, species)

            iResultCode = c2xd.creation_loop(csv_ignore_params, additional_entries[species], out_prefix, bDoXML, bDoDAT, width)
        except QHGError as e:
            print("ERROR: %s"%e)
        #-- end try

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
   
    #print("@@TODO Need to think about a generalisation of dat creation!!!!!")
    #print("@@TODO Less hardwired code!!!!!")
    #print("@@TODO Make csv2xmldat_new.py work for all files (hyb or not)!!!!!")
    sys.exit(iResultCode)
#-- end if (main)
