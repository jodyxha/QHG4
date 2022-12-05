#include <cstdio>
#include <cstring>
#include <cmath>


#include <vector>

#include <hdf5.h>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "ParamReader.h"

#include "QDFUtils.h"
#include "QDFUtilsT.h"

#include "SCellGrid.h"
#include "Geography.h"

#include "GroupReader.h"
#include "GridGroupReader.h"
#include "GridWriter.h"
#include "GeoGroupReader.h"
#include "GeoWriter.h"

#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"
#include "IcoGridNodes.h"
#include "IcoNode.h"

#include "shpUtils.h"
#include "shpHeader.h"
#include "shpRecord.h"
#include "dbfReader.h"

typedef std::vector<vecdoubledouble> vecvecdoubledouble;

static const int PR_LINEAR = 7;
const double dDefUseVal = 1.0;


//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
   stdprintf("%s - converting polyline vector data to QDF\n", sApp);
   stdprintf("Usage:\n");
   stdprintf("  %s [-s <surf_file>]  -q <input_qdf>\n", sApp);
   stdprintf("        -v <shp_file> -d <dbf_file> -f <field_name>[:<match_val>[:<use_val>]]\n");
   stdprintf("        [-o <output_qdf>]\n");
   stdprintf("where\n");
   stdprintf("  surf_file    a surface description file (.ico, .ieq, ...). Can be omitted if input_qdf is a regular IEQ file.\n");
   stdprintf("  qdf_file     qdf file corresponding to <surf_file>\n");
   stdprintf("  shp_file     SHP file containig vector data\n");
   stdprintf("  dbf_file     DBF file corresponding to <shp_file>\n");
   stdprintf("  field_name   name of field in <dbf_file> to extract.\n");
   stdprintf("               To see all possible field names, call with '-d <dbf_file> only\n");
   stdprintf("  matchval     target value to select indexes\n");
   stdprintf("  useval       value to use instead of matchval\n");
   stdprintf("  output_qdf   name of output qdf to create.\n");
   stdprintf("               if omitted and input_qdf is given, input_qdf will be modified\n");
   stdprintf("\n");
   stdprintf("<surf_file> and <ign_file> are needed to create a cell grid with geography\n");
   stdprintf("all points contained in the poly lines are extracted and the values for <field_name>\n");
   stdprintf("are used as entries in the QDF files \"Water\" array corresponding to the points' coordinates\n");
   stdprintf("If <match_val> is specified, only polylines whose <field_name> value equals <match_val> are used,\n");
   stdprintf("and the array is set to <use_val> (or 1.0) in the corresponding places\n");
   stdprintf("\n");
   stdprintf("Examples\n");
   stdprintf("List the field names:\n");
   stdprintf("  %s -d ~/rivers/ne_10m_rivers.dbf\n", sApp);
   stdprintf("\n");
   stdprintf("Convert (using CellGrid and Geo from qdf file):\n");
   stdprintf("  %s -q GridSG_ieq_256.qdf -v ~/rivers/ne_10m_rivers.shp -d ~/rivers/ne_10m_rivers.dbf -f strokeweig -o rivers_1a_256.qdf\n",sApp);
   stdprintf("\n");
   stdprintf("\n");
}


//----------------------------------------------------------------------------
// readShapeFile
//
int readShapeFile(const std::string sShapeFile, vecvecdoubledouble &vRecs) {
    int iResult = 0;

    FILE *fIn = fopen(sShapeFile.c_str(), "rb");
    if (fIn != NULL) {
        shpHeader *pShapeHeader = new shpHeader(fIn);
        iResult = pShapeHeader->read();
        if (iResult == 0) {
            //      pShapeHeader->display(pShapeFile);
            while ((iResult == 0) && !feof(fIn)) {
                vecdoubledouble  vCoords;
                shpRecord *pShapeRecord = new shpRecord(fIn);
                iResult = pShapeRecord->read(vCoords);
                if (iResult == 0) {
                    //  pShapeRecord->display("");
                    vRecs.push_back(vCoords);
                } else {
                    if (iResult < 0) {
                       stdprintf("Shape read error\n");
                    }
                }
            }   
        } else {
           stdprintf("Reading of shape file failed\n");
        }
        delete pShapeHeader;
    } else {
       stdprintf("Couldn't open shapefile [%s]\n", sShapeFile);
    }
    if (iResult == 1) {
        iResult = 0;
    }
    //   stdprintf("Finished shapefile, res: %d\n", iResult);
    return iResult;
}

