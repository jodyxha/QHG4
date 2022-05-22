#!/usr/bin/python

from sys import argv,stdout
import re

double_items= {
    "LocEnv" : {
        "NPPCap_K_max_sapiens":"32.802734375",
        "NPPCap_K_min_sapiens":"0.0",
        "NPPCap_NPP_max_sapiens":"0.9720703125",
        "NPPCap_NPP_min_sapiens":"0.0",
        "NPPCap_coastal_factor_sapiens":"0.3",
        "NPPCap_coastal_max_latitude_sapiens":"66.0",
        "NPPCap_coastal_min_latitude_sapiens":"50.0",
        "NPPCap_water_factor_sapiens":"0.5568359375",
        "NPPCap_K_max_neander":"32.802734375",
        "NPPCap_K_min_neander":"0.0",
        "NPPCap_NPP_max_neander":"0.9720703125",
        "NPPCap_NPP_min_neander":"0.0",
        "NPPCap_coastal_factor_neander":"0.3",
        "NPPCap_coastal_max_latitude_neander":"66.0",
        "NPPCap_coastal_min_latitude_neander":"50.0",
        "NPPCap_water_factor_neander":"0.5568359375",
        "NPPCap_alt_pref_poly_neander":"-1.1 0 0.1 0.01 1500 1.0 2000 1 3000 -9999",
        "NPPCap_alt_pref_poly_sapiens":"-0.1 0 0.1 0.01 1501 1.0 2001 1 3001 -9999",
    },

    "PrivParamMix" : {
        "NPersHybBirthDeathRel_b0_sapiens":"0.2585693359375",
        "NPersHybBirthDeathRel_d0_sapiens":"0.020107421875",
        "NPersHybBirthDeathRel_theta_sapiens":"0.2585693359375",
        "NPersHybBirthDeathRel_other_sapiens":"neander",
        "NPersHybBirthDeathRel_this_sapiens":"sapiens",
        "Fertility_min_age_sapiens":"13.983398",
        "Fertility_max_age_sapiens":"44.94629",
        "Fertility_interbirth_sapiens":"2.2236328",
        "NPersWeightedMove_sapiens":"0.0885986328125",
        "OAD_max_age_sapiens":"69.404296875",
        "OAD_uncertainty_sapiens":"0.1",
        "NPersHybBirthDeathRel_b0_neander":"0.2585693359375",
        "NPersHybBirthDeathRel_d0_neander":"0.020107421875",
        "NPersHybBirthDeathRel_theta_neander":"0.2585693359375",
        "NPersHybBirthDeathRel_other_neander":"sapiens",
        "NPersHybBirthDeathRel_this_neander":"neander",
        "Fertility_min_age_neander":"13.983398",
        "Fertility_max_age_neander":"44.94629",
        "Fertility_interbirth_neander":"2.2236328",
        "NPersWeightedMove_neander":"0.0885986328125",
        "OAD_max_age_neander":"69.404296875",
        "OAD_uncertainty_neander":"0.1",
        "PrivParamMix_mode":"5",
        },
    }

single_items= {
    "NPersZHybBirthDeathRel": {
        "HybBirthDeathRel_hybminprob":"0.0100" 
    },

    "Navigate":{
        "Navigate_decay":"-0.001",
        "Navigate_dist0":"150.0",
        "Navigate_min_dens":"0.0",
        "Navigate_prob0":"0.1",
    }
}



prios='  <priorities>\n' \
'    <prio  name="PersFertility" value="2" />\n' \
'    <prio  name="GetOld" value="9" />\n' \
'    <prio  name="NPersZHybBirthDeathRel" value="6" />\n' \
'    <prio  name="PrivParamMix" value="1" />\n' \
'    <prio  name="LocEnv" value="1" />\n' \
'    <prio  name="Navigate" value="8" />\n' \
'    <prio  name="PersOldAgeDeath" value="10" />\n' \
'    <prio  name="RandomPair" value="3" />\n' \
'    <prio  name="NPersWeightedMove" value="7" />\n' \
'  </priorities>\n'


