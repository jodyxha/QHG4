#include <cstdio>

#include "types.h"
#include "EQsahedron.h"
#include "EQVertexFaces.h"


//-----------------------------------------------------------------------------
// constructor
//
EQVertexFaces::EQVertexFaces(int iNumSubDivs) 
    : m_iNumSubDivs(iNumSubDivs),
      m_pEQ(EQsahedron::createInstance(m_iNumSubDivs, false, NULL)) {
}



//-----------------------------------------------------------------------------
// destructor
//
EQVertexFaces::~EQVertexFaces() {
    delete m_pEQ;
} 


//-----------------------------------------------------------------------------
// getNeighborIDs
//
int EQVertexFaces::getFaceNeighborIDs(int iNodeID, intset &sv) {
    int iResult = -1;
    if ((iNodeID >= 0) && (iNodeID < EQsahedron::calcNumVerts(m_iNumSubDivs))) {
        if (iNodeID < ICOVERTS) {
            printf("Pure node\n");
            getNodeVertices(iNodeID, sv);
        } else if (iNodeID < ICOVERTS+m_iNumSubDivs*ICOEDGES) {
            printf("Pure edge\n");
            int iEdgeID =  getNodeEdgeID(iNodeID);
            getEdgeVertices(iEdgeID, sv);
        } else {
            printf("Pure face\n");
            int iFaceID = getNodeFaceID(iNodeID);
            getFaceVertices(iFaceID, sv);
        }
        iResult = sv.size();
    } else {
        sv.clear();
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getNeighborIDs
//
int EQVertexFaces::getFaceNeighborIDs(double dLon, double dLat, intset &sv) {
    int iNode = m_pEQ->findNode(dLon, dLat);
    printf("Have node %d\n", iNode);
    return getFaceNeighborIDs(iNode, sv);
}

//-----------------------------------------------------------------------------
// getNodeEdgeID
//   we assume the node is an edge node
//
int EQVertexFaces::getNodeEdgeID(int iNodeID) {
    int e0 = (iNodeID-ICOVERTS);
    return  (e0 - (e0%m_iNumSubDivs)) / m_iNumSubDivs;
}

//-----------------------------------------------------------------------------
// getNodeFaceID
//
int EQVertexFaces::getNodeFaceID(int iNodeID) {
    int f0 = iNodeID -ICOEDGES*m_iNumSubDivs;
    int k0 = f0%(m_iNumSubDivs*(m_iNumSubDivs-1)/2);
    return (f0 - k0)/(m_iNumSubDivs*(m_iNumSubDivs-1)/2);
}


//-----------------------------------------------------------------------------
// getNodeVertices
//   we assume iNode is a pure vertex (i.e. part of 5 faces)
//
void EQVertexFaces::getNodeVertices(int iNode, intset &sv) {
    for (int i = 0; i < 5; i++) {
        getFaceVertices(EQsahedron::s_V2F[iNode][i], sv);
    }
}

//-----------------------------------------------------------------------------
// getEdgeVertices
//   we assume iNode is a pure edge vertex (i.e. part of 2 faces)
//
void EQVertexFaces::getEdgeVertices(int iEdgeID, intset &sv) {
    getFaceVertices(EQsahedron::s_E2F[iEdgeID][0], sv);
    getFaceVertices(EQsahedron::s_E2F[iEdgeID][1], sv);
}

//-----------------------------------------------------------------------------
// getFaceVertices
//   we assume iNode is a pure face vertex (i.e. part of 1 face)
//
void EQVertexFaces::getFaceVertices(int iFaceID, intset &sv) {
    // vetices at the corners
    for (int i = 0; i < VERTS_PER_FACE; i++) {
        sv.insert(EQsahedron::s_aMainFaces[iFaceID][i]);
    }

    // pure edge verts
    for (int i = 0; i < EDGES_PER_FACE; i++) {
        int iOffset = ICOVERTS +m_iNumSubDivs*EQsahedron::s_F2E[iFaceID][i];
        for (int j = 0; j < m_iNumSubDivs; j++) {
            sv.insert(iOffset+j);
        }
    }


    int iOffset = ICOVERTS + m_iNumSubDivs*ICOEDGES +
        iFaceID*m_iNumSubDivs*(m_iNumSubDivs-1)/2;
    printf("offset[%d]: %d\n", iFaceID, iOffset);
    for (int j = 0; j < m_iNumSubDivs*(m_iNumSubDivs-1)/2; j++) {
        sv.insert(iOffset+j);
    }
    
}
