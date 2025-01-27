#include <cstdio>

#include <vector>

#include <hdf5.h>

#include "ParamReader.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"


#include "QDFUtils.h"

#include "SCellGrid.h"
#include "Geography.h"

#include "GeoWriter.h"
#include "GridWriter.h"

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


//----------------------------------------------------------------------------
// usage
//
void usage(const std::string sApp) {
    xha_printf("%s - converting shp+dbf data to QDF\n", sApp);
    xha_printf("Usage:\n");
    xha_printf("  %s -s <surface_> -i <ign_file> \n", sApp);
    xha_printf("     -v <shp_file> -d <dbf_file> -f <field_name>\n");
    xha_printf("     -o <qdf_output>\n");
    xha_printf("where\n");
    xha_printf("  surface     surface file for icosahdron\n");
    xha_printf("  ign_file    IGN file corresponding to <surface>\n");
    xha_printf("  shp_file    a SHP file containing 'PolyLine vector data\n");
    xha_printf("  dbf_file    DBF file corresponding to <shp_file>\n");
    xha_printf("  field_name  fieldname in <dbf_file> to extract\n");
    xha_printf("\n");
    xha_printf("Example:\n");
    xha_printf("  %s -s resources/eq128.ieq -i resources/eq128_000.ign\n", sApp);
    xha_printf("     -v water/ne_10m_coastline.shp -d water/ne_10m_coastline.dbf\n");
    xha_printf("     -f strokeweig -o coastline_128.qdf\n");
    xha_printf("\n");
}


//----------------------------------------------------------------------------
// readShapeFile
//
int readShapeFile(const std::string sShapeFile, vecvecdoubledouble &vRecs) {
    int iResult = -1;

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
                        xha_printf("Shape read error\n");
                    }
                }
            }   
        } else {
            xha_printf("Reading of shape file failed\n");
        }
        delete pShapeHeader;
    } else {
        xha_printf("Couldn't open [%s]\n", sShapeFile);
    }
    if (iResult == 1) {
        iResult = 0;
    }
    xha_printf("Finished shapefile, res: %d\n", iResult);
    return iResult;
}


