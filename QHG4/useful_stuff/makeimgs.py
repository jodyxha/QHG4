#!/usr/bin/python
from sys import argv
import shutil
import gzip
import os
import glob
import subprocess as sp
from QHGError import QHGError

def_roll = 335
#def_img_fmt = "1600x800"
def_img_fmt = "800x400"
pre_def = {
    'alt_pop_mag_ice': 'alt|geo:-6000:0:6000,ice|twotone:0.5:#00000000:#FFFFFFFF,pop_Sapiens_ooa|fadeout:0:10:#FF00FFFF',
#    'alt_pop_yel_ice': 'alt|geo2:-3000:0:3000,ice|twotone:0.5:#00000000:#BDEAFFFF,pop_Sapiens_ooa|fadeout:0:10:#FFCF00FF',
    'alt_pop_yel_ice': 'alt|geo:-6000:0:6000,ice|twotone:0.5:#00000000:#BDEAFFFF,pop_Sapiens_ooa|fadeout:0:10:#FFCF00FF',
    'npp_alt_ice': 'npp|rainbow:0.01:1.2,alt|twotone:0.5:#00000000:#FFFF0040,ice|twotone:0.5:#00000000:#FFFFFF40',
    'npp_b_alt_ice': 'npp_b|rainbow:0.05:1.2,alt|twotone:0.5:#00000000:#FFFF0040,ice|twotone:0.5:#00000000:#FFFFFF40',
    'alt_pop_ice_tt': 'alt|twotone:0:#FFFFFFFF:#000000FF,ice|twotone:0.5:#000000FF:#FFFFFFFF,pop_Sapiens_ooa|twotone:1:#FFFFFFFF:#000000FF'
}

qhg_dir = '/home/jody/progs/multi_spc_QHG3'
array_specs = 'alt_pop_ice'
def_geo_grid = qhg_dir + '/resources/grids/GridSG_ieq_256.qdf'
qdf2png      = qhg_dir + '/tools_io/QDF2PNGNew'

def_pop_pat = 'ooa_pop-Sapiens_ooa_'
def_env_pat = 'ooa_SGCVNM'
def_out_pat = 'image_###_@@@.png'
def_comp    = 'over'
def_file    = 'temp.desc'

overlay=True

