#!/usr/bin/python

from sys import argv,exit
import h5py

coords = ['Longitude', 'Latitude']

#-----------------------------------------------------------------------------
#-- get_special_namevals
#--
def get_special_namevals(tags):
    special_namevals = {}
    special_tags     = []

    for x in tags:
        if '#' in x:
            special_tags.append(x)
        #-- end if
    #-- end for
    for y in special_tags:
        tags.remove(y)
        k= y.split('#')
        special_namevals[k[0]] = k[1]
    #-- end for
    return special_namevals
#-- end def 


#-----------------------------------------------------------------------------
#-- get_coord_names
#--
def get_coord_names(tags):
    # check if coords needed & load them
    coord_names = []
    for c in coords:
        if c in tags:
            coord_names.append(tags.pop(tags.index(c)))  
        #-- end if
    #-- end for
    return coord_names
#-- end def


#-----------------------------------------------------------------------------
#-- build_header
#--
def build_header(dd, coord_names, special_namevals):
    sHeader = "#"
    if selects == 'all':
        indexes = range(len(dd))
        sHeader = "%s"%';'.join(dd)
    else:
        if coord_names:
            sHeader = sHeader + "%s;"%(';'.join(coord_names))
        #-- end if
        sHeader = sHeader + "%s"%';'.join(tags)
    #-- end if
    if special_namevals:
        for t in special_namevals:
            sHeader = sHeader + ";%s"%t
        #-- end for
    #-- end if
    return sHeader
#-- end def



if len(argv) > 3:
    qdf_pop = argv[1]
    offs = 1
    if (len(argv) > 4):
        qdf_geo = argv[2]
        offs = 2;
    else:
        qdf_geo = None
    #-- end if
    selects = argv[offs+1]
    outfile = argv[offs+2]

    t = qdf_pop.split(':')
    pop_qdf = t[0]
    if (len(t) > 1):
        tags = selects.split(':')

        coord_names      = get_coord_names(tags)
        special_namevals = get_special_namevals(tags)

        fOut = open(outfile, 'w')

        print("opening [%s]"%pop_qdf)
        # load agent data and prepare selection indexes
        species = t[1]
        hPop= h5py.File(pop_qdf, 'r')
        attrs = hPop['Populations/%s/AgentDataSet'%species]
        print("got attrs: %d"%len(attrs))
        attr_list=list(attrs)
        dd = attrs.dtype.names
        cp = dd.index('CellID')

        print("mmaking list")
        # make list of indexes of requires items
        indexes = [dd.index(x) if x in dd else None for x in tags]
        while None in indexes:
            indexes.remove(None)
        #-- end while
        print("tags:%s"%str(tags))
        # construct the header
        sHeader = build_header(dd, coord_names, special_namevals)
        
        fOut.write("%s\n"%sHeader)

        coord_arr = {}
        if coord_names:
            if ('Geography' in hPop):
                hGeo = hPop
            else:
                if not qdf_geo is None:
                    hGeo =  h5py.File(qdf_geo, 'r')
                else:
                    raise Exception("No 'Geography' in %s and no GeoQDF provided"%pop_qdf)
                #-- end if
            #-- end if
            for x in coord_names:
                coord_arr[x] = hGeo['Geography/%s'%x]
            #-- end for
        #-- end if
        
        
        # in att we have 'LifeState', 'CellIdx', 'CellID', 'AgentID', 'BirthTime', 'Gender', 'Age', 'LastBirth',
        for att in attrs:
            al=list(att)

            # build value line: coords, then tags, then special
            s=[]
            if coord_names:
                ci = al[cp]
                for x in coord_names:
                    s.append(str(coord_arr[x][ci]))
                #-- end for
            #-- end if

            s.extend([str(al[u]) for u in indexes])
            
            if special_namevals:
                for t in special_namevals:
                    s.append(special_namevals[t])
                #-- end for
            #-- end if

            k=';'.join(s)
            fOut.write("%s\n"%k)
            
        #-- end for

        # close qdfs & output file
        hPop.close()
        if coord_names:
            hGeo.close()
        #-- end if
        fOut.close()
        
    else:
        print("no species appended to file name [%s]"%qdf_pop)
    #-- end if
else:
    print("%s - extracting agent data"%argv[0])
    print("  %s <qdf_pop>:<species> [<qdf_geo>] <items> <csv_out>"%argv[0])
    print("where")
    print("  qdf_pop   qdf file containing a greoup 'Population/<species>'")
    print("  qdf_geo   geo file must be provided if coords are requested and qdf_pop has no 'Geography' group")
    print("  items     item selection (enclose in quotes to avoid problems)")
    print("            format:")
    print("              items          ::= <selection_list> | 'all'")
    print("              selection_list ::= <VarNameEx>[':'<VarNameEx>]*[':'<SpecialTag>]*")
    print("              VarNameEx      ::= 'Longitude' | 'Latitude' | <VarName>")
    print("              SpecialTag     ::= <ColName>'#'<SpecialValue>")
    print("              VarName        : name of an AgentDataSet variable")
    print("              'all'          : get *all* items of the AgentData")
    print("  cvs_out   output file")
    print("Examples:")
    print("No coords requested:")
    print("  %s populations.qdf:sapiens  'LifeState:CellIdx:CellID:AgentID:BirthTime:Gender:Age:LastBirth' out.dat"%argv[0])
    print("Coords requested, Populations.qdf contains 'Geography' group:")
    print("  %s populations.qdf:sapiens   'Longitude:Latitude:LifeState:AgentID:Gender' out.dat"%argv[0])
    print("Coords requested, Populations.qdf does not contains 'Geography' group:")
    print("  %s populations.qdf:sapiens   geography.qdf 'Longitude:Latitude:LifeState:AgentID:Gender' out.dat"%argv[0])
    print("Coords requested, Populations.qdf contains 'Geography' group, ad column 'Species' with constant value 1:")
    print("  %s populations.qdf:sapiens   'Longitude:Latitude:LifeState:AgentID:Gender:Species#1' out.dat"%argv[0])
          
    
#-- end if
         
