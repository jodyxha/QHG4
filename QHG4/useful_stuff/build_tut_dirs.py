#!/usr/bin/python

from sys import argv, exit
import os
import shutil

qhg_base = os.environ.get("QHG4_DIR")

tut_data_top = qhg_base + "/tutorial_data"
tut_data_xmldat = tut_data_top + "/xmldat"
tut_data_grids = tut_data_top + "/grids"
tut_data_config = tut_data_top + "/config"

qhg_pops = qhg_base + "/populations"

tutorial_info = [
    {
        'name': 'tutorial_01',
        'code': ['tut_StaticPop.h', 'tut_StaticPop.cpp'],
        'xmldat': [('tut_Static.xml', 'tut_Static.dat')],
        'grid': 'ico32s.qdf',
        'num_iters': ['200', '0'],
        'events': 'write|grid+geo+pop:sapiens@20000'
    },

    {
        'name': 'tutorial_02',
        'code': ['tut_OldAgeDiePop.h', 'tut_OldAgeDiePop.cpp'],
        'xmldat': [('tut_OldAgeDie.xml', 'tut_OldAgeDie.dat')],
        'grid': 'ico32s.qdf',
        'num_iters': ['200', '0'],
        'events': 'write|grid+geo+pop:sapiens@20000'
    },

    {
        'name': 'tutorial_03',
        'code': ['tut_MovePop.h', 'tut_MovePop.cpp'],
        'xmldat': [('tut_Move.xml', 'tut_Move.dat')],
        'grid': 'ico32s.qdf',
        'num_iters': ['200', '0'],
        'events': 'write|grid+geo+pop:sapiens@20000'
    },

    {
        'name': 'tutorial_04',
        'code': ['tut_ParthenoPop.h', 'tut_ParthenoPop.cpp'],
        'xmldat': [('tut_Partheno.xml', 'tut_Partheno.dat')],
        'grid': 'ico32s.qdf',
        'num_iters': ['3000', '-3000'],
        'events': 'write|grid+geo+pop:sapiens@20000'
    },

    {
        'name': 'tutorial_05',
        'code': ['tut_SexualPop.h', 'tut_SexualPop.cpp'],
        'xmldat': [('tut_Sexual.xml', 'tut_Sexual.dat')],
        'grid': 'ico32s.qdf',
        'num_iters': ['10000', '-10000'],
        'events': 'write|grid+geo+pop:sapiens@20000'
    },

    {
        'name': 'tutorial_06',
        'code': ['tut_EnvironAltPop.h', 'tut_EnvironAltPop.cpp'],
        'xmldat': [('tut_EnvironAlt.xml', 'tut_EnvironAlt.dat')],
        'grid': 'eq64Alt.qdf',
        'num_iters': ['10000', '-10000'],
        'events': 'write|grid+geo+pop:sapiens@1000'
    },

    {
        'name': 'tutorial_07',
        'code': ['tut_EnvironCapAltPop.h', 'tut_EnvironCapAltPop.cpp'],
        'xmldat': [('tut_EnvironCapAlt.xml', 'tut_EnvironCapAlt.dat')],
        'grid': 'eq64CapAlt.qdf',
        'num_iters': ['80000', '-80000'],
        'events': 'write|grid+geo+pop:sapiens@1000'
    },

    {
        'name': 'tutorial_08',
        'code': ['GrassPop.h', 'GrassPop.cpp',
                 'SheepPop.h', 'SheepPop.cpp'],
        'xmldat': [('tut_Grass.xml', 'tut_Grass.dat'),
                   ('tut_Sheep.xml', 'tut_Sheep.dat')],
        'grid': 'torus_500x500.qdf',
        'num_iters': ['5000', '0'],
        'events': 'write|grid+geo+pop:grass~+pop:sheep#@10'
    },

    {
        'name': 'tutorial_09',
        'code': ['VirusHostPop.h', 'VirusHostPop.cpp'],
        'xmldat': [('tut_VirusHost.xml', 'tut_VirusHost.dat')],
        'grid': 'torus_500x500.qdf',
        'num_iters': ['5000', '0'],
        'events': 'write|grid+geo+pop:sapiens@1000+[240]+[260],pop|sapiens:tut_VirusHost.xml:bad_guys.dat@[250]',
    }
]