#-----------------------------------------------------------------------------
#-- ImgMaker
#--
class ImgMaker:
    

    #-----------------------------------------------------------------------------
    #-- constructor
    #--  
    def __init__(self, args):
        self.arglist = {}
        self.arglist['-d'] = './'
        self.arglist['-w'] = ''
        self.arglist['-g'] = def_geo_grid
        self.arglist['-s'] = def_img_fmt
        self.arglist['-r'] = ''
        self.arglist['-b'] = ''
        self.arglist['-o'] = def_out_pat
        self.arglist['-c'] = def_comp
        self.arglist['-a'] = array_specs
        self.arglist['-f'] = ''
        self.arglist['-e'] = def_env_pat
        self.arglist['-p'] = def_pop_pat
        self.arglist['-R'] = def_roll
        self.arglist['-P'] = 1
        self.arglist['-T'] = 'no'

        self.desc_file = ''
        self.base = -1
        self.env_pat = ''
        self.pop_pat = ''
        self.out_pat = ''
        self.arr_spec = ''
        self.is_time  = False
        
        self.parse_args(args)
        self.check_args()
        self.create_desc()
    #-- end def
 

    #-----------------------------------------------------------------------------
    #-- parse_args
    #--  
    def parse_args(self, args):
        print("num args: %d" % len(argv))
        print("arg[0]: %s" % argv[0])
        print("arg[1]: %s" % argv[1])
        if (len(argv)%2)== 1:
            for i in range(1, len(argv), 2):
            
                if (argv[i][0] == '-') and (argv[i][1] in 'dwgsrboRcafepPT') :
                    self.arglist[argv[i]] = argv[i+1]
                    print("added %d: %s" % (i+1, argv[i+1]))
                else:
                    raise QHGError("Argument %d is notvalid: [%s]" % (i, argv[i]))
                #-- end if
            #-- end for
        else:
            raise QHGError("bad number of arguments")
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- check_args
    #--  
    def check_args(self):
        sErr = ''
        # -d: must exist
        if os.path.exists(self.arglist['-d']):
            self.data_dir = self.arglist['-d']
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') +  "The directory [%s] does not exist" % (self.arglist['-d'])
        #-- end if
        
        # -w: must exist or be empty
        if (len(self.arglist['-w']) > 0) and os.path.exists(self.arglist['-w']):
            self.work_dir = self.arglist['-w']
        else:
            if len(self.arglist['-w']) == 0:
                self.work_dir = self.data_dir
            else:
                sErr = sErr + ('\n' if len(sErr) > 0 else '') +  "The directory [%s] does not exist" % (self.arglist['-w'])
            #-- end if
        #-- end if
        
        # -g: must exist
        print("[%s] exists: %s"%(self.arglist['-g'], "yes" if   os.path.isfile(self.arglist['-g']) else "no"))
        if os.path.isfile(self.arglist['-g']):
            self.geo_file = self.arglist['-g']
        else:
            sErr =  sErr + ('\n' if len(sErr) > 0 else '') +"The grid file [%s] does not exist" % (self.arglist['-g'])
        #-- end if
        
        # -s: must be<W>x<H>
        a = self.arglist['-s'].split('x')
        if (len(a) == 2):
            try:
                w = int(a[0])
                h = int(a[1])
                self.img_format = self.arglist['-s']
            except:
                sErr = sErr + ('\n' if len(sErr) > 0 else '') + "The format string must have form <integer>x<integer>: [%s]" % (self.arglist['-s'])
            #-- end try
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Not a valid format string (<W>x<H>): [%s]" % (self.arglist['-s'])
        #-- end if
        
        # -r: must be <start>[:<stop>[:<step>]][R]
        a = self.arglist['-r'].split(':')
        for i in range(len(a)):
            if a[i][-1] == 'S':
                self.is_time = False
            elif  a[i][-1] == 'T':
                self.is_time = True
            #-- end if
        #-- end for
        if (len(a) <= 3) and (len(a) > 0):
            try:
                self.start = int(a[0].replace('S',' ').replace('T',' '))
                self.stop  = int(a[1].replace('S',' ').replace('T',' ')) if (len(a) > 1) else 0
                self.step  = int(a[2].replace('S',' ').replace('T',' ')) if (len(a) > 2) else 1000
                if (self.start < self.stop) or  (self.step > (self.start - self.stop)):
                    sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Empty range (start must be greater than stop): [%s]" % (self.arglist['-r'])
                    min < max 
            except:
                sErr = sErr + ('\n' if len(sErr) > 0 else '') + "The format string must have form <integer>x<integer>: [%s]" % (self.arglist['-r'])
            #-- end try
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Not a valid range format (<min>[:<max>[:<step>]]): [%s]" % (self.arglist['-r'])
        #-- end if
        
        # -b: must be number
        try:
            self.base = int(self.arglist['-b'])
        except:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "The numbering base must be a number [%s]" % (self.arglist['-b'])
        #-- end try
        
        # -o: must contain '_###' and '_@@@'
        if (('_@@@' in self.arglist['-o']) and ('_###' in self.arglist['-o'])):
            self.out_pat =  self.arglist['-o']
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "The ouput pattern must contain '_###' and '_@@@': [%s]" % (self.arglist['-o'])
        #-- end if
        
        # -c: 'over' or nothing
        if (self.arglist['-c'] == 'over') or (self.arglist['-c'] == ''):
            self.comp = self.arglist['-c']
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Composition mode must be 'over' or '': [%s]" % (self.arglist['-c'])
        #-- end if
        
        # -f: '' or must exist
        if os.path.exists(self.arglist['-f']):
            self.desc_file = self.arglist['-f']
        else:
            if not (self.arglist['-f'] == ''):
                sErr = sErr + ('\n' if len(sErr) > 0 else '') + "The Description file does not exist: [%s]" % (self.arglist['-f'])
            #-- end if
        #-- end if
        
        # -a: user's responsability
        if (self.arglist['-a'] in pre_def):
            self.arr_spec = pre_def[self.arglist['-a']]
        else:
            self.arr_spec = self.arglist['-a']
        #-- end if
        
        # -e: user's responsability
        if (self.arglist['-e'] != ''):
            self.env_pat = self.arglist['-e']
            print("env:pat is [%s]"%self.env_pat)
        else:
            sErr =  sErr + ('\n' if len(sErr) > 0 else '') + "The env_pat must not be empty"
        #-- end if
        
        # -p: user's responsability
        if (self.arglist['-p'] != ''):
            self.pop_pat = self.arglist['-p']
            print("pop_pat is [%s]"%self.pop_pat)
        else:
            sErr =  sErr + ('\n' if len(sErr) > 0 else '') + "The pop_pat must not be empty"
        #-- end if


        # -R: number
        try:
            self.roll = int(self.arglist['-R'])
        except:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Roll must be a number: [%s]" % (self.arglist['-R'])
        #-- end if


        # -P: number
        try:
            prec = int(self.arglist['-P'])
            if (prec > 0) and (prec <= 6):
                
                self.format_string = "%%0%dd" % prec
            else:
                sErr = sErr + ('\n' if len(sErr) > 0 else '') + "xPrecision must be a integer between 1 and 6: [%s]" % (self.arglist['-P'])
            #-- end if
            
        except:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "ePrecision must be a integer: [%s]" % (self.arglist['-p'])
        #-- end if

        # -T: 'yes' or 'no'
        if (self.arglist['-T'] == 'yes'):
            self.timestamp = True
        elif (self.arglist['-T'] == 'no'):
            self.timestamp = False
        else:
            sErr = sErr + ('\n' if len(sErr) > 0 else '') + "Timestamp state must be 'yes' or 'no': [%s]" % (self.arglist['-T']) 
        
                        
        if (sErr != ''):
            raise QHGError(sErr)
        #-- end if
    #-- end def

    

    #-----------------------------------------------------------------------------
    #-- create_desc
    #--  loop through range and create descriptor file
    #--
    def create_desc(self):
        self.file_list = {}
        self.out_list  = []
        if (self.desc_file == ''):
            self.desc_file = def_file
            fOut = open(self.desc_file, "wt")
            print("stop: %d, start: %d, step: %d" % (self.start, self.stop, self.step))
            
            steps = list(range(self.stop, self.start+1, self.step))
            if  self.is_time:
                steps.reverse()
            #-- end if
            print("Nummm: "+str(len(steps))) 
            print("first: %d, last: %d, step: %d" % (steps[0], steps[-1], self.step))
                        
            sLine_pop=""
            sLine_env=""
            for x in steps:
                if self.is_time:
                    sIn  = self.format_string % (x)
                    sOut = self.format_string % (self.base - x)
                else:
                    sIn  = self.format_string % (self.base - x)
                    sOut = sIn
                #-- end if
                print("x: %d, base-x: %d,  fIn: %s, fOut: %s" % (x, self.base-x,  sIn, sOut))
                if self.env_pat != '<none>':
                    sLine_env   =  "%s/%s" % (self.data_dir, self.env_pat.replace("_@@@", "_"+sIn))
                    self.file_list[sLine_env] = False
                #-- end if
                if self.pop_pat != '<none>': 
                    sLine_pop   =  "%s/%s" % (self.data_dir, self.pop_pat.replace("_@@@", "_"+sIn))
                    self.file_list[sLine_pop] = False
                #-- end if
                sLine_out   =  "%s/%s" % (self.work_dir, self.out_pat.replace("_@@@", "_"+sOut))
                
                fOut.write("%d:%s,%s:%s\n" % (x, sLine_env, sLine_pop, sLine_out))
            #-- end for 
            fOut.close()
        #-- end if
    #-- end def
    
 
    #-----------------------------------------------------------------------------
    #-- cond_unzip
    #--   unzip if necessary
    #--
    def cond_unzip(self):
        for f in self.file_list:
            if os.file.exists(f):
                self.file_list[f] = False
            elif  os.file.exists(f+'.gz'):
                self.file_list[f] = False
                gunzip_file(f+'.gz', f)
            else:
                raise QHGError("Neither [%s] nor [%s] exist" % (f,f +'.gz'))
            #-- end if
        #-- end for
    #-- end def
                           
    
 
    #-----------------------------------------------------------------------------
    #-- cond_zip
    #--   unzip if necessary
    #--
    def cond_zip(self):
        for f in self.file_list:
            if (self.file_list[f]):
                gzip_file(f, f+'.gz')
            #-- end if
        #-- end for
    #-- end def

 
    #-----------------------------------------------------------------------------
    #-- create_images
    #--
    def create_images(self):
        arguments = [qdf2png, 
                     "-g", self.geo_file,
                     "-f", self.desc_file,
                     "-s", self.img_format,
                     "-a", self.arr_spec,
                     "-r", str(self.roll)]
        if (self.comp == 'over') :
            arguments.extend(["-c", "over"])
        #-- end if
        
            
        print("argumens [%s]\n"%arguments)
        proc = sp.Popen(arguments,
                        stdout=sp.PIPE,
                        stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        print(sOut.decode())
        if (iResult == 0):
            print("images ok")
        else:
            raise QHGError("%s failed with %d; error: [%s]\n" % (qdf2png, iResult, sError.decode()))
        #-- end if
    #-- end def
 
 
    #-----------------------------------------------------------------------------
    #-- make_movie
    #--
    def make_movie(self):
        out_core = self.out_pat.replace('_###', '').replace('_@@@', '')
        movie_name = self.work_dir+"/"+out_core.replace('.png', '.mpeg')

        img_list = glob.glob(self.work_dir+"/"+self.out_pat.replace('_###', '_*').replace('_@@@', '_[0-9]*').replace('.png','2.png'))
        print(img_list)
        arguments = ["convert", 
                     "-quality", "100",
                     "-delay", "1x4"]
        arguments.extend(sorted(img_list))
        arguments.append(movie_name)

        print("arguments to 'convert':\n"+str(arguments))
        
        proc = sp.Popen(arguments,
                        stdout=sp.PIPE,
                        stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        print(sOut.decode())
        if (iResult == 0):
            print("cropped ok")
        else:
            raise QHGError("convert failed with %d; error: [%s]" % (iResult, sError.decode()))
        #-- end if
    #-- end def


  
 
    #-----------------------------------------------------------------------------
    #-- crop_images
    #--
    def crop_images(self):
        out_core = self.out_pat.replace('_###', '').replace('_@@@', '')
        cropped_name = self.work_dir+"/"+out_core.replace('.png', '2.png')

        a = self.arglist['-s'].split('x')
        print("a is [%s]: "%a)
        
        cx =  5.0 * int(a[0]) / 720.0
        cy = 13.0 * int(a[1]) / 360.0
        cw = 407  * int(a[0]) / 720.0
        ch = 245  * int(a[1]) / 360.0
        
        img_list = glob.glob(self.work_dir+"/"+self.out_pat.replace('_###', '_*').replace('_@@@', '_[0-9]*'))

        for f in img_list:
            arguments = ["convert", f,
                         "-crop", "%dx%d+%d+%d"%(round(cw),round(ch),round(cx),round(cy)),
            f.replace(".png","2.png")]

            print("arguments to 'convert':\n"+str(arguments))
        
            proc = sp.Popen(arguments,
                        stdout=sp.PIPE,
                        stderr=sp.PIPE)
            sOut, sError = proc.communicate()
            iResult = proc.returncode
            print(sOut.decode())
        ##-- end if
        if (iResult == 0):
            print("cropping ok (%s)")
        else:
            raise QHGError("convert failed with %d; error: [%s]" % (iResult, sError.decode()))
        #-- end if
    #-- end def

    
    #-----------------------------------------------------------------------------
    # gunzip_file
    #  gunzip gz_file to out_file
    #
    def gunzip_file(gz_file, out_file):
    
        with gzip.open(gz_file, 'rb') as f_in, open(out_file, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
        #-- end with
    #-- end def


    #-----------------------------------------------------------------------------
    # gzip_file
    #  gzip gz_file to out_file
    #
    def gzip_file(out_file, gz_file):
    
        with open(out_file, 'rb') as f_in, gzip.open(gz_file, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
        #--- end with
    #-- end def


#-- end class


#-----------------------------------------------------------------------------
#-- usage
#--
def usage(app):
    print("%s - extract images fromqf files and make a movie" % (app))
    print("usage:")
    print("  %s -d <data_dir> -r <start[:<stop>[:<step>]][\"S\"|\"T\"] -b <base_time>" % (app))
    print("     -a <array_specs> [-f <desc_file>] )")
    print("    [-R <roll>] [-c <comp_mode>] [-w <work_dir>]")
    print("    [-g <geo_qdf>]")
    print("    [-o <out_pat>] [-e <env_pat>] [-p <pop_pat>]")
    print("    [-P <precision>] -T <timestamp_state>")
    print("where")
    print("  data_dir     directory containing qdf files")
    print("  work_dir     work directory (images, movie, intermediate files")
    print("  start        start time in kyb.p.")
    print("  stop         stop time in kyb.p. Default: 0")
    print("  step         step in ky. Default: 1")
    print("  base_time    time stamp of first qdf file")
    print("  array_specs  specifications for arrays to use. Format:")
    print("                 array_spec ::= <pre_def> or <explicit>")
    print("                 pre_def     : pre-defnined array spec (s. below)")
    print("                 explicit   ::= <array_name> [\"@\"<index>][\"|\"<lookup>]")
    print("                 array_name  :   name of array (s. below)");
    print("                 index       :   index of qdf in which to look (0: qdf_geogrid, 1-N: qdf-data in given order)");
    print("                 lookup      :   info for lookup, with format <lookup_name>[:<data>]* (s. below)");
    print("  roll         longitude shift. Default:%s" % (def_roll))
    print("  comp_mode    composition mode. Currently only 'over' or 'sep'")
    print("  geo_qdf      QDF foile containing 'grid' and 'geography' groups")
    print("  desc_file    description file for QDF2PNGNew")
    print("  out_pat      pattern for output image files. Must contain '_@@@' for step number and '_###' for array name")
    print("               Default: %s" % (def_out_pat))
    print("  env_pat      pattern for environement QFD files. Must contain '_@@@' for step number")
    print("               Default: %s" % (def_env_pat))
    print("  pop_pat      pattern for population QDF files. Must contain '_@@@' for stepnumber")
    print("               Default: %s" % (def_pop_pat))
    print("  precision    number of digits used for representation of steps")
    print("               Default: 1")
    print("  timestamp    print timestamp on image: 'yes' or 'no'")
    print("               Default: 'no'")
    print("")
    print("Arraynames:\n");
    print("  lon       longitude   (Geography::m_adLongitude)");
    print("  lat       latitude    (Geography::m_adLatitude)");
    print("  alt       altitudes   (Geography::m_adAltitude)");
    print("  ice       ice cover   (Geography::m_abIce)");
    print("  water     water       (Geography::m_adWater)");
    print("  coastal   coastal     (Geography::m_abCoastal)");
    print("  temp      temperature (Climate::m_adAnnualMeanTemp)");
    print("  rain      rainfall    (Climate::m_adAnnualRainfall)");
    print("  npp       total npp   (NPPVegetation::m_adTotalANPP)");
    print("  npp_b     base npp    (NPPVegetation::m_adBaseANPP)");
    print("  dist      travel distance (MoveStats::m_adDist)");
    print("  time      travel time     (MoveStats::m_adTime)");
    print("  pop       population number");
    print("Lookups:");
    print("  rainbow   data: min, max");
    print("  rainbow2  data: min, max");
    print("  geo       data: min, sealevel, max");
    print("  twotone   data: Sepvalue, RGBA1, RGBA2");
    print("  fadeout   data: min,max, RGBAmax");
    print("Predefined array specs:");
    for x in pre_def:
        print("  %s" % (x))
    print("")
    print("Example:")
    print("%s -d  /home/jody/Simulations/multi_comp85B")
    print("      -r 85000:0:500 ")
    print("      -b 85000 ")
    print("      -a 'alt\|twotone:0:\#000080FF:\#FFFFFFFF,pop_neander@1\|fadeout:0:6:\#00FF00FF,pop_sapiens@1\|fadeout:0:6:\#FF0000FF'")
    print("      -c over")
    print("      -g /home/jody/Simulations/multi_comp85D/ooa_pop-neander_pop-sapiens_SG_085000.qdf")
    print("      -o comp85B2_###_@@@.png")
    print("      -e ooa_pop-neander_pop-sapiens_SG_@@@.qdf")
    print("      -p  ooa_pop-neander_pop-sapiens_SG_@@@.qdf") 

    
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 1):
        try:
            im = ImgMaker(argv)
            im.cond_unzip
            im.create_images()
            im.cond_zip
            im.crop_images()
            im.make_movie() 
        except QHGError as qhge:
            print("QHGError: [%s]" % qhge)
        #except Exception, e:
        #    print("Exception: [%s]" % e.message)
        #-- end try
    else:
        usage(argv[0])
    #-- end if
#-- end main


