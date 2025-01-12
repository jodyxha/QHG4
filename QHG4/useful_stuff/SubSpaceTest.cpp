#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <locale.h>

#include "types.h"
#include "xha_strutils.h"
#include "xha_strutilsT.h"

#include "ParamReader.h"
#include "LineReader.h"
#include "SubSpace.h"
#include "SubSpace.cpp"
#include "SubSpaceIndexParser.h"

//----------------------------------------------------------------------------
// usage
//
void usage(std::string sApp) {
    xha_printf("usage:\n");
    xha_printf("   %s  (-e <extents> | -i <input-file>) -s <slice-indexes>\n", sApp);
    xha_printf("       [-o <output-file>] [-q] [-v]\n");
    xha_printf("where\n");
    xha_printf("  extents        extents of hypervolume. If set, SubSpace elements will be consecutive\n");
    xha_printf("  slice-indexes  operations to perform along the dimensions\n");
    xha_printf("  input-file     read SubSpace from file\n");
    xha_printf("  output-file    write SubSpace to file\n");
    xha_printf("  -q             squeeze entire hypervolume (remove dimensions with extent 1)\n");
    xha_printf("  -v             verbose\n");
    xha_printf("formats:\n");
    xha_printf("  dimensions     ::= <number> [\":\" <number>]\n");
    xha_printf("  slice-indexes  ::= <slice-index> [\":\"<slice-index>]\n");
    xha_printf("    slice-index  ::= <numeric> | <reduction>[\".\"]\n");
    xha_printf("    reduction    ::= \"*\" | \"#\" | \"%%\"\n");
    xha_printf("    numeric      ::= <number> [\"+\"] <number>}\n");
    xha_printf("If the format is numeric, slices will be extracted for the specified numbers\n"); 
    xha_printf("If the format is \"*\" this dimension is kept (shorthand for 0+1+2+..+n)\n"); 
    xha_printf("If the format is \"#\" the SubSpace will be reduced along this dimension\n"); 
    xha_printf("If the format is \"%%\" the SubSpace will be averaged along this dimension\n");
    xha_printf("If a reduction format is followed by a \".\", the corresponding dimension will be squeezed\n"); 
    xha_printf("requirements:\n");
    xha_printf("  * the number of slice indexes must be equal to the number of dimensions\n"); 
    xha_printf("  * the numbers used in the i-th <numeric> must be less than the size of the i-th dimension\n"); 
    xha_printf("  * the numbers used in a <numeric> should all differ from each other\n"); 
    xha_printf("Note: slices come before reductions (i.e. sums and averages)\n"); 

    xha_printf("Example:\n");
    xha_printf("  %s -e 4:2:3:5 -s 1+2:*:#.:4 -v\n", sApp);
}

//----------------------------------------------------------------------------
// show_uintvecvec
// 
static void show_uintvec(const uintvec vS, std::string &s){
    s = "";
    for (uint j = 0; j < vS.size(); j++) {
        if (j > 0) {
            s = s + ",";
        }
        s = s + xha_sprintf("%d", vS[j]);
    }
}

//----------------------------------------------------------------------------
// fill_buffer_recursively
//  recursively fills he buffer such that the element at coordinates 
//    (x_0, x_1,...x_(n-1)) 
//  has the value
//    sum_0^n-1 x_i*10^i
//
void fill_buffer_recursively(double **pCur, uintvec vSizes, uint iCurVal) {
    if (vSizes.size() > 0) {
        uint v = vSizes.back();
        vSizes.pop_back();
        for (uint i = 0; i < v; i++) {
            fill_buffer_recursively(pCur, vSizes, 10*iCurVal+i);
        }
    } else {
        **pCur = (double)(iCurVal);
        *pCur = *pCur + 1;
    }
    
}


