#!/usr/bin/python

from sys import argv
import h5py
import region_utils as ru



#----------------------------------------------------------------------------
#-- usage
#--
def usage(sApp):
    print("%s - count number of agent in region" % sApp)
    print("usage:")
    print("  %s <qdf-file>[:<species>] (<region-name> | <region_def>) " % sApp)
    print("where")
    print("  qdf-file     name of qdf file to analyse")
    print("  species      name of species to count")
    print("  region-name  name of predefined region:")
    print("                  'africa'")
    print("                  'americas'")
    print("                  'australia'")
    print("                  'australia2'")
    print("                  'eurasia'")
    print(" region-def    name of file containing vertex list")
    print("               where each line contains the coordinates of one vertex");
    print("")
#-- end if


#----------------------------------------------------------------------------
#-- main
#--

if (len(argv) > 2):

    verts = []
    vertboxes = {}

    a = argv[1].split(':')
    print("opening [%s]\n" %a[0])
    h = h5py.File(a[0])
    if (len(a) > 1):
        species_name = a[1]
    else:
        k = list(h["/Populations"].keys())
        species_name = k[0]
    #-- end if
    sl = argv[2].split(':')

    for s in sl:
        verts = ru.get_predefined(s)
        if not verts is None:
            bbox = ru.get_bbox(verts)
            vertboxes[s] = (verts,bbox)
            print(vertboxes)
        #-- end if
    #-- end for

    if (len(vertboxes) == 0):
        # try as file name
        verts = []
        try:
            fIn = open(argv[2])
            for line in fIn:
                verts.append(line.split())
            #-- end for
            if not verts is none:
                bbox = ru.get_bbox(verts)
                vertboxes[argv[2]] = (verts,bbox)
                print(vertboxes)
            #--
        except:
            print("couldn't find region or file named [%s]" % argv[2])
        #-- end try
    #-- end if

    agent_count = 0
    if (len(vertboxes) > 0):
        lons = h["/Geography/Longitude"]
        lats = h["/Geography/Latitude"]
        print("looking fo agents in [/Populations/%s/AgentDataSet]"% species_name)
        agents = h["/Populations/%s/AgentDataSet" % species_name]
        print("%d lons, %d lats, %d ags" % (len(lons),len(lats), len(agents)))
    
        for  i in range(len(agents)):
            if (i % 1000) == 0:
                print("i %d" % (i))
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
                    #-- end if
                #-- end if
            #-- end for
        #-- end for
        print("found %d agents in region" % (agent_count))
        h.flush()
        h.close()
    #-- end if
else:
    usage(argv[0])
#-- end main
