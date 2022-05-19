#ifndef __EQVERTEXFACES_H__
#define __EQVERTEXFACES_H__

#include "types.h"
#include "EQsahedron.h"

class EQVertexFaces {
public:
    //static EQVertexFaces *createInstance(int iNumSubDivs);
    EQVertexFaces(int iNumSubDivs);
    virtual ~EQVertexFaces();

    int getFaceNeighborIDs(int iNodeID, intset &sv);
    int getFaceNeighborIDs(double dLon, double dLat, intset &sv);


protected:

    int init();
    int getNodeVertID(int iNodeID) {return iNodeID;};;
    int getNodeEdgeID(int iNodeID);
    int getNodeFaceID(int iNodeID);
    
    void getNodeVertices(int iNodeID, intset &sv);
    void getEdgeVertices(int iEdgeID, intset &sv);
    void getFaceVertices(int iFaceID, intset &sv);

    int m_iNumSubDivs;
    EQsahedron *m_pEQ;
};


#endif