//----------------------------------------------------------------------------
// readDBFFile
//
int readDBFFile(const std::string sDBFFile, const std::string sFieldName, std::vector<double> &vVals) {
    int iResult = -1;

    FILE *fIn = fopen(sDBFFile.c_str(), "rb");
    if (fIn != NULL) {
        dbfReader *pDBFReader = new dbfReader(fIn);
        iResult = pDBFReader->read(sFieldName, vVals);
        if (iResult == 0) {
            
        } else {
            xha_printf("Reading of dbf file failed\n");
        }
        delete pDBFReader;
    } else {
        xha_printf("Couldn't open [%s]\n", sDBFFile);
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
        iResult = readDBFFile(pDBFFile, sFieldName, vVals);
        if (iResult == 0) {
            if (vRecs.size() == vVals.size()) {
                xha_printf("Have %zd records\n", vRecs.size());
            } else {
                xha_printf("size mismatch: shp [%zd], dbf [%zd]\n", vRecs.size(), vVals.size()); 
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// insertRiverData
//
int insertRiverData(vecvecdoubledouble &vRecs, vecdouble &vVals, SCellGrid *pCG, Surface *pSurf) {
    int iResult = 0;

    Geography *pGeo = pCG->m_pGeography;
    memset(pGeo->m_adRivers, 0, pCG->m_iNumCells*sizeof(double));
    xha_printf("Looping over %zd recs\n", vRecs.size());
    for (uint i = 0; i < vRecs.size(); ++i) {
        vecdoubledouble &vdd = vRecs[i];

        for (uint j = 0; j < vdd.size(); ++j) {

            double dLon = vdd[j].first;
            double dLat = vdd[j].second;
            bool bDeg2Rad = true;
            
            /*
            // for LINEAR projections do not transform to radians
            if (pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
                iResult = -1;
                char sPT[256];
                strcpy(sPT, pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE].c_str());
                char *p = strtok(sPT, " ");
                if (p != NULL) {
                    char *pEnd;
                    int iPT = strtol(p, &pEnd, 10);
                    if (*pEnd == '\0') {
                        iResult = 0;
                        if (iPT == PR_LINEAR) {
                            // xha_printf("have LINEAR\n");
                            bDeg2Rad = false;
                        }
                    }
                }
            }
            */
            if (iResult == 0) {
                if (bDeg2Rad) {
                    /*
                    dLon *= Q_PI/180;
                    dLat *= Q_PI/180;
                    */
                }
                gridtype lNode = pSurf->findNode(dLon, dLat);
                int iIndex = pCG->m_mIDIndexes[lNode];
                double dVal = vVals[i];
                // coastlie.scalerank=0 for all
                if (dVal == 0) {
                    dVal = 1;
                }
                xha_printf("Path %d, segment %d:(%f,%f)->%d:  %f\n", i, j, dLon*180/Q_PI, dLat*180/Q_PI, iIndex, dVal);
                pGeo->m_adRivers[iIndex] = vVals[i];
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
    xha_printf("Testing type of IGN surface:[%s]\n", pCG->m_smSurfaceData[SURF_TYPE].c_str());
    if (pCG->m_smSurfaceData[SURF_TYPE].compare(SURF_LATTICE) == 0) {
        xha_printf("  --> is lattice\n");
        iResult = -1;
        char sPT[256];
        strcpy(sPT, pCG->m_smSurfaceData[SURF_LTC_PROJ_TYPE].c_str());
        xha_printf("PROJ type  --> [%s]\n", sPT);

        char *p = strtok(sPT, " ");
        if (p != NULL) {
            char *pEnd;
            int iPT = (int)strtol(p, &pEnd, 10);
            xha_printf("First word [%s]\n", p);
            if (*pEnd == '\0') {
                iResult = 0;
                if (iPT == PR_LINEAR) {
                    xha_printf("have LINEAR\n");
            
                    bDeg2Rad = false;
                }
            }
        }
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
                    pGeo->m_adLatitude[iIndex]  *=  180/Q_PI;
                    pGeo->m_adLongitude[iIndex] *=  180/Q_PI;
                }
                // the neighbor arrays are arranged sequentially into a big 1-d array
                int i0 = pGeo->m_iMaxNeighbors*iIndex;
                for (int j = 0; j < pIN->m_iNumLinks; j++) {
                    pGeo->m_adDistances[i0+j] = pIN->m_adDists[j];
                }
                pGeo->m_adRivers[iIndex] = pIN->m_dArea;
                pGeo->m_adArea[iIndex] = pIN->m_dArea;
                pGeo->m_abIce[iIndex] = false;

            } else {
                xha_fprintf(stderr,"[initializeGeography] node of index %d not found\n",iIndex);
                iResult = -1;
            }
        }
    } else {
        xha_fprintf(stderr,"[initializeGeography] couldn't read projection details\n");
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// createCells
//   create cells
//   link cells
//
int createCells(SCellGrid *pCG, IcoGridNodes *pIGN) { // THIS IS FOR ICOSAHEDRON GRID
    
    
    
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
        for (int j = pIN->m_iNumLinks; j < pCG->m_iMaxNeighbors; ++j) {
            pCG->m_aCells[i].m_aNeighbors[j] = -1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char *sSurface = NULL;
    char *sIGNFile = NULL;
    char *sSHPFile = NULL;
    char *sDBFFile = NULL;
    char *sFieldName = NULL;
    char *sOutputQDF = NULL;
 

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(6,
                                   "-s:S",          &sSurface,
                                   "-i:S",          &sIGNFile,
                                   "-v:S",          &sSHPFile,
                                   "-d:S",          &sDBFFile,
                                   "-f:S",          &sFieldName,
                                   "-o:S",          &sOutputQDF);


        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                
                int iNumCells =0;

                // get surface
                Surface *pSurf = NULL;

                Lattice *pLat = new Lattice();
                iResult = pLat->load(sSurface);
                if (iResult == 0) {
                    pSurf = pLat;
                    xha_printf("Have Lattice\n");
                    iNumCells = pLat->getLinkage()->getNumVertices();
                } else {
                    EQsahedron *pEQ = EQsahedron::createEmpty();
                    iResult = pEQ->load(sSurface);
                    if (iResult == 0) {
                
                        pEQ->relink();
                        pSurf = pEQ;
                        xha_printf("Have EQsahedron\n");
                        iNumCells = pEQ->getLinkage()->getNumVertices();
                    } else {
                        Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                        pIco->setStrict(true);
                        bool bPreSel = false;
                        pIco->setPreSel(bPreSel);
                        iResult = pIco->load(sSurface);
                        xha_printf("Have Icosahedron\n");

                    }
                }
        
                // get cell grid from ign file
                SCellGrid *pCG = NULL;
                if (pSurf != NULL) {
                    xha_printf("Surface OK\n");
                    IcoGridNodes *pIGN = new IcoGridNodes();
                    iResult = pIGN->read(sIGNFile);
                    if (iResult == 0) {
                        iNumCells = pIGN->m_mNodes.size();
            
                        int iC = 0;
                        pCG = new SCellGrid(0, iNumCells, pIGN->getData());
                
                        createCells(pCG, pIGN);
              
                        Geography *pGeo = new Geography(iNumCells, pCG->m_iMaxNeighbors, 6371.0);  // create geography
                        pCG->setGeography(pGeo);
                        initializeGeography(pCG, pIGN);
                

                    } else {
                        xha_printf("Couldn't read from [%s]\n", apArgV[2]);
                    }
                    delete pIGN;
                }
        
                if (iResult ==0) {
                    vecvecdoubledouble vRecs;
                    vecdouble vVals;
                    xha_printf("collecting data\n");
                    iResult = collectData(sSHPFile, sDBFFile, sFieldName, vRecs, vVals);
                    if (iResult == 0) {
                        xha_printf("inserting rivers\n");
                        iResult = insertRiverData(vRecs, vVals, pCG, pSurf); 
            

                        xha_printf("result before writing to [%s]: %d\n", sOutputQDF, iResult);
                        // dummy time value
                        float fTime = -1;
                        // create output file
                        hid_t hFile = qdf_createFile(sOutputQDF, fTime);
                        if (hFile >= 0) {
                            GridWriter *pGridW = new GridWriter(pCG, NULL);
                            pGridW->write(hFile);
                            GeoWriter *pGeoW = new GeoWriter(pCG->m_pGeography);
                            pGeoW->write(hFile);
                            qdf_closeFile(hFile);
                            xha_printf("Written to QDF file [%s]\n", sOutputQDF);
                    
                        } else {
                            xha_printf("Couldn't create QDF file [%s]\n", sOutputQDF);
                        }
                    }
                }
                delete pSurf;
                delete pCG;
        
            } else {
                usage(apArgV[0]);
            }
        } else {
            xha_printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        xha_printf("Couldn't create ParamReader\n");
    }
    return iResult;
}
