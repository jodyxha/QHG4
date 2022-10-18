#include <cstdio>
#include <vector>
#include <map>
#include <hdf5.h>

#include "types.h"
#include "QDFUtils.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "Vegetation.h"
#include "GroupReader.h"
#include "GeoGroupReader.h"
#include "VegGroupReader.h"
#include "SurfaceGrid.h"


typedef std::map<int, std::vector<int>> neighlist;
typedef std::map<int, int>              ididxlist;

//----------------------------------------------------------------------------
// pruneNodes
//
int pruneNodes(const SCellGrid *pCG, ididxlist &mIdIdx) {
    int iResult = 0;
    int iIndex = 0;

    printf("Eliminating underwater and single cells\n");
    double *pAltitudes = m_pCG->m_pGeography->m_adAltitude;
    SCell * aCells = pCG->m_aCells;
    for (uint i = 0; i < pCG->m_iNumCells; i++) {
        bool bSearching = true;
        if (pAltitudes[aCells[i].m_iGlobalID] > 0) {
            
            for (int j = 0; bSearching && (j < aCells[i].m_iNumNeighbors); j++) {
                int iNeighID =  aCells[aCells[i].m_aNeighbors[j]].m_iGlobalID;
                if (pAltitudes[iNeighID] > 0)  {
                    bSearching = false;
                }
            }
        }
        if (!bSearching) {
            printf("adding %d -> %d\n", aCells[i].m_iGlobalID, iIndex);
            mIdIdx[aCells[i].m_iGlobalID] = iIndex++;
        }
    }
    return iResult;
}

   
//----------------------------------------------------------------------------
// convertNeighbors
//
int convertNeighbors(const SCellGrid *pCG, ididxlist &mIdIdx, neighlist &mvNeigh) { 
    int iResult = 0;
    printf("Saving connections for %zd cells\n", mIdIdx.size());
    SCell * aCells = pCG->m_aCells;
    double *pAltitudes = m_pCG->m_pGeography->m_adAltitude;
    std::map<gridtype, int>::iterator it;
    for (it = mIdIdx.begin(); it != mIdIdx.end(); ++it) {
        SCell &sCell = aCells[it->first];
        std::string s = std::to_string(it->second)+"  ";
        std::vector<int> vNeighs;
        for (int j = 0; j < sCell.m_iNumNeighbors; j++) {
            int iNeighID = aCells[sCell.m_aNeighbors[j]].m_iGlobalID;
            if (pAltitudes[iNeighID] > 0) {
                vNeighs.push_back(mIdIdx[iNeighID]);
            }
        }
        
        if (vNeighs.size() > 0) {
            mvNeigh[it->second] = vNeighs;
        }
    }
    printf("Have %zd lines of neighbor info\n", mvNeigh.size());
    return iResult;
}


