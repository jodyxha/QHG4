#include <cstdio>
#include <cstring>

#include "types.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "BinGeneFile.h"
#include "BinPheneFile.h"


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - Converting Creating bin sample files to numeric\n", pApp);
    printf("Usage:\n");
    printf("%s -i <InBinFile> [-o <OutNumFile>]\n", pApp);
    printf("where\n");
    printf("  InBinFile   binary sample file (as created by QDFSampler\n");
    printf("  OutNumFile  numeric samplefile (default: same name as input; with suffix 'dat')\n");
    printf("              output format: \n");
    printf("                output ::= <line>*\n");
    printf("                line   ::= <AgentID> <nucleotides>*\n");
    printf("                nucleotides ::= <nuc2bit> | <nuc1bit>\n");
    printf("                nuc1bit ::= \"0\" | \"1\"\n");
    printf("                nuc2bit ::= \"0\" | \"1\" | \"2\" | \"3\"\n");
    printf("%s checks the header of InBinFile to determine the type (genes, or phenes)\n", pApp); 
    printf("\n");
}


//----------------------------------------------------------------------------
// convertGen
//
int convertGen(const char *pInBinFile, const char *pOutNumFile) {
    int iResult = -1;
    BinGeneFile *pBGF = BinGeneFile::createInstance(pInBinFile);
    if (pBGF != NULL) {
        int iNumGenomes = pBGF->read();
                    
        if (iNumGenomes > 0) {
            const id_genomes &mIdGen = pBGF->getIDGen();
            int iGenomeSize   = pBGF->getGenomeSize();
            int iNumBlocks    = 0;
            int iNucsInBlock  = 0;
            char *(*blockToNumStr)(ulong lBlock, char *pNuc);
                        
            int iBitsPerNuc = pBGF->getBitsPerNuc();
            const tnamed_ids mvIDs =  pBGF->getvIDs();
            if (iBitsPerNuc == 1) {
                iNumBlocks    =  BitGeneUtils::numNucs2Blocks((int)iGenomeSize);
                iNucsInBlock  =  BitGeneUtils::NUCSINBLOCK;
                blockToNumStr = &BitGeneUtils::blockToNumStr;
            } else {
                iNumBlocks    =  GeneUtils::numNucs2Blocks((int)iGenomeSize);
                iNucsInBlock  =  GeneUtils::NUCSINBLOCK;
                blockToNumStr = &GeneUtils::blockToNumStr;
            }
            printf("read %d genomes with genome size %d\n", iNumGenomes, iGenomeSize);
            // 
            // open output file
            FILE *fOut = fopen(pOutNumFile, "wt");
            if (fOut != NULL) {
                tnamed_ids::const_iterator it;
                for (it= mvIDs.begin(); it != mvIDs.end(); ++it) {
                    for (uint i = 0; i < it->second.size(); ++i) {
                        fprintf(fOut, "%12ld ", it->second[i]);
                        id_genomes::const_iterator iti = mIdGen.find(it->second[i]);
                        const ulong *p = iti->second;
                        int iNucCount = iGenomeSize;
                        for (int iB = 0; iB < 2*iNumBlocks; iB++) {
                            char s[2*iNucsInBlock+1];
                            (*blockToNumStr)(*p, s);
                            iNucCount -= iNucsInBlock;
                            if ((iNucCount <  iNucsInBlock) && (iNucCount > 0)) {
                                s[2*iNucCount] = '\0';
                            }
                            if (iB == iNumBlocks) {
                                fprintf(fOut, " ");
                                iNucCount = iGenomeSize;
                            }
                            fprintf(fOut, "%s", s);
                            p++;
                        }
                        fprintf(fOut, "\n");
                    }
                }
                iResult = 0;
                fclose(fOut);
            } else {
                printf("Couldn't open [%s] for writing\n", pOutNumFile);
            }
        } else {
            fprintf(stderr,"Couldn't read from [%s]\n", pInBinFile);
        }
        delete pBGF;
    } else {
        fprintf(stderr,"Couldn't create BinGeneFile\n");
    }
    return iResult;     
}                


