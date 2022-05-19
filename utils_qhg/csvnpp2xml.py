#!/usr/bin/python

from sys import argv
import h5py

from QHGError import QHGError

templ_header = "<class name=\"NPersOoANavSHybRelPop\" species_name=\"%s\" species_id=\"%d\">\n"

templ_hybmin = "  <module name=\"NPersHybBirthDeathRel\">\n" \
         "    <param  name=\"HybBirthDeathRel_hybminprob\" value=\"%f\" />\n" \
         "  </module>\n"

templ_hybmin = "  <module name=\"NPersHybBirthDeathRel\">\n" \
         "    <param  name=\"HybBirthDeathRel_hybminprob\" value=\"%f\" />\n" \
         "  </module>\n"


templ_fparam = "    <param  name=\"%s\" value=\"%f\" />\n"
templ_sparam = "    <param  name=\"%s\" value=\"%s\" />\n"

templ_prios  = "  <priorities>\n"\
    "    <prio  name=\"PersFertility\" value=\"2\" />\n" \
    "    <prio  name=\"GetOld\" value=\"9\" />\n" \
    "    <prio  name=\"NPersHybBirthDeathRel\" value=\"6\" />\n" \
    "    <prio  name=\"PrivParamMix\" value=\"1\" />\n" \
    "    <prio  name=\"LocEnv\" value=\"1\" />\n" \
    "    <prio  name=\"Navigate\" value=\"8\" />\n" \
    "    <prio  name=\"PersOldAgeDeath\" value=\"10\" />\n" \
    "    <prio  name=\"RandomPair\" value=\"3\" />\n" \
    "    <prio  name=\"NPersWeightedMove\" value=\"7\" />\n" \
    "  </priorities>\n"


locenv_names = [ 
    "NPPCap_K_max_sapiens",
    "NPPCap_K_min_sapiens",
    "NPPCap_NPP_max_sapiens",             
    "NPPCap_NPP_min_sapiens",             
    "NPPCap_coastal_factor_sapiens",      
    "NPPCap_coastal_max_latitude_sapiens",
    "NPPCap_coastal_min_latitude_sapiens",
    "NPPCap_water_factor_sapiens",        
    "NPPCap_K_max_neander",               
    "NPPCap_K_min_neander",               
    "NPPCap_NPP_max_neander",             
    "NPPCap_NPP_min_neander",             
    "NPPCap_coastal_factor_neander",      
    "NPPCap_coastal_max_latitude_neander",
    "NPPCap_coastal_min_latitude_neander",
    "NPPCap_water_factor_neander",
    "NPPCap_alt_pref_poly_neander",
    "NPPCap_alt_pref_poly_sapiens",
]

locenv_defaults = {
    "NPPCap_alt_pref_poly_neander":"-1.1 0 0.1 0.01 1500 1.0 2000 1 3000 -9999",
    "NPPCap_alt_pref_poly_sapiens":"-0.1 0 0.1 0.01 1501 1.0 2001 1 3001 -9999",
}

privparammix_names = [
#   "PrivParamMix_mode",          
#   "HybBirthDeathRel_hybminprob",
   
   "NPersHybBirthDeathRel_b0_sapiens",   
   "NPersHybBirthDeathRel_d0_sapiens",   
   "NPersHybBirthDeathRel_theta_sapiens",
   "NPersHybBirthDeathRel_other_sapiens",
   "NPersHybBirthDeathRel_this_sapiens", 
   "Fertility_min_age_sapiens",          
   "Fertility_max_age_sapiens",          
   "Fertility_interbirth_sapiens",       
   "NPersWeightedMove_sapiens",          
                                 
   "OAD_max_age_sapiens",                
   "OAD_uncertainty_sapiens",            

   "NPersHybBirthDeathRel_b0_neander",   
   "NPersHybBirthDeathRel_d0_neander",   
   "NPersHybBirthDeathRel_theta_neander",
   "NPersHybBirthDeathRel_other_neander",
   "NPersHybBirthDeathRel_this_neander", 
   "Fertility_min_age_neander",          
   "Fertility_max_age_neander",          
   "Fertility_interbirth_neander",       
   "NPersWeightedMove_neander",          
                                 
   "OAD_max_age_neander",                
   "OAD_uncertainty_neander",            
]