//----------------------------------------------------------------------------
// writeNeighborList
//
int writeNeighborList(const neighlist &mvNeigh, const char *sOut) {
    int iResult = 0;
    
    char s1[256];
    sprintf(s1, "%s.conn", sOut);
    FILE *fOut = fopen(s1, "wt");
    if (fOut != NULL) {
        
        printf("Writing connections for %zd cells\n", mvNeigh.size());
        neighlist::const_iterator it;
        for (it = mvNeigh.begin(); it != mvNeigh.end(); ++it) {
            std::string s = std::to_string(it->first)+"  ";
            const std::vector<int> &v = it->second;
            for (uint j = 0; j < v.size(); j++) {
                s = s + " " +std::to_string(v[j]);
            }
            fprintf(fOut, "%s\n", s.c_str());

        }
        fclose(fOut);
    } else {
        printf("Couldn't open [%s] for writing\n", sOut);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeIndexTable
//
int writeIndexTable(ididxlist &mIdIdx, const char *sOut) {
    int iResult = -1;
    printf("writing index table for %zd nodes\n", mIdIdx.size()); fflush(stdout);
    char s2[256];
    sprintf(s2, "%s.idx", sOut);
    FILE *fOut2 = fopen(s2, "wt");
    if (fOut2 != NULL) {
        fprintf(fOut2, "# index  ID\n");
        printf("Writing idx->ID list\n");
        ididxlist::const_iterator it;
        for (it = mIdIdx.begin(); it != mIdIdx.end(); ++it) {
            fprintf(fOut2, "%d  %d \n", it->second, it->first);
        }
        fclose(fOut2);
        iResult = 0;
    } else {
        printf("coulfn'd open [%s] for writing idxid list\n", s2); fflush(stdout);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// writeAttributeTable
//
int writeAttributeTable(const SCellGrid *pCG, ididxlist &mIdIdx, const char *sOut) {
    int iResult = -1;

    printf("writing attribute list for %zd nodes\n", mIdIdx.size()); fflush(stdout);
    Geography  *pGeo = m_pCG->m_pGeography;
    Vegetation *pVeg = pCG->getVegetation();
    char s3[256];
    sprintf(s3, "%s.att", sOut);
    FILE *fOut3 = fopen(s3, "wt");
    if (fOut3 != NULL) {
        fprintf(fOut3, "# index  altitude  longitude  latitude  NPP\n");
        ididxlist::const_iterator it;
        for (it = mIdIdx.begin(); it != mIdIdx.end(); ++it) {
            fprintf(fOut3, "%d  %f %f %f %f \n", it->second, pGeo->m_adAltitude[it->first], pGeo->m_adLongitude[it->first], pGeo->m_adLatitude[it->first], pVeg->m_adTotalANPP[it->first]);
        }
        fclose(fOut3);
        iResult = 0;
    } else {
        printf("couldn't open [%s] for writing atttribute list\n", s3); fflush(stdout);
    }

    return iResult;
}



//----------------------------------------------------------------------------
// writeList
//
int writeList(const SCellGrid *pCG, const char *sOut) {
    int iResult = 0;
    int iIndex = 0;
    std::map<gridtype, int>   mIDIndexes;

    char s1[256];
    sprintf(s1, "%s.conn", sOut);
    FILE *fOut = fopen(s1, "wt");
    if (fOut != NULL) {
        printf("Eliminating underwater and single cells\n");
        double *pAltitudes = m_pCG->m_pGeography->m_adAltitude;
        SCell * aCells = pCG->m_aCells;
        for (uint i = 0; i < pCG->m_iNumCells; i++) {
            bool bSearching = true;
            if (pAltitudes[aCells[i].m_iGlobalID] > 0) {
                
                for (int j = 0; bSearching && (j < aCells[i].m_iNumNeighbors); j++) {
                    int iNeighID =  aCells[aCells[i].m_aNeighbors[j]].m_iGlobalID;
                    if (pAltitudes[iNeighID] > 0)  {
                        bSearching = false;
                    }
                }
            }
            if (!bSearching) {
                mIDIndexes[aCells[i].m_iGlobalID] = iIndex++;
            }
        }
        
        printf("Writing connections for %zd cells\n", mIDIndexes.size());
        std::map<gridtype, int>::iterator it;
        for (it = mIDIndexes.begin(); it != mIDIndexes.end(); ++it) {
            SCell &sCell = aCells[it->first];
            std::string s = std::to_string(it->second)+"  ";
            int iC = 0;
            for (int j = 0; j < sCell.m_iNumNeighbors; j++) {
                int iNeighID = aCells[sCell.m_aNeighbors[j]].m_iGlobalID;
                if (pAltitudes[iNeighID] > 0) {
                    s = s + " " +std::to_string(mIDIndexes[iNeighID]);
                    iC++;
                }
            }

            if (iC > 0) {
                fprintf(fOut, "%s\n", s.c_str());
            }
        }
        fclose(fOut);
     
        
    } else {
        printf("Couldn't open [%s] for writing\n", sOut);
    }
    
    printf("writing index table\n"); fflush(stdout);
    char s2[256];
    sprintf(s2, "%s.idx", sOut);
    FILE *fOut2 = fopen(s2, "wt");
    fprintf(fOut2, "# index  ID\n");
    printf("Writing id->index list\n");
    std::map<gridtype, int>::iterator it;
    for (it = mIDIndexes.begin(); it != mIDIndexes.end(); ++it) {
        fprintf(fOut2, "%d  %d \n", it->second, it->first);
    }
    fclose(fOut2);

    printf("writing attribute list\n"); fflush(stdout);
    Geography *pGeo = m_pCG->m_pGeography;
    char s3[256];
    sprintf(s3, "%s.att", sOut);
    FILE *fOut3 = fopen(s3, "wt");
    fprintf(fOut3, "# index  altitude  longitude  latitude  NPP\n");
    for (it = mIDIndexes.begin(); it != mIDIndexes.end(); ++it) {
        fprintf(fOut3, "%d  %f %f %f %f \n", it->second, pGeo->m_adAltitude[it->first], pGeo->m_adLongitude[it->first], pGeo->m_adLatitude[it->first], pCG->getVegetation()->m_adTotalANPP[it->first]);
    }
    fclose(fOut3);
    

    return iResult;
}


//----------------------------------------------------------------------------
// createGeography
//
Geography *createGeography(const char *pQDFFile) {
    Geography *pGeo = NULL;
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile > 0) {
        GeoGroupReader *pGeoR = GeoGroupReader::createGeoGroupReader(hFile);
        if (pGeoR != NULL) {
            GeoAttributes geoa; 
            iResult = pGeoR->readAttributes(&geoa);
            if (iResult == 0) {
                Geography *pGeoTemp = new Geography(geoa.m_iNumCells, geoa.m_iMaxNeighbors, geoa.m_dRadius);
                
                iResult = pGeoR->readData(pGeoTemp);
                if (iResult == 0) {
                    pGeo = pGeoTemp;
                } else {
                    delete pGeoTemp;
                    printf("Couldn't read geo data from [%s]\n", pQDFFile);
                }
            } else {
                printf("Couldn't read geo data from [%s]\n", pQDFFile);
            }
            delete pGeoR;
        } else {
            printf("Couldn't create GeoGroupReader for QDF file [%s]\n", pQDFFile);
        }
        qdf_closeFile(hFile);
    } else {
        printf("Couldn't open [%s] as QDF file(hFile: %ld)\n", pQDFFile, hFile);
    }
  return pGeo;
}


//----------------------------------------------------------------------------
// createVegetation
//
Vegetation *createVegetation(const char *pQDFFile) {
    Vegetation *pVeg = NULL;
    int iResult = -1;
    hid_t hFile = qdf_openFile(pQDFFile);
    if (hFile > 0) {
        VegGroupReader *pVegR = VegGroupReader::createVegGroupReader(hFile);
        if (pVegR != NULL) {
            VegAttributes vega; 
            iResult = pVegR->readAttributes(&vega);
            if (iResult == 0) {
                Vegetation *pVegTemp = new Vegetation(vega.m_iNumCells, 3, NULL, NULL);
                
                iResult = pVegR->readData(pVegTemp);
                if (iResult == 0) {
                    pVeg = pVegTemp;
                } else {
                    delete pVegTemp;
                    printf("Couldn't read geo data from [%s]\n", pQDFFile);
                }
            } else {
                printf("Couldn't read geo data from [%s]\n", pQDFFile);
            }
            delete pVegR;
        } else {
            printf("Couldn't create VegGroupReader for QDF file [%s]\n", pQDFFile);
        }
        qdf_closeFile(hFile);
    } else {
        printf("Couldn't open [%s] as QDF file(hFile: %ld)\n", pQDFFile, hFile);
    }
  return pVeg;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 2) {

        SurfaceGrid *pSG = SurfaceGrid::createInstance(apArgV[1]);
        if (pSG != NULL) {
            Geography *pGeo = createGeography(apArgV[1]);
            if (pGeo != NULL) {
                Vegetation *pVeg = createVegetation(apArgV[1]);
                if (pVeg != NULL) {
                    SCellGrid * pCG = pSG->getCellGrid();
                    pCG->setGeography(pGeo);
                    pCG->setVegetation(pVeg);
                
                    neighlist mvNeigh;
                    ididxlist mIdIdx;
                    iResult = pruneNodes(pSG->getCellGrid(), mIdIdx);
                    if (iResult == 0) {
                        iResult = convertNeighbors(pSG->getCellGrid(), mIdIdx, mvNeigh);
                    }
                    if (iResult == 0) {
                        iResult = writeNeighborList(mvNeigh, apArgV[2]);
                    }
                    if (iResult == 0) {
                        iResult = writeIndexTable(mIdIdx, apArgV[2]);
                    }
                    if (iResult == 0) {
                        iResult = writeAttributeTable(pSG->getCellGrid(), mIdIdx, apArgV[2]);
                    }

                    //                    iResult = writeList(pSG->getCellGrid(), apArgV[2]);
                
                } else {
                    printf("Couldn't create vegetation\n");
                }
            } else {
                printf("Couldn't create geography\n");
            }
            delete pSG;
        } else {
            printf("Couldn't create SurfaceGrid\n");
        }
    } else {
        printf("%s - convert QDF grid to NEMO srtyle matrix definition\n", apArgV[0]);
        printf("Usage: %s <qdf-file> <output-file>\n", apArgV[0]);
    }
    return iResult;
}