//----------------------------------------------------------------------------
// listDBFFields
//
void listDBFFields(const std::string sDBFFile) {
    FILE *fIn = fopen(sDBFFile.c_str(), "rb");
    if (fIn != NULL) { 
        std::vector<double> vVals;
        dbfReader *pDBFReader = new dbfReader(fIn);
        int  iResult = pDBFReader->read(NULL, vVals);
        if (iResult == 0) {
            const nameoffsets &no = pDBFReader->getOffsets();
            nameoffsets::const_iterator it;
            stdprintf("Names of numerical fields in [%s]\n", sDBFFile);
            for (it = no.begin(); it != no.end(); ++it) {
                stdprintf(" %s\n", it->first.c_str());
            }
        } else {
           stdprintf("Reading of dbf file failed\n");
        }
        delete pDBFReader;
    } else {
       stdprintf("Couldn't open dbf file [%s]\n", sDBFFile);
    }
    
}


//----------------------------------------------------------------------------
// readDBFFile
//
int readDBFFile(const std::string sDBFFile, const std::string sFieldName, std::vector<double> &vVals) {
    int iResult = 0;

    FILE *fIn = fopen(sDBFFile.c_str(), "rb");
    if (fIn != NULL) {
        dbfReader *pDBFReader = new dbfReader(fIn);
        iResult = pDBFReader->read(sFieldName, vVals);
        if (iResult == 0) {
            
        } else {
           stdprintf("Reading of dbf file failed\n");
            iResult = -1;
        }
        delete pDBFReader;
    } else {
       stdprintf("Couldn't open dbf file [%s]\n", sDBFFile);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// collectData
//
int collectData(const std::string sShapeFile,
                const std::string sDBFFile,
                const std::string sFieldName,
                vecvecdoubledouble &vRecs,
                vecdouble &vVals) {
    int iResult =-1;

    iResult = readShapeFile(sShapeFile, vRecs);
    if (iResult == 0) {
        iResult = readDBFFile(sDBFFile, sFieldName, vVals);
        if (iResult == 0) {
            if (vRecs.size() == vVals.size()) {
               stdprintf("Have %zd records\n", vRecs.size());
            } else {
               stdprintf("size mismatch: shp [%zd], dbf [%zd]\n", vRecs.size(), vVals.size()); 
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertRiverData
//
int insertRiverData(vecvecdoubledouble &vRecs, vecdouble &vVals, SCellGrid *pCG, Surface *pSurf, const double dMatchVal, const double dUseVal) {
    int iResult = 0;

    Geography *pGeo = pCG->m_pGeography;
    //   stdprintf("Looping over %zd recs\n", vRecs.size());
    for (uint i = 0; i < vRecs.size(); ++i) {
        vecdoubledouble &vdd = vRecs[i];

        for (uint j = 0; j < vdd.size(); ++j) {

            double dLon = vdd[j].first;
            double dLat = vdd[j].second;
            bool bDeg2Rad = true;

            if (iResult == 0) {
                if (bDeg2Rad) {
                    /*
                    dLon *= M_PI/180;
                    dLat *= M_PI/180;
                    */
                }
                gridtype lNode = pSurf->findNode(dLon, dLat);
                int iIndex = pCG->m_mIDIndexes[lNode];
                if (!std::isnan(dMatchVal)) {
                    if (dMatchVal == vVals[i]) {
                        pGeo->m_adWater[iIndex] = dUseVal;
                    } else {
                        // do nothing
                    }
                } else {
                    pGeo->m_adWater[iIndex] = vVals[i];
                }

                //stdprintf("Path %d, segment %d:(%f,%f)->%d:  %f(%f)\n", i, j, dLon*180/M_PI, dLat*180/M_PI, iIndex, dVal, vVals[i]);
                
            }
            
        }
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// initializeGeography
//  create Geography, set
//   Longitude
//   Latitude
//   Distances
//   Area
//
int initializeGeography(SCellGrid *pCG, IcoGridNodes *pIGN) {

    int iResult = 0;
    
    bool bDeg2Rad = true;
    // rectangular grids with linear "projection" should not 
    // have their coordinates modified
    stdprintf("Testing type of IGN surface:[%s]\n", pCG->m_smSurfaceData[SURF_TYPE].c_str());
    if (pCG->m_smSurfaceData[SURF_TYPE] == SURF_LATTICE) {
       stdprintf("  --> is lattice\n");
        iResult = -1;
        std::string sPT = pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE];
        stdprintf("PROJ type  --> [%s]\n", sPT);

        stringvec vParts;
        uint iNum = splitString(sPT, vParts, " ");
        if (iNum > 0) {
            int iPT=-1;
            if (strToNum(vParts[0], &iPT)) {
                iResult = 0;
                if (iPT == PR_LINEAR) {
                   stdprintf("have LINEAR\n");
            
                    bDeg2Rad = false;
                }
            } else {
                stdprintf("bat projType [%s}n", vParts[0]);
            }
        }
        /*
        char *p = strtok(sPT, " ");
        if (p != NULL) {
            char *pEnd;
            int iPT = (int)strtol(p, &pEnd, 10);
            stdprintf("First word [%s]\n", p);
            if (*pEnd == '\0') {
                iResult = 0;
                if (iPT == PR_LINEAR) {
                   stdprintf("have LINEAR\n");
            
                    bDeg2Rad = false;
                }
            }
        }
        */
    }
    
    if (iResult == 0) {
        Geography *pGeo = pCG->m_pGeography;

        for (uint i=0; i< pCG->m_iNumCells; ++i) {
            gridtype iIndex = pCG->m_aCells[i].m_iGlobalID;  // for each cell find its ID
            IcoNode* pIN = pIGN->m_mNodes[iIndex];           // get the corresponding iconode in pIGN

       
            if(pIN != NULL) {
                pGeo->m_adAltitude[iIndex] = 0;

                pGeo->m_adLatitude[iIndex]  =  pIN->m_dLat;
                pGeo->m_adLongitude[iIndex] =  pIN->m_dLon;
                if (bDeg2Rad) {
                    pGeo->m_adLatitude[iIndex]  *=  180/M_PI;
                    pGeo->m_adLongitude[iIndex] *=  180/M_PI;
                }
                // the neighbor arrays are arranged sequentially into a big 1-d array
                int i0 = pGeo->m_iMaxNeighbors*iIndex;
                for (int j = 0; j < pIN->m_iNumLinks; j++) {
                    pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
                }
                pGeo->m_adWater[iIndex] = 0;
                pGeo->m_adArea[iIndex] = pIN->m_dArea;
                pGeo->m_abIce[iIndex] = false;

            } else {
                stdfprintf(stderr,"[initializeGeography] node of index %d not found\n",iIndex);
                iResult = -1;
            }
        }
    } else {
        stdfprintf(stderr,"[initializeGeography] couldn't read projection details\n");
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int createCells(IcoGridNodes *pIGN, SCellGrid *pCG) { // THIS IS FOR ICOSAHEDRON GRID
    
    int iC = 0;
    pCG->m_aCells = new SCell[pCG->m_iNumCells];
    std::map<gridtype, IcoNode*>::const_iterator it;
    for (it = pIGN->m_mNodes.begin(); it != pIGN->m_mNodes.end(); ++it) {
        pCG->m_mIDIndexes[it->first]=iC;
        pCG->m_aCells[iC].m_iGlobalID    = it->first;
        pCG->m_aCells[iC].m_iNumNeighbors = (uchar)it->second->m_iNumLinks;
        //        pCF->setGeography(m_pGeography, iC, it->second);
        iC++;
    }

 
    // linking and distances
    for (uint i =0; i < pCG->m_iNumCells; ++i) {
        // get link info from IcCell
        IcoNode *pIN = pIGN->m_mNodes[pCG->m_aCells[i].m_iGlobalID];
        for (int j = 0; j < pIN->m_iNumLinks; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = pCG->m_mIDIndexes[pIN->m_aiLinks[j]];
        }
        for (int j = pIN->m_iNumLinks; j < MAX_NEIGH; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// getSurface
//   try as Lattice, as IEQ or ICO
//
Surface *getSurface(const std::string sSurfFile, int *piNumCells) {
    int iResult = 0;
    Surface *pSurf = NULL;
    *piNumCells = -1;
    Lattice *pLat = new Lattice();
    iResult = pLat->load(sSurfFile);
    if (iResult == 0) {
        pSurf = pLat;
       stdprintf("Have Lattice\n");
        *piNumCells = pLat->getLinkage()->getNumVertices();
    } else {
        EQsahedron *pEQ = EQsahedron::createEmpty();
        iResult = pEQ->load(sSurfFile);
        if (iResult == 0) {
            
            pEQ->relink();
            pSurf = pEQ;
           stdprintf("Have EQsahedron\n");
            *piNumCells = pEQ->getLinkage()->getNumVertices();
        } else {
            Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
            pIco->setStrict(true);
            bool bPreSel = false;
            pIco->setPreSel(bPreSel);
            iResult = pIco->load(sSurfFile);
            if (iResult == 0) {
               stdprintf("Have Icosahedron\n");
                *piNumCells = pIco->getLinkage()->getNumVertices();
            } else {
                pSurf = NULL;
            }
        }
    }
    return pSurf;
}


//-----------------------------------------------------------------------------
// createCGFromIGN
//   
//
SCellGrid *createCGFromIGN(const std::string sIGNFile, int *piNumCells) {
    int iResult = 0;
    SCellGrid *pCG = NULL;
    IcoGridNodes *pIGN = new IcoGridNodes();
    iResult = pIGN->read(sIGNFile.c_str());
    if (iResult == 0) {
        *piNumCells = pIGN->m_mNodes.size();
        
        pCG = new SCellGrid(0, *piNumCells, pIGN->getData());
                                
        createCells(pIGN,pCG);
                                
                                
        // create new geography
        Geography *pGeo = NULL;
        pGeo = new Geography(*piNumCells, 6, 6371.0);  // create geography
        memset(pGeo->m_adWater, 0, pCG->m_iNumCells*sizeof(double));
        pCG->setGeography(pGeo);
        initializeGeography(pCG, pIGN);
                                
        
    } else {
       stdprintf("Couldn't read from [%s]\n", sIGNFile);
    }
    delete pIGN;
    return pCG;
}


//-----------------------------------------------------------------------------
// createCGFromQDF
//   
SCellGrid *createCGFromQDF(const std::string sInputQDF, int *piNumCells) {
    int iResult = -1;
   stdprintf("Creating CG from QDF\n");
    SCellGrid *pCG = NULL;
    hid_t hFile = qdf_openFile(sInputQDF, true);
    if (hFile > 0) {
        GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
        if (pGR != NULL) {
            GridAttributes ga;
            iResult = pGR->readAttributes(&ga);
            if (iResult == 0) {
                pCG = new SCellGrid(0, ga.m_iNumCells, ga.smData);
                pCG->m_aCells = new SCell[ga.m_iNumCells];
                
                iResult = pGR->readData(pCG);
                if (iResult == 0) {
                    GeoGroupReader *pGeoR = GeoGroupReader::createGeoGroupReader(hFile);
                    if (pGeoR != NULL) {
                        GeoAttributes geoa; 
                        iResult = pGeoR->readAttributes(&geoa);
                        if (iResult == 0) {
                            if (*piNumCells == (int)ga.m_iNumCells) {
                                Geography *pGeo = new Geography(geoa.m_iNumCells, geoa.m_iMaxNeighbors, geoa.m_dRadius);
                            
                                iResult = pGeoR->readData(pGeo);
                                if (iResult == 0) {
                                   stdprintf("CellGrid created!\n");
                                    pCG->setGeography(pGeo);
                                } else {
                                   stdprintf("Couldn't read geo data from [%s]\n", sInputQDF);
                                }
                            } else {
                               stdprintf("NumCells differs between grid (%d) and geo (%d)\n", *piNumCells, geoa.m_iNumCells);
                            }
                        } else {
                           stdprintf("Couldn't read geo data from [%s]\n", sInputQDF);
                        }
                        delete pGeoR;
                    } else {
                       stdprintf("Couldn't create GeoGroupReader for QDF file [%s]\n", sInputQDF);
                    }
                } else {
                   stdprintf("Couldn't read geo attributes from [%s]\n", sInputQDF);
                }
            } else {
               stdprintf("Couldn't get number of cells from [%s]\n", sInputQDF);
            }
            delete pGR;
        } else {
           stdprintf("Couldn't create GridGroupReader for QDF file [%s]\n", sInputQDF);
        }

        qdf_closeFile(hFile);
    } else {
       stdprintf("Couldn't open QDF file [%s]\n", sInputQDF);
    }
    if (iResult != 0) {
        if (pCG->m_pGeography != NULL) {
            delete pCG->m_pGeography;
        }
        delete pCG;
        pCG = NULL;
    }
    return pCG;
}

const double EPS = 1e-9;
//-----------------------------------------------------------------------------
// createSurfaceFromQDF
//
Surface *createSurfaceFromQDF(const std::string sInputQDF, int *piNumCells) {
    Surface *pSurf = NULL;
    hid_t hFile = qdf_openFile(sInputQDF, "R");
    if (hFile != H5P_DEFAULT) {
        hid_t hGrid = qdf_openGroup(hFile, GRIDGROUP_NAME);
        if (hGrid != H5P_DEFAULT) {
            *piNumCells = 0;
            int iRes = qdf_extractAttribute(hGrid,  GRID_ATTR_NUM_CELLS, 1, piNumCells);
            if (iRes == 0) {
                std::string sType = qdf_extractSAttribute(hGrid, GRID_ATTR_SURF_TYPE);
                if (!sType.empty()) {
                    if (sType == GRID_STYPE_IEQ) {
                        float f1 = sqrt((*piNumCells - 2)/10);
                        if (f1 - int(f1) < EPS) {
                            int iSubDivs = (int)(f1 - 1);
                            pSurf = EQsahedron::createInstance(iSubDivs, true);
                            if (pSurf != NULL) {
                               stdprintf("surface created!\n");
                            } else {
                               stdprintf("Couldn't create surface\n");
                            }
                        } else {
                           stdprintf("bad cell number [%d]\n", *piNumCells);
                        }

                    } else {
                       stdprintf("Can't create surface for type [%s]\n", sType.c_str());
                    }
                } else {
                    
                   stdprintf("Couldn't extract attribute [%s] from grid group\n", GRID_ATTR_SURF_TYPE);
                }
            } else {
                
               stdprintf("Couldn't extract attribute [%s] from grid group\n", GRID_ATTR_NUM_CELLS);
            }
            qdf_closeGroup(hGrid);
        } else {
           stdprintf("Couldn't open grid group in  [%s]\n", sInputQDF);
        }
        qdf_closeFile(hFile);
    } else {
       stdprintf("Couldn't open [%s] asd QDF file\n", sInputQDF);
    }
    return pSurf;
}



//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    std::string sSurfFile   = "";
  
    std::string sSHPFile    = "";
    std::string sDBFFile    = "";
    std::string sFieldName  = "";
    std::string sOutputQDF  = "";
    std::string sInputQDF   = "";

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(6,
                                   "-s:s",           &sSurfFile,
                                   "-q:s",           &sInputQDF,
                                   "-v:s",           &sSHPFile,
                                   "-d:s!",          &sDBFFile,
                                   "-f:s",           &sFieldName,
                                   "-o:s",           &sOutputQDF);
 

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                int iNumCells =0;
         
                if (sFieldName.empty()) {
                    listDBFFields(sDBFFile);
                } else if ((!sInputQDF.empty()) && (!sSHPFile.empty()) && (!sOutputQDF.empty())) {

                    SCellGrid *pCG = NULL;
                    Surface *pSurf = NULL;
                    // get surface
                    if (!sSurfFile.empty()) {
                        pSurf = getSurface(sSurfFile, &iNumCells);
                    }

                    if (pSurf != NULL) {
                        iResult = 0;
                    } else {
                        
                        pSurf = createSurfaceFromQDF(sInputQDF, &iNumCells);
                        if (pSurf != NULL) {
                            if (sOutputQDF.empty()) {
                                sOutputQDF = sInputQDF;
                            }
                            pCG = createCGFromQDF(sInputQDF, &iNumCells);
                            iResult = 0;
                        } else {
                        }
                          
                        if (pCG == NULL) {
                            iResult = -1;
                        }
                    }



                    

                    double dMatchVal = dNaN;
                    double dUseVal = -1;
                    if (iResult ==0) {
                        stringvec vParts;
                        uint iNumVals = splitString(sFieldName, vParts, ":");
                        if (iNumVals > 0) {
                            sFieldName = vParts[0];
                            if (iNumVals > 1) {
                                if (strToNum(vParts[1], &dMatchVal)) {
                                    if (iNumVals > 2) {
                                        if (strToNum(vParts[2], &dUseVal)) {
                                            iResult = 0;
                                        } else {
                                            stdprintf("Expected use_val to be a number [%s]\n", vParts[2]);
                                            iResult = -1;
                                        }
                                    }

                                } else {
                                    stdprintf("Expected match_val to be a number [%s]\n", vParts[1]);
                                    iResult = -1;
                                }
                            }
                        } else {
                            stdprintf("This should not happen here: field_name is emptyn");
                            iResult = -1;
                        }
                    }                                           
                    /*
                        char *pMatchValStr = strchr(sFieldName, ':');
                        if (pMatchValStr != NULL) {
                            *pMatchValStr = '\0';
                            pMatchValStr++;
                            pUseVal =&dDefUseVal;
                                    
                            char *pUseValStr = strchr(pMatchValStr, ':');
                            if (pUseValStr != NULL) {
                                *pUseValStr = '\0';
                                pUseValStr++;
                                if (strToNum(pUseValStr, &dUseVal)) {
                                    pUseVal =  &dUseVal;
                                } else {
                                   stdprintf("Bad use value: [%s]\n", pUseValStr);
                                    iResult = -1;
                                }
                            }
                            if (strToNum(pMatchValStr, &dMatchVal)) {
                                pMatchVal =  &dMatchVal;

                            } else {
                               stdprintf("Bad match value: [%s]\n", pMatchValStr);
                                iResult = -1;
                            }
                        }
                    
                    } 
                    */

                    if (iResult ==0) {
                        vecvecdoubledouble vRecs;
                        vecdouble vVals;
                        stdprintf("collecting data\n");
                        iResult = collectData(sSHPFile, sDBFFile, sFieldName, vRecs, vVals);
                        if (iResult == 0) {
                       

                            stdprintf("inserting rivers\n");
                            iResult = insertRiverData(vRecs, vVals, pCG, pSurf, dMatchVal, dUseVal); 
                                
                            // dummy time value
                            float fTime = -1;
                            int iStep = -1;
                            if (sInputQDF == sOutputQDF) {
                                hid_t hFile = qdf_openFile(sOutputQDF,true);     
                                if (hFile >= 0) {
                                    GeoWriter *pGeoW = new GeoWriter(pCG->m_pGeography);
                                    pGeoW->replace(hFile);
                                    qdf_closeFile(hFile);
                                    stdprintf("Written to QDF file [%s]\n", sOutputQDF);
                                } else {
                                    stdprintf("Couldn't open QDF file [%s]\n", sOutputQDF);
                                }
                            } else {
                                hid_t hFile = qdf_createFile(sOutputQDF, iStep, fTime, "VectorImport");
                            
                                if (hFile >= 0) {
                                    GridWriter *pGridW = new GridWriter(pCG, NULL);
                                    pGridW->write(hFile);
                                    
                                    GeoWriter *pGeoW = new GeoWriter(pCG->m_pGeography);
                                    pGeoW->write(hFile);
                                    qdf_closeFile(hFile);
                                    stdprintf("Written to QDF file [%s]\n", sOutputQDF);
                                } else {
                                    stdprintf("Couldn't create QDF file [%s]\n", sOutputQDF);
                                }
                            }
                        }
                    }
                    delete pSurf;
                    delete pCG;
        
                } else {
                    usage(apArgV[0]);
                }
            } else {
                usage(apArgV[0]);
            }


        } else {
            stdprintf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        stdprintf("Couldn't create ParamReader\n");
    }
    
    return iResult;
}
