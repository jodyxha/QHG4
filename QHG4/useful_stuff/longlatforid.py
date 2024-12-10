#!/usr/bin/python


from sys import argv, exit, stderr
import numpy as np
import h5py




class Node2Coords:
        
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdffile):
        self.numcells   = 0
        self.longitudes = None
        self.latitudes  = None
        self.altitudes  = None
        self.ice        = None
        self.npp        = None
        self.cap        = None
        self.bOK        = False
        self.openArrays(qdffile)
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- openArrays
    #--
    def openArrays(self, qdffile):
        try: 
            qdf = h5py.File(qdffile,'r+')
        except:
            stderr.write("Couldn't open[%s] as QDF file\n" % qdffile)
        else:
            self.bOK = True
            if (qdf.__contains__('Grid')):
                grid = qdf['Grid']
                attrs = grid.attrs
                if (attrs.__contains__('NumCells')):
                    self.numcells = int(attrs['NumCells'][0])
                    stderr.write("numcells %d\n" % (self.numcells)) 
                    if qdf.__contains__('Geography'):
                        geo = qdf['/Geography']
                        if geo.__contains__('Longitude'):
                            self.longitudes=geo['Longitude']
                            stderr.write("Found %d longitude values: %f\n" % (len(self.longitudes), self.longitudes[0]))
                        else:
                            stderr.write("'Geography' group does not contain 'Longitude' data set\n")
                            self.bOK = False
                        #-- end if
                
                        if geo.__contains__('Latitude'):
                            self.latitudes=geo['Latitude']
                            stderr.write("Found %d latitude values\n" % (len(self.latitudes)))
                        else:
                            stderr.write("'Geography' group does not contain 'Latitude' data set\n")
                            self.bOK = False
                        #-- end if
                        if geo.__contains__('Altitude'):
                            self.altitudes=geo['Altitude']
                            stderr.write("Found %d altitude values\n" % (len(self.altitudes)))
                        else:
                            stderr.write("'Geography' group does not contain 'Altitude' data set\n")
                            self.bOK = False
                        #-- end if
                        if geo.__contains__('IceCover'):
                            self.ice=geo['IceCover']
                            stderr.write("Found %d ice values\n" % (len(self.ice)))
                        else:
                            stderr.write("'Geography' group does not contain 'IceCover' data set\n")
                            self.bOK = False
                        #-- end if
                    else:
                        stderr.write("No 'Geography' group in [%s]\n" % (qdffile))
                        self.bOK = False
                    #-- end if  
                    if qdf.__contains__('Vegetation'):
                        veg = qdf['/Vegetation']
                        if veg.__contains__('NPP'):
                            self.npp=veg['NPP']
                            stderr.write("Found %d npp values: %f\n" % (len(self.npp), self.npp[0]))
                        else:
                            stderr.write("'Vegetation' group does not contain 'NPP' data set\n")
                            #self.bOK = False
                        #-- end if
                    else:
                        stderr.write("No 'Vegetation' group in [%s]\n" % (qdffile))
                        #self.bOK = False
                else:
                    stderr.write("No 'NumCells' attribute in Grid group\n")
                    self.bOK = False
                #-- end if

            else:
                stderr.write("No 'Grid' group in [%s]\n" % (qdffile))
                self.bOK = False
            #-- end if  
        #-- end if
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- getLonLat
    #--
    def getLonLat(self, nodeid):
        return  (self.longitudes[nodeid], self.latitudes[nodeid])
    #-- end if
    
        
    #-------------------------------------------------------------------------
    #-- getValuesNice
    #--
    def getValuesNice(self, nodeid):
        if (nodeid < self.numcells):
            print("Node: %d" % (nodeid))
            self.longitudes[0]
            print("  Lon %f\n  Lat %f" % (self.longitudes[nodeid], self.latitudes[nodeid]))
            print("  Alt %f\n  Ice %f" % (self.altitudes[nodeid], self.ice[nodeid]))
            if (not self.npp is None):
                print("  NPP %f" % (self.npp[nodeid],))
            #-- end if
            if (not self.cap is None):
                print("  cap %f" % (self.cap[nodeid],))
            #-- end if
        else:
            print("Node ID (%d)must  be less than %d" % (nodeid, self.numcells))
        #-- end if
    #-- end def

    #-------------------------------------------------------------------------
    #-- writeHeaderCSV
    #--
    def writeHeaderCSV(self):
        print("nodeID,longitude,latitude,altitude,ice")
    #-- end def
    
    #-------------------------------------------------------------------------
    #-- getValuesCSV
    #--
    def getValuesCSV(self, nodeid):
        if (nodeid < self.numcells):
            print("%d,%f,%f,%f,%d" % (nodeid,self.longitudes[nodeid], self.latitudes[nodeid],self.altitudes[nodeid], self.ice[nodeid]))
        else:
            print("Node ID (%d)must  be less than %d" % (nodeid, self.numcells))
        #-- end if
    #-- end def

if __name__ == '__main__':

    if (len(argv) > 2):
        q=Node2Coords(argv[1])
        bOK = q.bOK
        if bOK:
            mode=argv[2]
            if argv[3].isnumeric():
                nodes = argv[3:]
            else:
                try:
                    f = open(argv[3])
                    nodes=[]
                    for line in f:
                        x= line.strip()
                        if (x):
                            nodes.append(int(x))
                        #-- end if
                    #-- end for
                    f.close()
                except:
                    stderr.write("couldn't open data file\n")
                    bOK = False
                #-- end try
            #-- end if
        #-- end if
        if bOK:
            if (mode == "csv"):
                q.writeHeaderCSV()
                for node in nodes:
                    q.getValuesCSV(int(node))
                #-- end for
            elif (mode == "nice"):
                for node in nodes:
                    q.getValuesNice(int(node))
            else:
                stderr.write("unknown mode: [%s]" % (mode))
            #-- end if     
        #-- end if
    else:
        print("%s - show qdf values" % (argv[0]))
        print("Usage")
        print("  %s <qdf-file> <mode> <nodeid>* " % (argv[0]))
        print("or")
        print("  %s <qdf-file> <mode> <nodefile> " % (argv[0]))
        print("where")
        print("  qdf-file   a qdf file with Grid and Geography grtoup")
        print("  mode       \"csv\" or \"nice\"")
        print("  nodeids    node id")
        print("  nodefile   a textfile containing one node ID per line")
        print("The first version returns the geo data of the specified cell IDs.")
        print("The second version returns the geo data for the cell IDs found in the data file.")
        print("")
        print("Examples")
        print("  %s  navworld_085_kya_256.qdf nice 123 4565 7878 31112 0" % (argv[0]))
        print("  %s  navworld_085_kya_256.qdf csv:nodedata.txt" % (argv[0]))
        print("")
#-- endmain
