#include <time.h>
#include <string.h>
#include "types.h"

#include "xha_strutils.h"
#include "xha_strutilsT.h"
#include "WELL512.h"


#include "SurfaceGrid.h"
#include "SCellGrid.h"
#include "PopLooper.h"
#include "GroupPop.h"
#include "ChildManager.h"
#include "ChildManager.cpp"


static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};



int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    if (iArgC > 1)  {
   
        uint32_t  m_aulState[STATE_SIZE];
        uint      m_aiSeeds[8];


        memcpy(m_aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
        memset(m_aiSeeds, 0, NUM_SEEDS*sizeof(uint));

        SurfaceGrid *pSG = SurfaceGrid::createInstance(apArgV[1]);
        SCellGrid *pCG = pSG->getCellGrid();
   
        PopLooper *pPopLooper = new PopLooper();
        IDGen** apIDG = new IDGen*[1];
        GroupPop *pGP = new GroupPop(pCG, pPopLooper, 32768, apIDG, m_aulState, m_aiSeeds);

        ChildManager *pCM = new ChildManager(pGP, 12);

        // do something
        delete pCM;
        delete apIDG;
        delete pPopLooper;
        delete pCG;
        delete pSG;

            
    } else {
        iResult = -1;
    }
    return iResult;
}
