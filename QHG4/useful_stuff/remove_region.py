#!/usr/bin/python

from sys import argv
import h5py
import region_utils as ru

#----------------------------------------------------------------------------
#-- usage
#--
def usage(sApp):
    print("%s - remove landmasses defined by region" % sApp)
    print("usage:")
    print("  %s <qdf-file> (<region-name> | <region_def>) " % sApp)
    print("where")
    print("  qdf-file     name of qdf file to moddify")
    print("  region-name  ':'-separated string of names of predefined region:")
    print("                  'africa'")
    print("                  'americas'")
    print("                  'australia'")
    print("                  'australia2'")
    print("                  'antarctica'")
    print("                  'eurasia'")
    print(" region-def    name of file containing vertex list")
    print("               where each line contains the coordinates of one vertex");
    print("")
    print("NOTE: the c++ version 'RemoveRegions' is much faster");
#-- end if


def test_1():
    verts=[(0,0),(4,0),(4,1),(1,1),(1,4),(3,4),(3,3),(2,3),(2,2),(4,2),(4,5),(0,5)]

    test1=(2.5,3.5)
    test2=(2.5,2.5)
    test3=(4.5,2.0)
    test3=(1.5,2.0)

    a = point_in_poly(verts,test1)
    print("point 1 is %sin the poly" % ("" if a else "not "))

    a = point_in_poly(verts,test2)
    print("point 2 is %sin the poly" % ("" if a else "not "))

    a = point_in_poly(verts,test3)
    print("point 3 is %sin the poly" % ("" if a else "not "))

    a = point_in_poly(verts,test3)
    print("point 4 is %sin the poly" % ("" if a else "not "))
#-- end def


#----------------------------------------------------------------------------
#-- main
#--

if (len(argv) > 2):
    verts = []
    h = h5py.File(argv[1])

    vertboxes = {}

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
            if not verts is None:
                bbox = ru.get_bbox(verts)
                vertboxes[argv[2]] = (verts,bbox)
                print(vertboxes)
            #--
        except:
            print("couldn't find region or file named [%s]" % argv[2])
        #-- end try
    #-- end if
    
    if len(vertboxes) > 0:
        lons = h["/Geography/Longitude"]
        lats = h["/Geography/Latitude"]
        alts = h["/Geography/Altitude"]
        print("%d lons, %d lats" % (len(lons),len(lats)))

        iFlips = 0
        for  i in range(len(lons)):
            if (i % 1000) == 0:
                print("i %d" % (i))
            #-- end if
            if (alts[i] > 0):
                is_in_bboxes = False
                for vb in vertboxes:
                    bbox = vertboxes[vb][1]
                    if (lons[i] >= bbox[0]) and \
                           (lats[i] >= bbox[1]) and \
                           (lons[i] <= bbox[2]) and \
                           (lats[i] <= bbox[3]):
                        if ru.point_in_poly(vertboxes[vb][0], (lons[i], lats[i])):
                            alts[i] = -alts[i]
                            iFlips = iFlips+1
                        #-- end if
                    #-- end if
                #-- end if
            #-- end for
        #-- end for
        print("did %d flips\n" % iFlips);
    #-- end if
    h.flush()
    h.close()
else:
    usage(argv[0])
#-- end main