//----------------------------------------------------------------------------
// convertPhen
//
int convertPhen(const char *pInBinFile, const char *pOutNumFile) {
    int iResult = -1;
    BinPheneFile *pBPF = BinPheneFile::createInstance(pInBinFile);
    if (pBPF != NULL) {
        int iNumPhenomes = pBPF->read();
        if (iNumPhenomes > 0) {
            // open output file
            FILE *fOut = fopen(pOutNumFile, "wt");
            if (fOut != NULL) {
                int iPhenomeSize   = pBPF->getPhenomeSize();
                int iPloidy      = pBPF->getPloidy();
                id_phenomes mIDPhen;
                tnamed_ids  mvIDs;
                int iLine = 1;
                pBPF->getMapsMain(mIDPhen, mvIDs);
                tnamed_ids::const_iterator it;
                for (it= mvIDs.begin(); it != mvIDs.end(); ++it) {
                    for (uint i = 0; i < it->second.size(); ++i) {
                        id_phenomes::const_iterator iti = mIDPhen.find(it->second[i]);
                        const float *p = iti->second;

                        for (int iP = 0; iP < iPloidy*iPhenomeSize; iP++) {
                            if (std::isnan(p[iP]) or std::isinf(p[iP])) {
                                printf("**** ID %ld (%d) : element %d is %s\n", iti->first, iLine, iP, std::isnan(p[iP])?"nan":"inf");
                            }
                            
                            fprintf(fOut, "%+.6e ", p[iP]);
                        }
                        fprintf(fOut, "\n");
                        iLine++;
                    }
                }
                iResult = 0;
                fclose(fOut);
            } else {
                printf("Couldn't open [%s] for writing\n", pOutNumFile);
            }

        } else {
            fprintf(stderr,"Couldn't read from [%s]\n", pInBinFile);
        }

        delete pBPF;
    } else {
        fprintf(stderr,"Couldn't create BinPheneFile\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int   iResult     = 0;
    char *sInBinFile  = NULL;
    char *sOutNumFile = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(2,
                                   "-i:S!",  &sInBinFile,
                                   "-o:S",   &sOutNumFile);

        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                // make sure we hav an output name
                char *sActualOut = NULL;
                if (sOutNumFile != NULL) {
                    sActualOut = new char[strlen(sOutNumFile)+1];
                    strcpy(sActualOut, sOutNumFile);
                } else {
                    // create new output name based on input file name
                    sActualOut = new char[strlen(sInBinFile)+4+1];
                    strcpy(sActualOut, sInBinFile);

                    char *p = strstr(sActualOut, ".bin");
                    if (p != NULL) {
                        *p = '\0';
                    }
                    strcat(sActualOut, ".asc");
                }

                // get the magich header from the file
                FILE *fH = fopen(sInBinFile, "rb");
                if (fH != NULL) {
                    char sMagic[5];
                    int iRead = fread(sMagic, 4, 1, fH);
                    sMagic[4] = '\0';
                    fclose(fH);
                    
                    if (iRead > 0) {
                        if (strncmp(sMagic, "GEN", 3) == 0) {
                            iResult = convertGen(sInBinFile, sActualOut);
                            
                        } else if (strncmp(sMagic, "PHN", 3) == 0) {
                            iResult = convertPhen(sInBinFile, sActualOut);
                            
                        } else {
                            fprintf(stderr, "Unknown magic number: [%s]\n", sMagic);
                        }
                    } else {
                        fprintf(stderr, "Couldn't read magic number from [%s]\n", sInBinFile);
                    }
                } else {
                    fprintf(stderr, "Couldn't open [%s]\n", sInBinFile);
                }
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr,"Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    return iResult;
}
