#!/usr/bin/python

from sys import argv, exit
import os
import shutil

print("getting qhg_base")
qhg_base = os.environ.get("QHG4_DIR")
print("got qhg_base;%s"%qhg_base)

# files required to build populations
popfiles = [
    qhg_base + '/populations/DynPopFactory.h', 
    qhg_base + '/populations/DynPopFactory.cpp',
    qhg_base + '/populations/PopulationFactory.h',
    qhg_base + '/populations/StatPopFactory.cpp_template',
    qhg_base + '/populations/StatPopFactory.h',
    qhg_base + '/populations/Makefile',
    ]

modfiles = [qhg_base + '/actions/Makefile']

force_rmdir = False

#-----------------------------------------------------------------------------
#-- basic_pop
#--  create the basic population directory named <prefix>_pop
#--  and fill it with all files required to compile populations
#--
def basic_pop(prefix):
    pop_dir = qhg_base+'/'+prefix+"_pop"

    bOK = True
    if os.path.exists(pop_dir):
        bOK = ask_delete(pop_dir, force_rmdir)
    #-- end if
    if not bOK:
        print("going")
        raise IOError("directory already exists")
    #-- end if
    try:
        os.mkdir(pop_dir)
        for f in popfiles:
            f0 = f.split('/')[-1]
            os.symlink(f, pop_dir + "/" + f0)
        #-- end for
        
    except IOError as e:
        print("Exception in basic_pop: %s"%e)
        exit
    #-- end try

    return pop_dir
#-- end def


#-----------------------------------------------------------------------------
#-- basic_mod
#--  create the basic action directory named <prefix>_mod
#--  and fill it with all files required to compile action
#--
def basic_mod(prefix, modfiles):
    mod_dir = qhg_base+'/'+prefix+"_mod"

    # delete mod directory if it alread exists
    bOK = True
    if os.path.exists(mod_dir):
        bOK = ask_delete(mod_dir, force_rmdir)
    #-- end if
    if not bOK:
        print("going")
        raise IOError("directory already exists")
    #-- end if  

    try:
        os.mkdir(mod_dir)
        for f in modfiles:
            f0 = f.split('/')[-1]
            os.symlink(f, mod_dir + "/" + f0) 
            # print("have %s -> %s"%(f,  qhg_base + "/" + mod_dir + "/" + f0))
        #-- end for
        
    except IOError as e:
        print("Exception in basic_mod: %s"%e)
        exit
    #-- end try
    return mod_dir
#-- - end def


#-----------------------------------------------------------------------------
#-- extract_includes
#--   find all include statements and extract the file names
#--
def extract_includes(pop_file):
    inc_list = []
    f = open(pop_file)
    for line in f:
        line = line.strip()
        if line.startswith("#include"):
            a = line.split('"')
            if len(a) >= 2:
                inc_list.append(a[1])
            #-- end if
        #-- end if
    #-- end for
    f.close()
    return inc_list
#-- end def


#-----------------------------------------------------------------------------
#-- select_for_dir
#--   select all filenames from the list which exist in the specified
#--   directory
#--
def select_for_dir(inclist, code_dir):
    qhg_base = os.environ.get("QHG4_DIR")

    outlist = []
    for t in inclist:
        f = "%s/%s/%s"%(qhg_base, code_dir, t)

        if os.path.exists(f):
            outlist.append(f)
        #-- end if
    #-- end for
    return outlist
#-- end def


#-----------------------------------------------------------------------------
#-- find_all_required
#--  find all required files. First extract the included files,
#--  then extract the included files from each of them and add them to the
#--  output list
#--
def find_all_required(inclist):

    mod_list = select_for_dir(inc_list, "actions")
    # print("Actions:")
    # print(mod_list)

    metalist = []
    for f in mod_list:
        m_list =  extract_includes(f)
        addl = []
        for z in m_list:
            if z.endswith('.cpp'):
                g = z.replace('.cpp','.h')
                if os.path.exists(qhg_base + "/actions/" + g):
                    addl.append(g)
                #-- end if
                # print("g:%s"%g)
            #-- end if
        #-- end for
        metalist.extend(m_list)
        metalist.extend(addl)
    #-- end for
    metaset = set(metalist)

    mod_list2 = select_for_dir(list(metaset), "actions") 

    mod_list.extend(mod_list2)
    mod_set = set(mod_list)
    # print("\nmodset:%s"%mod_set)
    return list(mod_set)
#-- end if


#-----------------------------------------------------------------------------
#-- ask_delete
#--
def ask_delete(kill_dir, force_rmdir):

    bRemove = False
    if force_rmdir:
        bRemove = True
    else:
        print("directory [%s] already exists"%kill_dir)

        bWaiting = True
        while bWaiting:
            np = input("Delete [%s]? (yes/no) "%kill_dir)
            if np.lower() == "yes":
                bRemove = True
                bWaiting = False
            elif np.lower() == "no":
                bRemove = False
                bWaiting = False
            #-- end if
        #- end while
    #-- end if
    if bRemove:
        shutil.rmtree(kill_dir)
    #-- end if
    return bRemove
#-- end def

#-----------------------------------------------------------------------------
#-- main
#--
if len(argv) > 2:
    print("start")

    qhg_base = os.environ.get("QHG4_DIR")
    print("qhg_base[%s]"%qhg_base)

    prefix  = argv[1]

    
    pops = argv[2:]
    print("prefix[%s], pops %s"%(prefix, pops))

    print("Creating alternative population and action directories: %s_pop, %s_mod"%(prefix,prefix))
    print("for compilation of QHG with the classes")
    s  = ""
    for p in pops:
        s = s+"  %s"%p
    #-- end for
    print(s)

    try:
        # create a basic population directory
        pop_dir = basic_pop(prefix)
    


        # for all provided populations, collect the required include files
        full_mod = []
        for pop_file in pops:
            inc_list = extract_includes(qhg_base+"/populations/"+pop_file+".cpp")
            mod_list = find_all_required(inc_list)
            full_mod.extend(mod_list)
            # add the files for provided populations
            os.symlink(qhg_base +"/populations/"+pop_file+".h", pop_dir + "/" + pop_file+".h")
            os.symlink(qhg_base +"/populations/"+pop_file+".cpp",  pop_dir + "/" + pop_file+".cpp")
        #-- end for
        full_mod.extend(modfiles)
        
        # print("for action:")
        # for f in sorted(full_mod):
        #     print("  %s"%f)
        #- end for
    
        # remove duplicates
        full_mod_final = list(set(full_mod))
        # create action directory
        basic_mod(prefix, full_mod_final)

        print("success")
        print("You can now cd to [%s] and compile QHG like this:"%qhg_base)
        print("  SHORT=%s make clean QHGMain"%prefix)
    except IOError as e:
        print("Exception in create_config file: %s"%e)
        exit
    #-- end try
else:
    print("%s - create and appropriately fill alternativ population and action subdirectories"%argv[0])
    print("Usage:")
    print("  %s <prefix> <pop_name>+"%argv[0])
    print("where")
    print("  prefix     prefix to be used for the naming of the alternative popuation and action subdirectories")
    print("  pop_name   name of a class whose c++ and h files are in the population subdirectory")
    
#-- end if
