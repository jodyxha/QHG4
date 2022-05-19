#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "BitGeneUtils.h"
#include "BinGeneFile.h"

#define S_NONE   0
#define S_FIRST  1
#define S_SECOND 2
#define S_BOTH   3

void showGenome(id_genomes::const_iterator it, int iNumBlocks, bool bID, int iAlleles, int iShowBlocks) {
    char s[128];

    if (bID) {
        printf("%ld\n", it->first);
    }

    if ((iAlleles & S_FIRST) != 0) {
        for (int i = 0; i < iShowBlocks; i++) {	
            BitGeneUtils::blockToNucStr(it->second[i], s);
            printf("  1: %s\n", s);
        }
    }

    if ((iAlleles & S_SECOND) != 0) {
        for (int i = 0; i < iShowBlocks; i++) {	
            BitGeneUtils::blockToNucStr(it->second[i+iNumBlocks], s);
            printf("  2: %s\n", s);
        }
    }
	
}

int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    bool bHeader     = true;
    int  iAlleles    = S_BOTH;
    bool bShowID     = true;
    int  iShowBlocks = 1;
    if  (iArgC > 1) {  
        if (iArgC > 2) {
            int i = 2;
            while ((iResult == 0) && ((i+1) < iArgC)) {
                if (strcmp(apArgV[i], "-h") == 0) {
                    bHeader = (0 != atoi(apArgV[i+1]));
                } else if (strcmp(apArgV[i], "-i") == 0) {
                    bShowID = (0 != atoi(apArgV[i+1]));
                } else if (strcmp(apArgV[i], "-a") == 0) {
                    iAlleles = atoi(apArgV[i+1]);
                    if (iAlleles == 0) {
                        iResult = -1;
                    }	
                } else if (strcmp(apArgV[i], "-n") == 0) {    
                    iShowBlocks = atoi(apArgV[i+1]);
                }
		i += 2;
            } 
        }
   // TODO usage tec. 
        if (iResult == 0) { 
            BinGeneFile *pBGF = BinGeneFile::createInstance(apArgV[1]);
            if (bHeader) {
                pBGF->showHeader();
            }
            pBGF->read();

            int iNumBlocks = BitGeneUtils::numNucs2Blocks(pBGF->getGenomeSize());
            int iPloidy = 2;
     
            int iFirstBlock = 0;
            int iShiftBlocks = 1;
            bool bDoShift = false;


            const id_genomes ig = pBGF->getIDGen();
            printf("have %u genomes\n", ig.size());
            id_genomes::const_iterator it;
            for (it = ig.begin(); it != ig.end(); ++it) {

                showGenome(it, iNumBlocks, bShowID, iAlleles, iShowBlocks);
            }

            printf("-----\n");

	
            if (bDoShift) {
                char s[256];
                ulong *pSeq = it->second;
 
                memmove(pSeq, pSeq+iFirstBlock, iShiftBlocks*sizeof(ulong));
                if (iPloidy > 1) {
                     ulong *p = pSeq + iShiftBlocks;
                     memmove(p, pSeq+(iNumBlocks+iFirstBlock), iShiftBlocks*sizeof(ulong));
                }
                BitGeneUtils::blockToNucStr(pSeq[0], s);

                printf("  %s\n", s);

                BitGeneUtils::blockToNucStr(pSeq[iShiftBlocks], s);
                printf("  %s\n", s);
                printf("=====\n");
            }

            delete pBGF;
        }
    } 
    return iResult;
}