# -----------------------------------------------------------------------------
# -- create_config_file
# --
# def create_config_file(full_path, tut_info, num_iters):
def create_config_file(full_path, tut_info):
    pops = []
    for x in tut_info['xmldat']:
        pops.append("%s:%s" % x)
    # -- end for
    pops_line = ",".join(pops)

    repls = {
        '+++GRID+++': tut_info['grid'],
        '+++OUTBODY+++': tut_info['name'],
        '+++NUMITERS+++': tut_info['num_iters'][0],
        '+++STARTTIME+++': tut_info['num_iters'][1],
        '+++EVENTS+++': tut_info['events'],
        '+++XMLDAT+++': pops_line
    }

    try:
        in_file = open(tut_data_config + "/template.cfg", "r")
        out_file = open(full_path + "/" + tut_info['name'] + ".cfg", "w")

        for line in in_file:
            for r in repls:
                line = line.replace(r, repls[r])
            # -- end for
            out_file.write(line)
        # -- end for
        in_file.close()
        out_file.close()
    except IOError as e:
        print("Exception in create_config file: %s" % e)
        exit()
    # -- end try


# -- end def


# -----------------------------------------------------------------------------
# -- create_and_fill_subdir
# --
# def create_and_fill_subdir(top_dir, tut_info, num_iters):
def create_and_fill_subdir(top_dir, tut_info):
    try:
        full_path = top_dir + "/" + tut_info['name']
        os.mkdir(full_path)
        os.mkdir(full_path + "/output")

        for c in tut_info['code']:
            shutil.copy(qhg_pops + "/" + c, full_path)
        # -- end for

        for x in tut_info['xmldat']:
            shutil.copy(tut_data_xmldat + "/" + x[0], full_path)
            shutil.copy(tut_data_xmldat + "/" + x[1], full_path)
        # -- end for

        # shutil.copy(tut_data_grids+"/"+tut_info['grid'], full_path)

        # create_config_file(full_path, tut_info, num_iters)
        create_config_file(full_path, tut_info)
    except IOError as e:
        print("Exception in create_and_fill_subdir: %s" % e)
        exit()
    # -- end try


# -- end def


# -----------------------------------------------------------------------------
# -- ask_delete
# --
def ask_delete(kill_dir, b_force_rmdir):
    b_ok = False
    b_remove = False
    if b_force_rmdir:
        b_remove = True
    else:
        print("directory [%s] already exists" % kill_dir)

        b_waiting = True
        while b_waiting:
            np = input("Delete [%s]? (yes/no) " % kill_dir)
            if np.lower() == "yes":
                b_remove = True
                b_waiting = False
                b_ok = True
            elif np.lower() == "no":
                b_remove = False
                b_waiting = False
            # -- end if
        # - end while
    # -- end if
    if b_remove:
        shutil.rmtree(top_tut)
    # -- end if
    return b_ok


# -- end def


# -----------------------------------------------------------------------------
# -- main
# --
if len(argv) > 1:

    force_rmdir = False

    top_tut = argv[1]
    # num_iters = 8000

    bOK = True
    if os.path.exists(top_tut):
        bOK = ask_delete(top_tut, force_rmdir)
    # -- end if
    if bOK:
        os.mkdir(top_tut)
        for t in tutorial_info:
            # print("t:%s"%t)
            print("creating and filling %s" % t['name'])
            # create_and_fill_subdir(top_tut, t, num_iters)
            create_and_fill_subdir(top_tut, t)
        # -- end for
        print("To facilitate the use of the tutorials, add the line")
        print("  TUT_TOP=%s" % top_tut)
        print(" to '~/.bashrc', and source it:")
        print(" . ~/.bashrc")
    else:
        print("Couldn't delete [%s]" % top_tut)
    # -- end if
else:
    print("%s <top_tut_dir>" % argv[0])
# -- end if