//----------------------------------------------------------------------------
// createBuffer
//
double *createBuffer(uintvec vSizes, uint *piNumVals) {
    uint iNumVals = 1;
    // calculate number of values
    for (uint i = 0; i < vSizes.size(); ++i) {
        iNumVals *= vSizes[i];
    }

    std::string sVec;
    show_uintvec(vSizes, sVec);
    xha_fprintf(stderr, "[*createBuffer(uintvec vSizes, uint *piNumVals)] numvals %u, sizes [%s]\n", iNumVals, sVec);
    *piNumVals = iNumVals;
    
    // allocate buffer
    double *pOut = new double[iNumVals];
    double *pCur = pOut;
    // fill buffer 
    fill_buffer_recursively(&pCur, vSizes, 0);
    
    return pOut;

}


//----------------------------------------------------------------------------
// createBuffer
//
template<typename T>
double *createBuffer(SubSpace<T> *pSS) {
    uint iNumVals = pSS->getNumVals();
    uintvec vSizes = pSS->get_sizes_copy();

    std::string sVec;
    show_uintvec(vSizes, sVec);
    xha_fprintf(stderr, "[*createBuffer(SubSpace<T> *pSS)] numvals %u, sizes [%s]\n", iNumVals, sVec);

    // allocate buffer
    double *pOut = new double[iNumVals];
    double *pCur = pOut;
    // fill buffer 
    fill_buffer_recursively(&pCur, vSizes, 0);
    
    return pOut;

}