translatables = {
    "NPPCap_alt_pref_poly_neander":"AltCapPref",
    "NPPCap_alt_pref_poly_sapiens":"AltCapPref",
    "NPersHybBirthDeathRel_b0_sapiens":"BirthDeathRelComp_b0",
    "NPersHybBirthDeathRel_d0_sapiens":"BirthDeathRelComp_d0",
    "NPersHybBirthDeathRel_theta_sapiens":"BirthDeathRelComp_theta",
    "NPersHybBirthDeathRel_other_sapiens":"BirthDeathRelComp_other",
    "NPersHybBirthDeathRel_this_sapiens":"BirthDeathRelComp_this",
    "NPersHybBirthDeathRel_b0_neander":"BirthDeathRelComp_b0",
    "NPersHybBirthDeathRel_d0_neander":"BirthDeathRelComp_d0",
    "NPersHybBirthDeathRel_theta_neander":"BirthDeathRelComp_theta",
    "NPersHybBirthDeathRel_other_neander":"BirthDeathRelComp_other",
    "NPersHybBirthDeathRel_this_neander":"BirthDeathRelComp_this",
    "NPersWeightedMove_sapiens":"WeightedMove_prob",
    "NPersWeightedMove_neander":"WeightedMove_prob"

}

polypat='[\[]*(.*)\] *[\[]*(.*)\] *[\[]*.*\]'

def translate(name):
    out = name
    if name in translatables:
        out = translatables[name]
    #-- end if
    return out
#-- end def

def write_module_open(fOut, module_name):
    fOut.write('  <module name="%s">\n'%module_name)
#-- end def


def write_module_close(fOut):
    fOut.write('  </module>\n')
#-- end def


def write_param(fOut, param_name, param_value):
    fOut.write('    <param  name="%s" value="%s" />\n'%(param_name, param_value))
#-- end def


def write_intro(fOut):
    fOut.write('<class name="NPersZOoANavSHybRelPop" species_name="sapiens" species_id="117">\n')
#-- end def


def write_extro(fOut):
    fOut.write('</class>\n')
#-- end def


def write_prios(fOut):
    fOut.write(prios)
#-- end def


def dissect(xmlfile):
    pat='<attr name="(.*)" value="(.*)" />'

    f = open(xmlfile, "r")
    h={}
    for line in f:
        line = line.strip()
        m = re.match(pat, line)
        if not m is None:
            name  = m[1]
            value = m[2]
            print("::: %s => %s"%(name, value))
            h[name] = value
        #-- end if
    #-- end for
    h['PrivParamMix_mode']='5'
    return h
#-- end def


if len(argv) > 2:
    neander_in = argv[1]
    sapiens_in = argv[2]
    fOut = stdout
    if len(argv) > 3:
        fOut = open(argv[3], 'w')
    #-- end if    

    hneander=dissect(neander_in)
    hsapiens=dissect(sapiens_in)

    write_intro(fOut)

    for it in single_items:
        write_module_open(fOut, it)
        for par in single_items[it]:
            write_param(fOut, par, single_items[it][par])
        #-- end for
        write_module_close(fOut)
    #-- end for

    for it in double_items:
        write_module_open(fOut, it)
        for par in double_items[it]:
            if par.endswith('_neander'):
                print("par: [%s]; t(par): [%s], t(par).rep: [%s]"%(par, translate(par),translate(par).replace('_neander', '')))
                val=hneander[translate(par).replace('_neander', '')]
            elif par.endswith('_sapiens'):
                val=hsapiens[translate(par).replace('_sapiens', '')]
            else:
                val=hsapiens[translate(par).replace('_sapiens', '')]
                #raise Exception("error for [%s]:[%s]"%(it,par))
            #-- end if
            
            m=re.match(polypat, val)
            if not m is None:
                val = "%s %s"%(m[1],m[2])
            #-- end if

            write_param(fOut, par, val)
        #-- end for
        write_module_close(fOut)
    #-- end for

    
    write_prios(fOut)
    write_extro(fOut)

    if len(argv) > 3:
        fOut.close()
    #-- end if
else:
    print("%s <neander_xml> <sapiens_xml> [<output_xml>]"%(argv[0]))
#-- end if
   