privparammix_defaults = {
    "NPersHybBirthDeathRel_other_neander":"sapiens",
    "NPersHybBirthDeathRel_this_neander":"neander", 
    "NPersHybBirthDeathRel_other_sapiens":"neander",
    "NPersHybBirthDeathRel_this_sapiens":"sapiens", 
}

navigate_names = [
    "Navigate_decay",
    "Navigate_dist0",
    "Navigate_min_dens",
    "Navigate_prob0",
]

navigate_defaults = {
    "Navigate_decay":-0.001,
    "Navigate_dist0":150,
    "Navigate_min_dens":0,
    "Navigate_prob0":0.1,
}

general_names = [
    "general_species_name",
    "general_species_id",
    "general_hybminprob",
    "general_PrivParamMix_mode",
]

agent_data_order = [
    "NPersWeightedMove",
    "OAD_max_age",                
    "OAD_uncertainty",            
    "Fertility_min_age",          
    "Fertility_max_age",          
    "Fertility_interbirth",       
    "NPersHybBirthDeathRel_b0",   
    "NPersHybBirthDeathRel_d0",   
    "NPersHybBirthDeathRel_theta",
]
agent_basic =  [
    "Longitude",
    "Latitude",
    "LifeState",
    "AgentID",
    "BirthTime",
    "Gender",
    "Age",
    "LastBirth"
    ]

NUM_BASIC_ELEMENTS = len(agent_basic)

