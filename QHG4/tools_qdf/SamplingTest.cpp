#include <cstdio>

#include "Sampling.h"
#include "SamplingT.cpp"
#include "CellSampling.h"
#include "FullSampling.h"
#include "RangeSampling.h"

class SamplingTest : public Sampling {
public:

    SamplingTest() {
	m_mGroups[0] = {2,4,6,8};
	m_mGroups[1] = {3,5,7,9,11};
	m_mGroups[3] = {1,10,12};
	
	makeRefs();
    }

};


void showGroups(groupedvals<float> &vG) {
    groupedvals<float>::const_iterator it;
    for (it = vG.begin(); it != vG.end(); ++it) {
        printf("%03d: ", it->first);
	for (unsigned int k = 0; k < it->second.size(); ++k) {
             printf(" %f", it->second[k]);
	}
	printf("\n");
    }
}


int main(int iArgC, char *apArgV[]) {

    int iResult = 0;

    int iW = 5;
    int iH = 5;
    int iNumCells = iW*iH;
    
    indexedvals<float> vVals;
    for (int i =0; i < iNumCells; ++i) {
        vVals.push_back(std::pair<int, float>(i, i*i));
    }
    pointrad pr {{0,2}, {4,2}, {20,2}, {24,1}};
    double *dLon = new double[iNumCells];
    double *dLat = new double[iNumCells];
    for (int h = 0; h < iH; h++) {
        for (int w = 0; w < iW; w++) {
	   dLat[h*iW + w] = h;
	   dLon[h*iW + w] = w;
        }
    }

    groupedvals<float> vG;
    SamplingTest st;
    printf("SamplingTest Samples:\n");
    st.showSamples();
    printf("--------------\n");
    vG.clear();
    iResult = st.groupValues(vVals, vG);
    showGroups(vG);
    printf("=========================\n");

    CellSampling cs(iNumCells);
    printf("CellSampling Samples:\n");
    cs.showSamples();
    printf("--------------\n");
    vG.clear();
    iResult = cs.groupValues(vVals, vG);
    showGroups(vG);
    printf("=========================\n");

    FullSampling fs(iNumCells);
    printf("FullSampling Samples:\n");
    fs.showSamples();
    printf("--------------\n");
    vG.clear();
    iResult = fs.groupValues(vVals, vG);
    showGroups(vG);
    printf("=========================\n");

    RangeSampling ds(iNumCells, pr, dLon, dLat, 1.0, false);
    printf("RangeSampling Samples:\n");
    ds.showSamples();
    printf("--------------\n");
    vG.clear();
    iResult = ds.groupValues(vVals, vG);
    showGroups(vG);
    printf("=========================\n");

    
    delete[] dLon;
    delete[] dLat;
    
    
    return iResult;
}
