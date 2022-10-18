#!/usr/bin/python

from sys import argv
import h5py
import numpy as np
import region_utils as ru



#----------------------------------------------------------------------------
#-- usage
#--
def usage(sApp):
    print("%s - count or delete agents in region" % sApp)
    print("usage:")
    print("  %s [\"count\"| [\"kill\"] <qdf-file>[:<species>] (<region-name> | <region_def>) " % sApp)
    print("where")
    print("  qdf-file     name of qdf file to analyse")
    print("  species      name of species to count")
    print("  region-name  name of predefined region:")
    for n in ru.predefs:
        print("                  '%s'"% n)
    print(" region-def    name of file containing vertex list")
    print("               where each line contains the coordinates of one vertex");
    print("")
#-- end if


#----------------------------------------------------------------------------
#-- main
#--

if (len(argv) > 2):
    bKill = False
    verts = []
    vertboxes = {}
    ioffs = 0
    
    if argv[1] in ['count', 'kill']:
        if argv[1] == 'kill':
            bKill = True
        #-- end if
        ioffs = 1
    #-- end if

    a = argv[1+ioffs].split(':')
    print("opening [%s]\n" %a[0])
    h = h5py.File(a[0])
    if (len(a) > 1):
        species_name = a[1]
    else:
        k = list(h["/Populations"].keys())
        species_name = k[0]
    #-- end if
    vertinfo = argv[2+ioffs]
    
    sl = vertinfo.split(':')

    # is it the name of a predefined region?
    for s in sl:
        verts = ru.get_predefined(s)
        if not verts is None:
            bbox = ru.get_bbox(verts)
            vertboxes[s] = (verts, bbox)
            print(vertboxes)
        #-- end if
    #-- end for

    # is it a file  with vertex coordinates?
    if (len(vertboxes) == 0):
        # try as file name
        verts = []
        #try:
        if True:    
            fIn = open(vertinfo)
            for line in fIn:
                if line.strip() != "":
                  verts.append([float(x) for x in line.split()])
                #-- end if
            #-- end for
            if not verts is None:
                print("got region with %d points in %s"%(len(verts),vertinfo))
                bbox = ru.get_bbox(verts)
                vertboxes[vertinfo] = (verts, bbox)
                print(vertboxes)
            #-- end if
        #except:
        else:
            print("couldn't find region or file named [%s]" % argv[2])
        #-- end try
    #-- end if

    agent_count = 0
    if (len(vertboxes) > 0):
        lons = h["/Geography/Longitude"]
        lats = h["/Geography/Latitude"]
        sPath = "/Populations/%s/AgentDataSet" % species_name
        print("looking for agents in [%s]"% sPath)
        agents = h[sPath]
        print("%d lons, %d lats, %d ags" % (len(lons),len(lats), len(agents)))
        agents2 = []
    
        for  i in range(len(agents)):
            if (i % 1000) == 0:
                print("i %d (found %d)" % (i, agent_count))
            #-- end if
            cell_id = agents[i][1]
            lon = lons[cell_id]
            lat = lats[cell_id]
            is_in_bboxes = False
            for vb in vertboxes:
                bbox = vertboxes[vb][1]
                if (lon >= bbox[0]) and \
                       (lat >= bbox[1]) and \
                       (lon <= bbox[2]) and \
                       (lat <= bbox[3]):
                    if ru.point_in_poly(vertboxes[vb][0], (lon, lat)):
                        agent_count = agent_count + 1
                        is_in_bboxes = True
                        #print(i)
                    #-- end if
                #-- end if
            #-- end for
            if not is_in_bboxes:
                agents2.append(agents[i])
            #-- end if
        #-- end for
        if bKill:
            del  h[sPath]
            h[sPath]=np.array(agents2)
            print("removed %d agents in region" % (agent_count))
            print("(you might want to run 'h5repack' on this QDF file)")
        else:
            print("found %d agents in region" % (agent_count))
        #-- end if
        h.flush()
        h.close()
    #-- end if
else:
    usage(argv[0])
#-- end main