//----------------------------------------------------------------------------
// writeOutputFile
//
int writeOutputFile(const std::string sOutputFile, SubSpace<double> *pSS, std::string sSliceDesc) {
    int iResult = 0;
    FILE *fOut = fopen(sOutputFile.c_str(), "wt");
    if (fOut != NULL) {
        xha_fprintf(fOut, "# dimensions: ");
        const uintvec vSizes = pSS->get_sizes();
        for (uint i = 0; i < vSizes.size(); i++) {
            xha_fprintf(fOut, " %u", vSizes[i]);
        }
        xha_fprintf(fOut, "\n");
        // xha_fprintf(fOut, "# slices:      %s\n", sSliceDesc);
        // xha_fprintf(fOut, "\n");
        

        pSS->show_data_nice(DISP_FLOAT, fOut, vFileSeps, false);
        fprintf(fOut, "\n");
        fclose(fOut);
    } else {
        xha_fprintf(stderr, "Couldn't open [%s] for writing\n", sOutputFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// readInputFile
//
double *readInputFile(const std::string sInputFile, uintvec &vSizes) {
    double *pBuffer = NULL;

    LineReader *pLR = LineReader_std::createInstance(sInputFile, "rt");

    int iResult = 0;
    if (pLR != NULL) {
        // get the dimensions
        char *pLine = pLR->getNextLine(GNL_IGNORE_BLANKS | GNL_TRIM );
        if (strstr(pLine, "# dimensions:") != NULL) {
            char *pD = strchr(pLine, ':'); // this will work
            pD++;
            // convert dimensions to sizes
            iResult = SubSpace<double>::dims2Sizes(pD, vSizes, " ");

            // must reverse user dimension order to get SubSpace dimension order
            //std::reverse(vSizes.begin(), vSizes.end());

            if (iResult == 0) {
                uint iNumVals = 1;
                for (uint i = 0; i < vSizes.size(); i++) {
                    iNumVals *= vSizes[i];
                }

                // initialize SubSpace
                pBuffer = new double[iNumVals];
                // read values
                uint iCount = 0;
                double *pCur = pBuffer;
                pLine = pLR->getNextLine();
                while ((iResult == 0) && (!pLR->isEoF())) {
                    stringvec vVals;
                    uint iNum = splitString(pLine, vVals, ",; ");
                    for (uint i = 0; i < iNum; i++) {
                        double v;
                        if (strToNum(vVals[i], &v)) {
                            memcpy(pCur, &v, sizeof(double));
                            pCur++;
                            iCount++;
                        } else {
                            iResult = -1;
                            xha_fprintf(stderr, "Couldn't convert [%s] to number\n");
                        }
                    }
                    pLine = pLR->getNextLine();
                }
                if ((iResult == 0) && (iCount != iNumVals)) {
                    xha_fprintf(stderr, "Only found %u values instead of %u\n", iCount, iNumVals);
                    iResult = -1;
                }
            } else {
                xha_fprintf(stderr, "Couldn't convert ·dimensions to sizes\n");
            }
        } else {
            xha_fprintf(stderr, "Expected \"dimensions: <d> <d>...\" as first line\n");
        }
        delete pLR;
    } else {
        xha_fprintf(stderr, "Couldn't open [%s] for reading\n", sInputFile);
    }


    return pBuffer;
}
                     

//----------------------------------------------------------------------------
// parseIndexElements
//  parse the index elements to get slice and reduction information
//
int parseIndexElements(uintvec vSizes, std::string sIndexDesc, uintvecvec &vAllIndexes, uintuintmap &mAllReds, bool bVerbose) {
    int iResult = -1;
    //setlocale(LC_ALL, "");
    vAllIndexes.clear();
    mAllReds.clear();

    stringvec vsA;
    //uintvec   vA;
    uint iNumDims = vSizes.size();
    uint iDims2 = splitString(sIndexDesc, vsA, ":");

    if (iDims2 == iNumDims) {
        iResult = 0;
        // each part has the form "<int>[+<int>]" or "#" or "*" or "%"
        for (uint i = 0; (iResult == 0) && (i < iNumDims); i++) {
                        
            stringvec vsX;
            uintvec   vX;
            if ((vsA[i] == "*")  || (vsA[i] == "#") || (vsA[i] == "%")) {
                if (bVerbose) {xha_printf("found '%s' for %d: range(%d)\n", vsA[i], i, vSizes[i]);}
                for (uint k = 0; k < vSizes[i]; ++k) {
                    vX.push_back(k);
                }
                if (bVerbose) {xha_printf("vX (%zd): %bv\n", vX.size(), vX);}
                // if (bVerbose) {xha_printf("vX (%zd):", vX.size());for (uint z = 0; z < vX.size(); z++){ xha_printf(" %d", vX[z]);}printf("\n");}
                if (vsA[i] == "#") {
                    mAllReds[i] = RED_TYPE_SUM;
                } else if (vsA[i] == "%") {
                    mAllReds[i] = RED_TYPE_AVG;
                }
            } else {
                uint iNumSummands = splitString(vsA[i], vsX, "+");
                if (bVerbose) {xha_printf("vsX (%zd): %bv\n", vsX.size(), vsX);}
                // if (bVerbose) {xha_printf("vsX (%zd):", vsX.size());for (uint z = 0; z < vsX.size(); z++){ xha_printf(" %s", vsX[z]);}printf("\n");}
                for (uint i = 0; (iResult == 0) && (i < iNumSummands); i++) {
                    int k = 0;
                    if (strToNum(vsX[i], &k)) {
                        vX.push_back(k);
                    } else {
                        xha_fprintf(stderr, "invalid index configuration [%s]\n", vsX[i]);
                        iResult = -1;
                    }
                }

            }
            vAllIndexes.push_back(vX);

        }
    } else {
        xha_fprintf(stderr, "there should be %u index combinations but only found %u in [%s]\n", iNumDims, iDims2, sIndexDesc); 
    }
    return iResult;
}


//----------------------------------------------------------------------------
// showSliceIndexes
//
void showSliceIndexes(const uintvecvec  &vNeededDims, const uintuintmap &mReductionDims, const uintuintmap &mSqueezeDims) {
    stringvec sRed{"*", "+", "%"};

    xha_fprintf(stderr, "SliceIndexes:\n");
    for (uint i = 0; i < vNeededDims.size(); i++) {

        uintuintmap::const_iterator it = mReductionDims.find(i);
        std::string s = (it != mReductionDims.end())?sRed[it->second]:" ";
        
        it = mSqueezeDims.find(i);
        s += (it != mSqueezeDims.end())?"!":" ";

        xha_fprintf(stderr, ":  %s ", s);
        for (uint j = 0; j < vNeededDims[i].size(); j++) {
            xha_fprintf(stderr, "%d", vNeededDims[i][j]);
        }
        xha_fprintf(stderr, "\n");
    }
}


//----------------------------------------------------------------------------
// createSubSpaceFromExtents
//
SubSpace<double> *createSubSpaceFromExtents(std::string sExtents, bool bVerbose) {
  
    double *pData = NULL;   
    SubSpace<double> *pSS = SubSpace<double>::create_instance_from_exts(sExtents, bVerbose);
    if (pSS != NULL) {
        try {
            pData = createBuffer(pSS);
            pSS->set_data(pData, 0, pSS->getNumVals());
            delete[] pData;
        } catch (const SubSpaceException &sse) {
            throw SubSpaceException(xha_sprintf("Exception in [createSubSpaceFromExtents]: %s\n", sse.what()));
        }
        
    } else {
        xha_fprintf(stderr, "failed to create SubSpace from dimension string [%s]\n");
    }
    return pSS;
}
    



//----------------------------------------------------------------------------
// createSubSpaceFromFile
//
SubSpace<double> *createSubSpaceFromFile(std::string sInputFile, bool bVerbose) {
    SubSpace<double> *pSS = NULL;
    double *pData = NULL;   
    uintvec vSizes;
    pData = readInputFile(sInputFile, vSizes);
    if (pData != NULL) {
        
        pSS = SubSpace<double>::create_instance_from_sizes(vSizes, bVerbose); 
        if (pSS != NULL) {
            try {
                pSS->set_data(pData, 0, pSS->getNumVals());
                xha_fprintf(stderr, "read file\n");
            } catch (const SubSpaceException &sse) {
                throw SubSpaceException(xha_sprintf("Exception in [createSubSpaceFromFile]: %s\n", sse.what()));
            }
        }
        
        delete[] pData;
    }
    return pSS;
}


//----------------------------------------------------------------------------
// makeSlices
//
SubSpace<double> *makeSlices(SubSpace<double> *pSS, const uintvecvec  &vNeededDims, bool bVerbose) {
    SubSpace<double> *pSSResult;

    if (vNeededDims.size() > 0) {
        SubSpace<double> *pOldSS = pSS;
        try {
            pSSResult = pSS->create_slice(vNeededDims);
        } catch (const SubSpaceException &sse) {
            throw SubSpaceException(xha_sprintf("Exception in [makeSlices]: %s\n", sse.what()));
        }
        delete pOldSS;   
        
        if (pSSResult != NULL) {
            xha_fprintf(stderr, "Have slice\n");
            
            pSSResult->show_sizes();
            if (bVerbose) {
                pSSResult->show_data_nice(DISP_FLOAT, stdout, vStandardSeps, true);
            }
        }    
    } else {
        pSSResult = pSS;
    }
    return pSSResult;
}

//----------------------------------------------------------------------------
// makeReductions
//
SubSpace<double> *makeReductions(SubSpace<double> *pSS, const uintuintmap &mReductionDims, bool bVerbose) {
 
    SubSpace<double> *pSSResult = NULL;
    if (mReductionDims.size() > 0) {
        SubSpace<double> *pOldSS = pSS;
        try {
            pSSResult = pOldSS->create_reductions(mReductionDims);
        } catch (const SubSpaceException &sse) {
            throw SubSpaceException(xha_sprintf("Exception in [makeReductions]: %s\n", sse.what()));
        } 
        delete pOldSS;
        
        if (pSSResult != NULL) {
            xha_fprintf(stderr, "Have reductiona\n");
            
            pSSResult->show_sizes();
            if (bVerbose) {
                pSSResult->show_data_nice(DISP_FLOAT, stdout, vStandardSeps, true);
            }
        }
    } else {
        pSSResult = pSS;
    }
    return pSSResult;
}


//----------------------------------------------------------------------------
// makeSqueeze
//
SubSpace<double> *makeSqueeze(SubSpace<double> *pSS, const uintuintmap &mSqueezDims,  bool bVerbose) {
    SubSpace<double> *pSSResult;
    SubSpace<double> *pOldSS = pSS;
    try {
        pSSResult = pOldSS->squeeze(mSqueezDims);
    } catch (const SubSpaceException &sse) {
        throw SubSpaceException(xha_sprintf("Exception in [makeSqueeze]: %s\n", sse.what()));
    }
    delete pOldSS;
    
    if (pSSResult != NULL) {
        xha_fprintf(stderr, "Have squeeze\n");
        
        pSSResult->show_sizes();
        if (bVerbose) {
            pSSResult->show_data_nice(DISP_FLOAT, stdout, vStandardSeps, true);
        }
    }
    return pSSResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    

    std::string sExtents = "";
    std::string sSlices = "";
    std::string sInputFile = "";
    std::string sOutputFile = "";
    bool bVerbose   = false;
    bool bSqueeze = false;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,
                               "-e:s",      &sExtents,
                               "-s:s!",     &sSlices,
                               "-i:s",      &sInputFile,
                               "-o:s",      &sOutputFile,
                               "-v:0",      &bVerbose,
                               "-q:0",      &bSqueeze);
     


    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (!(sExtents.empty() && sInputFile.empty())) {

                SubSpace<double> *pSS = NULL;
            
                if (!sExtents.empty()) {
                    // create the SubSpace from the dimensions strin
                    pSS = createSubSpaceFromExtents(sExtents, bVerbose);

                } else if (!sInputFile.empty()) {
                pSS = createSubSpaceFromFile(sInputFile, bVerbose);
                
                } else {
                    // should not happen: either "-e" or "-i" must be specified
                    iResult = -1;
                }    

                // show inital state
                if (pSS != NULL) {
                    // we know that the buffer vSizes, iNumVals and iDim are correctly set
                    pSS->show_sizes();
                    //                if (bVerbose) {
                    pSS->show_data_nice(DISP_FLOAT, stdout, vStandardSeps, true);
                    //                }
                    
                }
       
                if (pSS != NULL) { 
                    SubSpace<double> *pSSResult = pSS;  
                    
                    SubSpaceIndexParser *pSSIP = SubSpaceIndexParser::create_instance(pSS->get_sizes(), bVerbose);
                    if (pSSIP != NULL) {
                        // we know that vSizes and iDim are correctly set
                        stringvec vSliceDescs;
                        uint iNumDesc = splitString(sSlices, vSliceDescs, ",");
                        try {
                            for (uint j = 0; j < iNumDesc; j++) {
                                std::string sCurSlices = vSliceDescs[j];
                                // xha_printf(">>>>>handling [%s]\n", sCurSlices);
                                
                                pSSIP->init(pSS->get_sizes());
                                
                                // <iResult = parseIndexElements(vSizes, sCurSlices, vAllIndexes, mRedDims,  bVerbose);
                                iResult = pSSIP->parseSliceDesc(sCurSlices, bSqueeze);
                                if (iResult == 0) {
                                    
                                    showSliceIndexes( pSSIP->get_needed_dims(), pSSIP->get_reduction_dims(), pSSIP->get_squeeze_dims());
                                    
                                    // Perform the operations
                                    //SubSpace<double> *pSSResult = NULL;  
                                    
                                    // slices
                                    pSSResult = makeSlices(pSSResult, pSSIP->get_needed_dims(), bVerbose);
                                    
                                    
                                    // reductions
                                    pSSResult = makeReductions(pSSResult, pSSIP->get_reduction_dims(), bVerbose);
                                    
                                    
                                    // squeezing
                                    pSSResult = makeSqueeze(pSSResult, pSSIP->get_squeeze_dims(),  bVerbose);
                                   
                                    
                                    
                                    pSSResult->show_data_nice(DISP_FLOAT, stdout, vStandardSeps, true);
                                    if (!sOutputFile.empty()) {
                                        iResult = writeOutputFile(sOutputFile, pSSResult, sSlices);
                                    }
                                    // delete pSSResult;
                                    pSS = pSSResult;
                                } else {
                                    // error in parseIndex
                                }
                                
                            } // descr loop
                        } catch (const SubSpaceException &sse) {
                            xha_fprintf(stderr, sse.what());
                        }
                        delete pSSIP;
                    }
                    delete pSSResult;
                    
                } else {
                    // pSS is NULL
                }    
            }  else {
                xha_fprintf(stderr, "Either dimensions or in0put file must be specified\n");
            }
        } else {
            usage(apArgV[0]);
        }
    } else {
        xha_fprintf(stderr, "Couldn't create ParamReader\n");
    }


    delete pPR;
    return iResult;
}
