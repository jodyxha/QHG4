#include "ParamReader.h"
#include "LineReader.h"
#include "types.h"
#include "strutils.h"
#include "DistMat.h"
#include "BinGeneFile.h"
#include "SequenceDist.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"

#include "DistMat.cpp"
#include "SequenceDist.cpp"


//----------------------------------------------------------------------------
// usage
//   
void usage(char *pApp) {
    printf("%s - count alleles in a sample\n", pApp);
    printf("Usage;\n");
    printf("  %s -g <bingenomefile>\n", pApp);
    printf("where\n");
    printf("  bingenomefile   binary genome sample file\n");
    printf("\n");
}

//----------------------------------------------------------------------------
// readGenomes2
//   try to read given file a s binary
//
BinGeneFile *readGenomes2(const char *pGeneFile) {
    int iNumGenomes = -1;
    BinGeneFile *pBG = BinGeneFile::createInstance(pGeneFile);
    if (pBG != NULL) {
        iNumGenomes = pBG->read();
        if (iNumGenomes <= 0) {
            delete pBG;
            pBG = NULL;
        }
    }
    return pBG;
}   

//----------------------------------------------------------------------------
// countAlleleTypes
//
void countAlleleTypes(ulong *al1, ulong *al2, int iSize, uint *paz1, uint *pat1, uint *pao1) {
    int iNumThreads = omp_get_num_threads();
    //    printf("number of threads: %d\n", iNumThreads);
    
    memset(paz1, 0, iNumThreads*sizeof(uint));
    memset(pat1, 0, iNumThreads*sizeof(uint));
    memset(pao1, 0, iNumThreads*sizeof(uint));

    //#pragma omp parallel for
    for (int j = 0; j < iSize; j++) {
        int iThreadNum = omp_get_thread_num();
        //printf("looking at %016lx and %016lx\n", al1[j], al2[j]); 
        ulong lOR  = al1[j] | al2[j];
        ulong lAND = al1[j] & al2[j];
        ulong lXOR = al1[j] ^ al2[j];
        paz1[iThreadNum] += 64-BitGeneUtils::bitcount(lOR);
        pat1[iThreadNum] += BitGeneUtils::bitcount(lAND);
        pao1[iThreadNum] += BitGeneUtils::bitcount(lXOR);
        //printf("paz1[%d]: %u\n", iThreadNum, paz1[iThreadNum]); 
    }
   
}


//----------------------------------------------------------------------------
// main
//   
int main(int iArgC, char *apArgV[]) {
    int iResult =-1;
    char *pGeneFile     = NULL;


    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        bool bOK = pPR->setOptions(1,
                                   "-g:S!",   &pGeneFile);
        
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                std::map<idtype,int> mIDNodes;
                tnamed_ids mvIDs;
                id_locs mIdLocs;

                id_genomes mIDGen;

                BinGeneFile *pBG = readGenomes2(pGeneFile);
                if (pBG != NULL) {
                    int iNumGenes   = pBG->getNumGenomes();
                    int iGenomeSize = pBG->getGenomeSize();
                    int iNumBlocks  = BitGeneUtils::numNucs2Blocks(iGenomeSize);
                    mvIDs    = pBG->getvIDs();
                    mIDGen   = pBG->getIDGen();
                    mIDNodes = pBG->getIDNodes();
                    mIdLocs  = pBG->getIDLocs();
                    int iBitsPerNuc = pBG->getBitsPerNuc();
                    printf("Bits per Nuc: %d\n", iBitsPerNuc);

                    int iTotal00 = 0;
                    int iTotal01 = 0;
                    int iTotal11 = 0;

                    int iNumThreads = omp_get_num_threads();
                    uint *paz1 = new uint[iNumThreads];
                    uint *pat1 = new uint[iNumThreads];
                    uint *pao1 = new uint[iNumThreads];

                    tnamed_ids::const_iterator it;
                    for (it = mvIDs.begin(); it != mvIDs.end(); ++it) {
                        for (uint k = 0; k < it->second.size(); k++) {

                            ulong *pSeq1 = mIDGen[it->second[k]];                            
                            ulong *pSeq2 = mIDGen[it->second[k]]+iNumBlocks;


                            countAlleleTypes(pSeq1, pSeq2, iNumBlocks, paz1, pat1, pao1);
                            
                            for (int i = 0; i < iNumThreads; i++) {
                                iTotal00 += paz1[i];
                                iTotal11 += pat1[i];
                                iTotal01 += pao1[i];
                            }
                        }
                    }
                    delete[] paz1;
                    delete[] pat1;
                    delete[] pao1;
                    printf("genome size:         %d\n", iGenomeSize);
                    printf("number of genomes:   %d\n", iNumGenes);
                    printf("number of wildtypes: %d (%f = %f%%)\n", iTotal00, (1.0*iTotal00)/iNumGenes, (100.0*iTotal00)/(iNumGenes*iGenomeSize));
                    printf("number of fixated:   %d (%f = %f%%)\n", iTotal11, (1.0*iTotal11)/iNumGenes, (100.0*iTotal11)/(iNumGenes*iGenomeSize));
                    printf("number of heterozyg: %d (%f = %f%%)\n", iTotal01, (1.0*iTotal01)/iNumGenes, (100.0*iTotal01)/(iNumGenes*iGenomeSize));

                    delete pBG;
                } else {
                    // nothing to do
                    fprintf(stderr, "Problem reading the genome file [%s]\n", pGeneFile);
                }
     
            } else {
                usage(apArgV[0]);
            }
        } else {
            fprintf(stderr, "Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        fprintf(stderr, "Couldn't create ParamReader\n");
    }
    
    if (iResult == 0) {
        printf("+++ success +++\n");
    }

    return iResult;
} 