#----------------------------------------------------------------------
#-- class NPP2XML
#--  
class NPP2XML:
    
    #----------------------------------------------------------------------
    #-- constructor
    #--  
    def __init__(self, csv_file, qdf_file):
        self.csv_file = csv_file
        self.qdf_file = qdf_file
        self.fh       = None
        self.poplevel_cols  = {}
        self.poplevel_vals  = {}
        self.poplevel_names = {"LocEnv":locenv_names,
                               "PrivParamMix":privparammix_names,
                               "Navigate":navigate_names,
                               "General":general_names}
        
        self.poplevel_defaults = {"LocEnv":locenv_defaults,
                                  "PrivParamMix":privparammix_defaults,
                                  "Navigate":navigate_defaults}
        self.all_cols_found = False
        self.read_qdf_coords()
        #print("cols")
        self.find_poplevel_cols()
        #print("poplevels\n"+str(self.poplevel_cols))
        self.set_defaults()
        #print("poplevels\n"+str(self.poplevel_cols))
        if (self.check_complete()):
            pass
            #print("poplevels\n"+str(self.poplevel_cols))
            #print("vals")
            #line =self.get_next_line(self.fh)
            #self.find_poplevel_vals(line)
        #-- end if
    #-- end def


    #----------------------------------------------------------------------
    #-- read_qdf_coords
    #--  
    def read_qdf_coords(self):
        try:
            f = h5py.File(self.qdf_file, "r")
            self.lons = list(f['Geography/Longitude'])
            self.lats = list(f['Geography/Latitude'])
            f.close()
        except IOError as e:
            raise QHGError("problem getting coords:\n%s"%str(e))
        #-- end try
    #-- end def

    
    #----------------------------------------------------------------------
    #-- get_next_line
    #--  
    def get_next_line(self,fh):
        line = fh.readline() 
        while line.startswith("#") or (('\n' in line) and (len(line.strip())==0)):
            line = fh.readline()
        #-- end while
        return line
    #- end if


   
    #----------------------------------------------------------------------
    #-- find_poplevel_cols
    #--  
    def find_poplevel_cols(self):
        try:
            self.fh = open(self.csv_file, "r")
            line = self.get_next_line(self.fh)
            if len(line) > 0:
                
                a = line.strip().split(',')
                for i in range(len(a)):
                    #print("checking %s"%(a[i]))
                    bSearching = True
                    for cn in self.poplevel_names:
                        if (bSearching):
                            #print("checking %s in cn [%s]"%(a[i], cn))
                            if a[i] in self.poplevel_names[cn]:
                                if cn in self.poplevel_cols:
                                    #print("adding %s to %s"%(a[i],cn))
                                    self.poplevel_cols[cn][a[i]] = i
                                else:
                                    #print("setting %s in %s"%(a[i],cn))
                                    self.poplevel_cols[cn] = {a[i]:i}
                                #-- end if
                                bSearching = False
                            #-- end if
                        #-- end if
                    #-- end for
                #-- end for
                
            else:
                print("no lines found in [%s]"%self.csv_file)
            #-- end if
        except IOError as e:
            raise QHGError("Error reading [%s]:\n%s"%(self.csv_file, str(e)))
        #-- end try
    #-- end def

    
    #----------------------------------------------------------------------
    #-- set_defaults
    #--  
    def set_defaults(self):
        for sec in self.poplevel_defaults:
            for df in  self.poplevel_defaults[sec]:
                if (sec in self.poplevel_vals) and (not df in  self.poplevel_vals[sec]):
                    print("value for %s not set in %s - setting default [%s]"%(df, sec,  self.poplevel_defaults[sec][df]))
                    self.poplevel_vals[sec][df] = self.poplevel_defaults[sec][df]
                #-- end if
            #-- end for
        #-- end for
    #-- end def

        

    #----------------------------------------------------------------------
    #-- check_complete
    #--  
    def check_complete(self):
        self.all_cols_found = True
        for sec in  self.poplevel_names:
            #print("checking [%s]"%sec)
            if sec in self.poplevel_cols:
                self.all_cols_found = self.all_cols_found and True
                for n in self.poplevel_names[sec]:
                    if not n in self.poplevel_cols[sec]:
                        print("[%s] is missing from cols['%s']"%(n, sec))
                        if (sec in self.poplevel_defaults):
                             self.poplevel_cols[sec][n] = None
                             print("added entry from defaults")
                        else:
                            self.all_cols_found = False
                        #-- end if
                    #-- end if
                #-- end for
            #-- end if
        #-- end if
        print("check: %s"%self.all_cols_found)
        return self.all_cols_found
    #-- end def
    
        
    #----------------------------------------------------------------------
    #-- find_poplevel_vals
    #--  
    def find_poplevel_vals(self, line):
        if len(line) > 0:
           
            a = line.strip().split(',')
            for cn in self.poplevel_names:
                for x in  self.poplevel_names[cn]:
                    #print("cn %s, x %s, col %d"%(cn,x,self.named_cols[cn][x]))
                    curcol = self.poplevel_cols[cn][x]
                    
                    if cn in self.poplevel_vals:
                        if curcol is None:
                            self.poplevel_vals[cn][x] = self.poplevel_defaults[cn][x]
                        else:
                            self.poplevel_vals[cn][x] = a[curcol]
                        #-- end if
                    else:
                        if curcol is None:
                            self.poplevel_vals[cn] = {x:self.poplevel_defaults[cn][x]}
                        else:
                            self.poplevel_vals[cn] = {x:a[curcol]}
                        #-- end if
                    #-- end if
                #-- end for
            #-- end for
        else:
            print("no line s found in [%s]"%self.csv_file)
        #-- end if


    #-- end def


    #----------------------------------------------------------------------
    #-- write_xml
    #--  
    def write_xml(self, outfile, species):
        try:
            fo = open(outfile, "w")
            spc_name = self.gen_vals['general_species_name']
            spc_id   = self.gen_vals['general_species_id']
            hybmin   = self.gen_vals['general_hybminprob']
            ppm_mode = self.gen_vals['general_PrivParamMix_mode']
            del  self.gen_vals['general_PrivParamMix_mode']
            self.poplevel_vals['PrivParamMix']['PrivParamMix_mode'] = ppm_mode
            
            fo.write(templ_header%(spc_name, int(spc_id)))
            fo.write(templ_hybmin%(float(hybmin)))
            
            for sec in self.poplevel_names:
                if (sec != 'General'):
                    fo.write("  <module name=\"%s\">\n"%sec)
                    data = self.poplevel_vals[sec]
                    for x in data:
                        s=templ_sparam%(x,data[x])
                        fo.write(s)
                    #-- end for
                    fo.write("  </module>\n")
                #-- end if
            #-- end for
            fo.write(templ_prios)
            fo.write("</class>\n")
            fo.close()
                
        except IOError as e:
            raise QHGError("Error writing [%s]:\n%s"%(outfile, str(e)))
        #-- end try
    #-- end def 


    #----------------------------------------------------------------------
    #-- create_agent_data
    #--  
    def create_agent_data(self, species):
        ad = []
        for x in agent_data_order:
            ad.append(self.poplevel_vals['PrivParamMix']["%s_%s"%(x,species)])
        #-- end for
        return ad
    #-- end def
                      

    #----------------------------------------------------------------------
    #-- create_agent_data_all
    #--  
    def create_agent_data_all(self, species, data_line):
        data_list = data_line.strip().split(',')
        if (len(data_list) >= NUM_BASIC_ELEMENTS):
            data_list = data_list[:NUM_BASIC_ELEMENTS]
            data_list = self.fix_coords(data_list);
            if species == 'sapiens':
                # pure sapiens has hybridisation 0 
                data_list.append(0)
            elif species == 'neander':
                # pure neander has hybridisation 1 
                data_list.append(1)
            else:
                raise QHGError("unknown species [%s]"%species)
            #-- end if

            data_list.extend(self.create_agent_data(species))
        else:
            raise QHGError("not enough elements in data line [%s]: have %d but should be %d"%(data_line,len(data_list),NUM_BASIC_ELEMENTS))
        #-- end if
        return data_list
    #-- end def



    #----------------------------------------------------------------------
    #-- fix_coords
    #--  
    def fix_coords(self, data_list):
        cell_id = int(data_list[1])
        del data_list[3]  # 'CellIdx'
        del data_list[1]  # 'CellID'

        lon = self.lons[cell_id]
        lat = self.lats[cell_id]
        out_list = [lon,lat]
        out_list.extend(data_list)
        return out_list
    #-- end def
    

    #----------------------------------------------------------------------
    #-- process_data
    #--  
    def process_data(self, species, data_infile, out_prefix):
        try:
            outheader = "# "+" ".join(agent_basic) +" Hybridization "+ " ".join(agent_data_order) + "\n"
            print("processing")
            fdata_in  = open(data_infile, "r")
            iC = 0
            # we expect fh hs only read the header line
            val_line  = self.get_next_line(self.fh)
            while (val_line != ''):
                #print("have val line %d:%s"%(iC, val_line))
                filebody = "%s_%s_%05d"%(out_prefix,species,iC)
                self.find_poplevel_vals(val_line)

                self.gen_names = self.poplevel_names["General"]
                self.gen_vals  = self.poplevel_vals["General"]

                self.write_xml("%s.xml"%(filebody), species);
                print("[%s] written"%("%s.xml"%(filebody)))
                
                fdata_out = open("%s.dat"%(filebody), "w")
                fdata_out.write(outheader)
                fdata_in.seek(0)
                data_line = self.get_next_line(fdata_in)
                while (data_line != ''):
                    data_list = self.create_agent_data_all(species, data_line)
                    data_str = list(map(str, data_list))
                    fdata_out.write("%s\n"%(" ".join(data_str)))
                    data_line = self.get_next_line(fdata_in)
                #-- end while
                fdata_out.close()
                print("[%s] written"%("%s.dat"%(filebody)))
                iC = iC+1
                val_line  = self.get_next_line(self.fh)
            #-- end while
        except IOError as e:
            raise QHGError("couldn't process data:\n%s"%str(e))
        finally:
            fdata_in.close()
            self.fh.close()
        #-- end try
#-- end class




if len(argv) > 5:
    infile   = argv[1]
    species  = argv[2]
    datafile = argv[3]
    icofile  = argv[4]
    outbody  = argv[5]
    try:
        n2x = NPP2XML(infile, icofile)
        if (n2x.all_cols_found):

            #print("---------------")
            #print(n2x.poplevel_names)
            #print("---------------")
            #n2x.write_xml(outfile, 'sapiens')

            n2x.process_data(species, datafile, outbody)
            
        #-- end if
    except QHGError as qhge:
        print(str(qhge))
else:
    print("usage: %s <csv-in> <species_name> <dat-in> <qdf-in> <body-out>"%argv[0])
#-- end if
  
