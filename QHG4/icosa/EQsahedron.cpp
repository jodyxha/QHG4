#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>

#include <omp.h>

#include "qhg_consts.h"
#include "strutils.h"
#include "xha_strutilsT.h"
#include "Vec3D.h"
#include "Quat.h"

//#include "ValReader.h"

#include "IcoFace.h"
#include "VertexLinkage.h"

#include "icoutil.h"

#include "Tegmark.h"
#include "EQTriangle.h"
#include "EQsahedron.h"

#define KEY_MAGIC "IEQ3"
#define KEY_LEVEL "LEVEL:"
/*#define KEY_ROT   "ROTATION:"*/
#define KEY_EQUAL "EQUAL:"
#define KEY_LAND  "LAND:"
#define KEY_RECT  "RECT:"

/******** ico layout: ids for base vertices, edges, and faces

       0            0            0            0            0    
      / \          / \          / \          / \          / \
    0/ 0 \1      1/ 1 \2      2/ 2 \3      3/ 3 \4      4/ 4 \0 
    /_____\      /_____\      /_____\      /_____\      /_____\
   1   5   2    2   6   3    3   7   4    4   8   5    5   9   1

   1___5___2    2___6___3    3___7___4    4___8___5    5___9___1
    \     /      \     /      \     /      \     /      \     /
   10\ 5 /11    12\ 7 /13    14\ 9 /15    16\11 /17    18\13 /19
      \ /          \ /          \ /          \ /          \ /
       6            7            8            9           10    
             2            3            4            5            1    
            / \          / \          / \          / \          / \
         11/ 6 \12    13/ 8 \14    15/10 \16    17/12 \18    19/14 \10 
          /_____\      /_____\      /_____\      /_____\      /_____\
         6  20   7    7  21   8    8  22   9    9  23   10  10  24   6

         6__20___7    7__21___8    8__22___9    9__23___10  10__24___6
          \     /      \     /      \     /      \     /      \     /
         25\15 /26    26\16 /27    27\17 /28    28\18 /29    29\19 /25
            \ /          \ /          \ /          \ /          \ /
            11           11           11           11           11    


vertex ids for subdivided ico (N subdivisions):

id of main k
  id = k;  (0 <= k < 12)

k-th (internal) vertex on edge i:    
  id = 12 + i*N + k;  (0 <= k < N)
(numbered from lower-id main vertex towards higher-id main vertex)

k-th (internal) vertex in face i
  id = 12 + 30*N + i*N*(N-1)/2 + k; (0 <= k < N*(N-1)/2)
(order is not important )

edge id from edge-vertex-id e
  e' = (e-12)
  k  = e'%N
  i  = (e'-k)/N             <-- vertex-nodes base id

face id from face-vertex id f
  f' = f - 12 - 30*N
  k  = f'%(N*(N-1)/2)
  i  = (f'-k)/(N*(N-1)/2)   <-- face-nodes base id

*******/

/****** number of elements
number of vertices:
  V = 12 + 30*s +20*s*(s-1)/2   = 12 + 10*s*(s+2) = 2 + 10*(s+1)^2
 
number of edges:
  E = 30*(s+1) + 20*3*s*(s+1)/2 = 30*(s+1)^2

number of faces:
  F = 20*(s+1)^2

******/

//F2V
int EQsahedron::s_aMainFaces[ICOFACES][VERTS_PER_FACE] = {
    // top
    { 0,  1,  2},
    { 0,  2,  3},
    { 0,  3,  4},
    { 0,  4,  5}, 
    { 0,  5,  1},

    // second & third row (alternatingly downpointing & uppointing)
    { 6,  2,  1},
    { 2,  6,  7},
    { 7,  3,  2},
    { 3,  7,  8},
    { 8,  4,  3},
    { 4,  8,  9},
    { 9,  5,  4}, 
    { 5,  9, 10},
    {10,  1,  5},
    { 1, 10,  6},


    // bottom
    {11,  7,  6},
    {11,  8,  7},
    {11,  9,  8},
    {11, 10,  9},
    {11,  6, 10},
};

// F2E[i]: edge base ids for face i
int EQsahedron::s_F2E[ICOFACES][EDGES_PER_FACE] = {
    { 0,  1,  5}, 
    { 1,  2,  6}, 
    { 2,  3,  7}, 
    { 3,  4,  8}, 
    { 4,  0,  9}, 

    { 5, 10, 11}, 
    {11, 12, 20}, 
    { 6, 12, 13}, 
    {13, 14, 21}, 
    { 7, 14, 15}, 
    {15, 16, 22}, 
    { 8, 16, 17}, 
    {17, 18, 23}, 
    { 9, 18, 19}, 
    {10, 19, 24}, 

    {20, 25, 26}, 
    {21, 26, 27}, 
    {22, 27, 28}, 
    {23, 28, 29}, 
    {24, 29, 25}, 
};

// E2V[i]: vertexes bounding edge i - IMPORTANT: pairs must be <-ordered
int EQsahedron::s_E2V[ICOEDGES][VERTS_PER_EDGE] = {
    { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, 
    { 1, 2}, { 2, 3}, { 3, 4}, { 4, 5}, { 1, 5},
    { 1, 6}, { 2, 6}, { 2, 7}, { 3, 7}, { 3, 8},
    { 4, 8}, { 4, 9}, { 5, 9}, { 5,10}, { 1,10},
    { 6, 7}, { 7, 8}, { 8, 9}, { 9,10}, { 6,10},
    { 6,11}, { 7,11}, { 8,11}, { 9,11}, {10,11},
};


int EQsahedron::s_V2E[ICOVERTS][EDGES_PER_VERT] = {
    { 0, 1, 2, 3, 4}, { 0, 5, 9,10,19}, { 1, 6, 7,11,12}, { 2, 6, 7,13,14}, 
    { 3, 7, 8,15,16}, { 4, 8, 9,17,18}, {10,11,12,20,25}, {12,13,20,21,26},
    {14,15,21,22,27}, {16,17,22,23,28}, {18,19,23,24,29}, {15,26,27,28,29}
};

static bool s_bVerbose = false;

// E2F[i]: faces sharing edge i
int EQsahedron::s_E2F[ICOEDGES][FACES_PER_EDGE] = {
    { 0, 4}, { 0, 1}, { 1, 2}, { 2, 3}, { 3, 4},
    { 0, 5}, { 1, 7}, { 2, 9}, { 3,11}, { 4,13},

    { 5,14}, { 5, 6}, { 6, 7}, { 7, 8}, { 8, 9},
    { 9,10}, {10,11}, {11,12}, {12,13}, {13,14},

    { 6,15}, { 8,16}, {10,17}, {12,18}, {14,19},
    {15,19}, {15,16}, {16,17}, {17,18}, {18,19},
};

int EQsahedron::s_V2F[ICOVERTS][FACES_PER_VERT] = {
    { 0, 1, 2, 3, 4}, { 0, 4, 5,13,14}, { 0, 1, 5, 6, 7},
    { 1, 2, 7, 8, 9}, { 2, 3, 9,10,11}, { 3, 4,11,12,13},  
    { 5, 6,14,15,19}, { 6, 7, 8,15,16}, { 8, 9,10,16,17},
    {10,11,12,17,18}, {12,13,14,18,19}, {15,16,17,18,19},
};


void setVertex(Vec3D *pv,int iFaceNum, int iVertNum) {
    
}


//-----------------------------------------------------------------------------
// constructor
//
EQsahedron::EQsahedron()
    : m_iSubDivs(0),
      m_bTegmark(true),
      m_apIcoFaces(NULL),
      m_pVL(NULL),
      m_curBox(0,0,0,0),
      //      m_curBox(-Q_PI,Q_PI+1e-8,-Q_PI/2, Q_PI/2+1e-8),
      //      m_fMinAlt(fNaN),
      m_bPartial(false) {


    memset((void*)m_tiTrans, 0, ICOFACES*sizeof(transinfo));

    memset(m_apMainVertices, 0, ICOVERTS*sizeof(Vec3D*));
    memset(m_apMainFaces,    0, ICOFACES*sizeof(Vec3D*));
    memset(m_apEQFaces, 0, ICOFACES*sizeof(EQTriangle*));

    calcIcoVerts();
    createFaces();

}


//-----------------------------------------------------------------------------
// destructor
//
EQsahedron::~EQsahedron() {

    for (int i = 0; i < ICOVERTS; i++) {
        if (m_apMainVertices[i] != NULL) {
            delete m_apMainVertices[i];
        }
    }

    for (int i = 0; i < ICOFACES; i++) {
        if (m_apEQFaces[i] != NULL) {
            delete m_apEQFaces[i];
        }
    }

    for (int i = 0; i < ICOFACES; i++) {
        if (m_apMainFaces[i] != NULL) {
            delete m_apMainFaces[i];
        }
    }

    if (m_apIcoFaces != NULL) {
        for (int i = 0; i < m_iNumIcoFaces; i++) {
            delete m_apIcoFaces[i];
        }
        delete[] m_apIcoFaces;
    }

    if (m_pVL != NULL) {
        delete m_pVL;
    }


}


//-----------------------------------------------------------------------------
// createInstance
//
EQsahedron *EQsahedron::createInstance(int iNumSubDivs, bool bTegmark) {
    EQsahedron *pEQS = new EQsahedron();
    int iResult = pEQS->init(iNumSubDivs, bTegmark);
    if (iResult < 0) {
        delete pEQS;
        pEQS = NULL;
    }
    return pEQS;
}


//-----------------------------------------------------------------------------
// createInstance
//
EQsahedron *EQsahedron::createEmpty() {
    return new EQsahedron();
}


//-----------------------------------------------------------------------------
// init
//
int EQsahedron::init(int iSubDivs, bool bTegmark) {
    int iResult = 0;
    if (iSubDivs >= 0) {
        m_iSubDivs = iSubDivs;
        m_bTegmark = bTegmark;
    
        EQTriangle *pEQ = EQTriangle::createInstance(m_iSubDivs, m_bTegmark);
        
        if (pEQ != NULL) {
             
            for (int i = 0; i < ICOFACES; i++) {
                mapTriangle(pEQ, i);
            }

            delete pEQ;

            makeIcoFaces();

        }
    } else {
        iResult = -1;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// makeIcoFaces
//  - create IcoFace arrays for each main face
//  - afterwards merge them into m_apIcoFaces
//
void EQsahedron::makeIcoFaces() {
    if (s_bVerbose) printf("Making Ico faces\n");  
    //    int iResult = 0;
    //    m_fMinAlt = fMinAlt;
  
    long aiNumIcoFaces[ICOFACES];
    memset(aiNumIcoFaces, 0, ICOFACES*sizeof(long));

    for (int i = 0; i < ICOFACES; i++) {
        aiNumIcoFaces[i] = m_apEQFaces[i]->getNumTriangles();
    }
    
    IcoFace ***aapIcoFaces = new IcoFace**[ICOFACES];
    for (int i = 0; i < ICOFACES; i++) {
        aapIcoFaces[i] = new IcoFace*[aiNumIcoFaces[i]];
        memset(aapIcoFaces[i], 0, aiNumIcoFaces[i]*sizeof(IcoFace*));
    }

    long aiCurIdx[ICOFACES];
    memset(aiCurIdx, 0, ICOFACES*sizeof(long));
    
    //   omp_set_num_threads(2);

    //#pragma omp parallel for
    
    for (int i = 0; i < ICOFACES; i++) {
        triangle *pTriangles = m_apEQFaces[i]->getTriangles();
        node *pNodes = m_apEQFaces[i]->getNodes();
        for (gridtype j = 0;  j < m_apEQFaces[i]->getNumTriangles(); j++) {   
            
                
            aapIcoFaces[i][aiCurIdx[i]] = IcoFace::createFace(&(pNodes[pTriangles[j].aiIdx[0]].v),
                                                              &(pNodes[pTriangles[j].aiIdx[1]].v),
                                                              &(pNodes[pTriangles[j].aiIdx[2]].v));

            aapIcoFaces[i][aiCurIdx[i]]->setIDs(pNodes[pTriangles[j].aiIdx[0]].lID,
                                                pNodes[pTriangles[j].aiIdx[1]].lID,
                                                pNodes[pTriangles[j].aiIdx[2]].lID);
            
            aiCurIdx[i]++;  
            
        }
        
    }

    
    m_iNumIcoFaces = 0;
    for (int i = 0; i < ICOFACES; i++) {
        m_iNumIcoFaces += aiCurIdx[i];
    }
    m_apIcoFaces = new IcoFace *[m_iNumIcoFaces];
    IcoFace **pCur = m_apIcoFaces;
    for (int i = 0; i < ICOFACES; i++) {
        memcpy(pCur, aapIcoFaces[i], aiCurIdx[i]*sizeof(IcoFace*));
        pCur +=  aiCurIdx[i];
        delete[] aapIcoFaces[i];
    }
    delete[] aapIcoFaces;

    if (s_bVerbose) printf("[EQsahedron::makeIcoFaces] Num actual faces %ld\n", m_iNumIcoFaces);
} 

    
//-----------------------------------------------------------------------------
// calcSinAngle
//
double EQsahedron::calcSinAngle() {
    double dDPhi = 2*Q_PI/5;
    double sps=sin(dDPhi/2);
    sps=2*sps*sps;
    double dSA = (1 - sqrt(1 - 4* sps*(1-sps)))/(2*sps);
    //    printf("dSA = %f -> (%f deg)\n", dSA, 180*asin(dSA)/Q_PI);
    return dSA;
}


//-----------------------------------------------------------------------------
// calcIcoVerts
//  calculate the vertices of a regular icosahedron
// 
void EQsahedron::calcIcoVerts() {
    // calculate sine and cosine of latitude at which 
    // the lowest vertices of a top triangle lie
    double dS = calcSinAngle();
    double dC = sqrt(1-dS*dS);
    // calculate vertexes
    // top and bottom
    m_apMainVertices[0]  = new Vec3D(0,0,1);
    m_apMainVertices[11] = new Vec3D(0,0,-1);

    double dPhi = 0;
    // first row
    double dDPhi = 2*Q_PI/5;
    for (int i = 0; i < 5; i++) {
        m_apMainVertices[i+1]=new Vec3D(dC*cos(dPhi), dC*sin(dPhi), dS);
        dPhi += dDPhi;
    }
    // second row
    dPhi = dDPhi/2;
    for (int i = 0; i < 5; i++) {
        m_apMainVertices[i+6]=new Vec3D(dC*cos(dPhi), dC*sin(dPhi), -dS);
        dPhi += dDPhi;
    }

}


//-----------------------------------------------------------------------------
// createFaces
//
void EQsahedron::createFaces() {

    for (int i = 0; i < ICOFACES; i++) {
        m_apMainFaces[i] = IcoFace::createFace(m_apMainVertices[s_aMainFaces[i][0]], 
                                               m_apMainVertices[s_aMainFaces[i][1]], 
                                               m_apMainVertices[s_aMainFaces[i][2]]);
    }
  
}


//-----------------------------------------------------------------------------
// calcTriangleMapping
//   find quaternion to rotate the equilateral 0-centered triangle A
//   onto the equilateral 0-centered triangle B
//
Quat EQsahedron::calcTriangleMapping(const Vec3D &vA1, const Vec3D &vA2, const Vec3D &vA3,
                                     const Vec3D &vB1, const Vec3D &vB2, const Vec3D &vB3) {

    // normal of first triangle
    Vec3D vnA = (vA2 - vA1) * (vA3 - vA2);
    vnA.normalize();
    

    // normal of second triangle
    Vec3D vnB = (vB2 - vB1) * (vB3 - vB2);
    vnB.normalize();

    // quaternion to rotate pnA to pnB
    Quat q0 = Quat::makeRotation(vnA, vnB);
    
    // quat to rotate image of pA1 under q0 onto pB1
    Vec3D vA10 = q0^vA1;
    Quat q1 = Quat::makeRotation(vA10, vB1); 

    // combine the quats
    q1 *= q0;
    
    
    return q1;
}


//-----------------------------------------------------------------------------
// findOrientedEdge
//   find base-ID for edge between V0->V1
//   set *pbReversed if edge is given as V1->V0
//  
//   assumes s_E2V[i] is ordered pair (first less than second)
//
int EQsahedron::findOrientedEdge(int iFaceNum, int iV0, int iV1, bool *pbReversed) {
    int e = -1;

    // swap if wrong order
    if (iV0 > iV1) {
        int iT = iV0;
        iV0 = iV1;
        iV1 = iT;
        *pbReversed = true;
    } else {
        *pbReversed = false;
    }
    // search edge id
    for (int i = 0; (e < 0) && (i < 3); i++) {
        int E =  s_F2E[iFaceNum][i];
        if (((iV0 == s_E2V[E][0]) && (iV1 == s_E2V[E][1]))) {
            e = E;
        }
    }
    return e;
}


//-----------------------------------------------------------------------------
// setGlobalIDs
//  calculate global IDs for nodes (in base triangle):
//  Vertex-nodes:  0 -> 11
//  Edge-nodes:   12 -> 12+30*m_iSubDivs-1
//  Face-nodes:   12+30*m_iSubDivs ->  12+30*m_iSubDivs+20*m_iSubDivs*(m_iSubDivs-1)/2 -1
//
//  V  Eh Eh Eh V      V: 
//  Ev F  F  Ed
//  Ev F  Ed
//  Ev Ed
//  V
//
void EQsahedron::setGlobalIDs(EQTriangle *pEQ, int iFaceNum) {
    int iN = pEQ->getNumNodes();
    node *pNodes = pEQ->getNodes();

    // set final vertex IDs
    pNodes[0].lID                  = s_aMainFaces[iFaceNum][0];
    pNodes[iN-1].lID               = s_aMainFaces[iFaceNum][1];
    pNodes[iN-m_iSubDivs-2].lID    = s_aMainFaces[iFaceNum][2];

    // set final edge-node IDs (right)
    bool bReversed = false;

    // nodes on diagonal side (indexes for diagonal side interior points: (i+2)*(i+3)/2-1 )
    int eID = findOrientedEdge(iFaceNum, pNodes[0].lID,  pNodes[iN-1].lID, &bReversed);
    int ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = (i+2)*(i+3)/2-1;
        pNodes[iIndex].lID = ebase + iReal;   
    }

    // nodes on vertical side (left) (indexes for vertical side interior points: (i+1)*(i+2)/2 )
    eID = findOrientedEdge(iFaceNum, pNodes[0].lID, pNodes[iN-m_iSubDivs-2].lID, &bReversed);
    ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = (i+1)*(i+2)/2;
        pNodes[iIndex].lID = ebase + iReal;   
    }

    // nodes on horizontal side (top) (indexes from iN-m_iSubDivs-1 to iN-1)
    eID = findOrientedEdge(iFaceNum,  pNodes[iN-m_iSubDivs-2].lID, pNodes[iN-1].lID, &bReversed);
    ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = iN - m_iSubDivs - 1 + i;
        pNodes[iIndex].lID = ebase + iReal;
    }

    // set final face-node IDs
    int fbase = ICOVERTS + ICOEDGES*m_iSubDivs + iFaceNum*m_iSubDivs*(m_iSubDivs-1)/2;
    for (int i = 0; i < (m_iSubDivs)*(m_iSubDivs-1)/2; i++) {
        // find row an column of interior triangle for element i (some magic)
        int r = (int)floor((sqrt(1+8*i)-1)/2);
        int c = i - r*(r+1)/2;
        // calculate node index for row and column
        int iIndex = (r+2)*(r+3)/2+c+1;
        pNodes[iIndex].lID = i+fbase;
    }

}


//-----------------------------------------------------------------------------
// mapTriangle
//
void EQsahedron::mapTriangle(EQTriangle *pEQ, int iFaceNum) {
    bool bCheck = false;

    EQTriangle *pEQC = pEQ->copy();

    setGlobalIDs(pEQC, iFaceNum);


    double T0  = tan(Q_PI/5);
    double dSideEQTri =  sqrt(9*T0*T0-3);
    double dSideIcoTri = 1/sin(2*Q_PI/5);
    
  
    // the vertices of the face pEQC is mapped to
    Vec3D w1(m_apMainVertices[s_aMainFaces[iFaceNum][0]]);
    Vec3D w2(m_apMainVertices[s_aMainFaces[iFaceNum][1]]);
    Vec3D w3(m_apMainVertices[s_aMainFaces[iFaceNum][2]]);

    // make w1w2w3 0-centered
    Vec3D d = (w1 + w2 + w3)*(1.0/3.0);
    w1 -= d;
    w2 -= d;
    w3 -= d;

    // v1v2v3 is already 0 centered
    Vec3D *v1 = pEQC->getCorner(0);
    Vec3D *v2 = pEQC->getCorner(1);
    Vec3D *v3 = pEQC->getCorner(2);
    
    // build & remember the transformation plane -> ico face
    m_tiTrans[iFaceNum].vShift  = d;
    m_tiTrans[iFaceNum].qRot    = calcTriangleMapping(*v1, *v2, *v3, w1, w2, w3);
    m_tiTrans[iFaceNum].qRotInv = !m_tiTrans[iFaceNum].qRot;
    m_tiTrans[iFaceNum].dScale  = dSideIcoTri/dSideEQTri;
    
    // apply the transformation: 
    //   adjust size
    pEQC->scale(m_tiTrans[iFaceNum].dScale);
    //   set orientation (I don't understand why i have to take the inverse rotation)
    pEQC->applyQuat(m_tiTrans[iFaceNum].qRotInv);
    //   shift outer corners to sphere
    pEQC->shift(m_tiTrans[iFaceNum].vShift);
    //   push all nodes to sphere
    pEQC->normalize();
    
    m_apEQFaces[iFaceNum] = pEQC;


    /* check reverse transformation tegmark triangle -> sphere*/
    //    printf("face %d\n", iFaceNum);
    if (bCheck) {
        Vec3D e1(&w2);
        e1.subtract(&w1);
        Vec3D e2(&w3);
        e2.subtract(&w2);
        Vec3D *n = e1.crossProduct(&e2);
        for (int i = 0; i < pEQC->getNumNodes(); i++) {
            // invert normaliozation (extend to plane)
            Vec3D v(pEQC->getNodes()[i].v);
            double dL =  n->dotProduct(&pEQC->getNodes()[0].v)/n->dotProduct(&v);
            v.scale(dL);
            
            Vec3D *pV = icoToPlane(iFaceNum, &v);
            
            double dDist =  pV->dist(&pEQ->getNodes()[i].v);
            //if (dDist > 0*1e-8) {
            if (!(*pV == pEQ->getNodes()[i].v)) {
                //            printf("face %d\n", iFaceNum);
                printf("  ori %d     : % f % f % f\n", i,  pEQ->getNodes()[i].v.m_fX, pEQ->getNodes()[i].v.m_fY, pEQ->getNodes()[i].v.m_fZ);
                printf("  icc %d %4d: % f % f % f\n", i, pEQC->getNodes()[i].lID,  pEQC->getNodes()[i].v.m_fX,  pEQC->getNodes()[i].v.m_fY,  pEQC->getNodes()[i].v.m_fZ);            
                printf("  pla %d     : % f % f % f\n", i, pV->m_fX, pV->m_fY, pV->m_fZ);
                printf("  dist ori-pla: %e\n", dDist);
            }
            delete pV;
            
        }
    }
}


//-----------------------------------------------------------------------------
// load
//
int EQsahedron::load(const std::string sFile) {
    int iResult = -1;

    int iSubDivs = 0;
    bool bEqualArea=true;

    /*
    float fMinAlt = fNaN;
    ValReader *pVR = NULL;
    */
    tbox box;
 

    FILE *fIn = fopen(sFile.c_str(), "rb");
    if (fIn != NULL) {
        iResult = -1;
        char sLine[1024];
        char *p = NULL;

        // forward to find magic
        *sLine = '#';
        while (*sLine == '#') {
            p = fgets(sLine, 1024, fIn);
        }
        if ((p != NULL) && (strcmp(trim(p), KEY_MAGIC) == 0)) {
            iResult = 0;
                                
        } else {
            printf("No magic start [%s]\n", p);
        }


        if (iResult == 0) {
            iResult = -1;
            // forward to find level
            *sLine = '#';
            while (*sLine == '#') {
                p = fgets(sLine, 1024, fIn);
            }
            if (p != NULL) {
                p = trim(p);
                if (strstr(p, KEY_LEVEL) == p) {
                    p = strchr(p, ':')+1;
                    if (strToNum(p, &iSubDivs)) {
                        iResult = 0;
                    } else {
                        printf("Invalid number [%s]\n", p);
                    }
                } else {
                    printf("Expected [%s] instead of [%s]\n", KEY_LEVEL, p);
                }
            } else {
                printf("Expected keyword [%s]\n", KEY_LEVEL);
            }
        }

        if (iResult == 0) {
            iResult = -1;
            // forward to find equal
            *sLine = '#';
            while (*sLine == '#') {
                p = fgets(sLine, 1024, fIn);
            }
            if (p != NULL) {
                p = trim(p);
                if (strstr(p, KEY_EQUAL) == p) {
                    p = strchr(p, ':')+1;
                    p = trim(p);
                    if  (strcmp(p, "on")==0) {
                        iResult = 0;
                        bEqualArea = true;
                    } else if (strcmp(p, "off")==0) {
                        iResult = 0;
                        bEqualArea = false;
                    } else {
                        iResult = -1;
                        printf("bad value for equal area attribute [%s]\n", p);
                    }
                } else {
                    printf("Expected [%s] instead of [%s]\n", KEY_EQUAL, p);
                }
            } else {
                printf("Expected keyword [%s]\n", KEY_EQUAL);
            }
        }
        
        /*
        if (iResult == 0) {
            iResult = -1;
            // forward to find land

            *sLine = '#';
            while (*sLine == '#') {
                p = fgets(sLine, 1024, fIn);
            }
            if (p != NULL) {
                p = trim(p);
                if (strstr(p, KEY_LAND) == p) {
                    p = strchr(p, ':')+1;
                    if (strToNum(p, &fMinAlt)) {
                        iResult = 0;
                    } else {
                        printf("Invalid number [%s]\n", p);
                    }
                } else {
                    printf("Expected [%s] instead of [%s]\n", KEY_LAND, p);
                }
                
            } else {
                printf("Expected keyword [%s]\n", KEY_LAND);
            }
        }

        if (iResult == 0) {
            iResult = -1;
            // forward to find rect
            *sLine = '#';
            while (*sLine == '#') {
                p = fgets(sLine, 1024, fIn);
            }
            if (p != NULL) {
                p = trim(p);
                if (strstr(p, KEY_RECT) == p) {
                    p = strchr(p, ':')+1;
                    p = trim(p);
                    int iRead = sscanf(p, "%lf %lf %lf %lf", &m_curBox.dLonMin, &m_curBox.dLonMax, &m_curBox.dLatMin, &m_curBox.dLatMax);
                    
                    if (iRead == 4) {
                        iResult = 0;
                    } else {
                        printf("bad rect specification [%s]\n", p);
                    }
                } else {
                    printf("Expected [%s] instead of [%s]\n", KEY_RECT, p);
                }
            } else {
                printf("Expected keyword [%s]\n", KEY_RECT);
            }
        }
        */
        fclose(fIn);
    } else {
        xha_printf("Couldn't open file [%s] for reading\n", sFile);
    }

    if (iResult == 0) {
        xha_printf("Calling init with subdiv %d, equalarea %s\n", 
               iSubDivs, bEqualArea?"yes":"no");
        iResult = init(iSubDivs, bEqualArea);
        //        setLand(fMinAlt);
    }
    
    return iResult;
}


//-----------------------------------------------------------------------------
// save
//
int EQsahedron::save(const std::string sFile) {
    int iResult = -1;
   
    FILE *fOut = fopen(sFile.c_str(), "wb");
    if (fOut != NULL) {
        iResult = 0;
        fprintf(fOut, "%s\n",     KEY_MAGIC);
        fprintf(fOut, "%s%d\n",   KEY_LEVEL, m_iSubDivs);
        //        fprintf(fOut, "%s%f %f %f %f\n", KEY_ROT,  m_qGlobal.m_fR,  m_qGlobal.m_fI,  m_qGlobal.m_fJ,  m_qGlobal.m_fK);
        fprintf(fOut, "%s%s\n", KEY_EQUAL,  m_bTegmark?"on":"off");
        //        fprintf(fOut, "%s%f\n",   KEY_LAND,  m_fMinAlt);
        //        fprintf(fOut, "%s%f %f %f %f\n", KEY_RECT, m_curBox.dLonMin, m_curBox.dLonMax, m_curBox.dLatMin, m_curBox.dLatMax);

        fclose(fOut);
    } else {
        xha_printf("Couldn't open file [%s] for writing reading\n", sFile);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// getFirstFace
//  get first face from face list
//
PolyFace *EQsahedron::getFirstFace() {
    m_iCurFace = 0;
    return getNextFace();
}


//-----------------------------------------------------------------------------
// getNextFace
//  get next face from face list
//
PolyFace *EQsahedron::getNextFace() {
    IcoFace *pF = NULL;
    //    if (m_iCurFace < m_iNumIcoFaces) {
    while ((pF == NULL) && (m_iCurFace < m_iNumIcoFaces)) {
        pF = m_apIcoFaces[m_iCurFace++];
    }

    return pF;
}


//-----------------------------------------------------------------------------
// relink
//  reestablish the Vertex connections
//
void EQsahedron::relink() {
    if (m_pVL != NULL) {
        delete m_pVL;
    }
    m_pVL = new VertexLinkage();
    
    clock_t w1 = clock();
    for (long i = 0; i < m_iNumIcoFaces; i++) {
        m_pVL->addFace(m_apIcoFaces[i], m_apIcoFaces[i]->getIDs());
    }
    clock_t w2 = clock();

    if (s_bVerbose) {
        printf("Relink: %fs\n", ((double)(w2-w1))/CLOCKS_PER_SEC);
        printf("VL has %d faces\n", m_pVL->getNumFaces());
        printf("VL has %zd nodes\n", m_pVL->m_mI2V.size());
    }  
}


//-----------------------------------------------------------------------------
// findFaceSlow
//  IcoLoc/Surface implementation
//  get face containing coords
//  
PolyFace *EQsahedron::findFaceSlow(double dLon, double dLat) {
    double c1=cos(dLat);
    Vec3D v(c1*cos(dLon), c1*sin(dLon), sin(dLat));
    
    PolyFace *pF0 = NULL;
    for (long i = 0; (pF0 == NULL) && (i < m_iNumIcoFaces); i++) {
        PolyFace *pF = m_apIcoFaces[i]->contains(&v);
        if (pF != NULL) {
            pF0 = pF;
        }
    }
    return pF0;
}


//-----------------------------------------------------------------------------
// findNodeSlow
//  Surface implementation
//  get if of node closest to pv
//  
gridtype EQsahedron::findNodeSlow(Vec3D *pv) {
    gridtype iID = -1;

    PolyFace *pF0 = NULL;
    for (long i = 0; (pF0 == NULL) && (i < m_iNumIcoFaces); i++) {
        PolyFace *pF = m_apIcoFaces[i]->contains(pv);
        if (pF != NULL) {
            pF0 = pF;
        }
    }
    if (pF0 != NULL) {
        iID = pF0->closestVertexID(pv);
        //        printf("closest1 %f, %f, %f: d:%f\n", pvClosest->m_fX, pvClosest->m_fY, pvClosest->m_fZ, pv->dist(pvClosest));
    }

    return iID;
}


//-----------------------------------------------------------------------------
// display
//   Surface implementation
//
void EQsahedron::display() {
    printf("EQsahedron %p...\n", this);
    for (int i = 0; i < ICOFACES; i++) {
        m_apEQFaces[i]->displayNodes(true);
    }
}


//-----------------------------------------------------------------------------
// findNode
//  IcoLoc implementation
//  fin id of node closest to coords
//  
gridtype EQsahedron::findNode(double dLon, double dLat) {
    dLat = dLat * Q_PI / 180.0;
    dLon = dLon * Q_PI / 180.0;
    double c1=cos(dLat);
    Vec3D v(c1*cos(dLon), c1*sin(dLon), sin(dLat));
    
    return findNode(&v);
}


//-----------------------------------------------------------------------------
// findCoords
//   IcoLoc implementation
//   find longitude and latitude of node with given ID
//
bool EQsahedron::findCoords(int iNodeID, double *pdLon, double *pdLat) {
    bool bOK = false;
    Vec3D *pV = m_pVL->getVertex(iNodeID);
    if (pV != NULL) {
        cart2Sphere(pV, pdLon, pdLat);
        bOK = true;
    }
    return bOK;
}


//-----------------------------------------------------------------------------
// icoToPlane
//   move point from ico face to tegmark triangle
//    - shift it (ico center ->0)
//    - rotate it to align it to tegmark triangle
//    - scale it to tegmark triangle size
//
Vec3D * EQsahedron::icoToPlane(int iFace, Vec3D *pV) {
    pV->subtract(m_tiTrans[iFace].vShift);
    //Vec3D *pW = m_tiTrans[iFace].qRotInv.apply(pV);
    Vec3D *pW = (&m_tiTrans[iFace].qRotInv)->apply(pV);
    pW->scale(1.0/m_tiTrans[iFace].dScale);
    return pW;
}


//-----------------------------------------------------------------------------
// planeToIco
//  move point from tegmark triangle to ico face:
//   - scale it to ico face size
//   - rotate it to ico face orientation
//   - shift it to ico face position
// 
Vec3D * EQsahedron::planeToIco(int iFace, Vec3D *pV) {
    pV->scale(m_tiTrans[iFace].dScale);
    //orig    Vec3D *pW = m_tiTrans[iFace].qRot.apply(pV);
    Vec3D *pW = (&m_tiTrans[iFace].qRot)->apply(pV);
    pW->add(m_tiTrans[iFace].vShift);
    return pW;
}



//-----------------------------------------------------------------------------
// icoToPlane
//   move point from ico face to tegmark triangle
//    - shift it (ico center ->0)
//    - rotate it to align it to tegmark triangle
//    - scale it to tegmark triangle size
//
Vec3D EQsahedron::icoToPlane(int iFace, const Vec3D &vV) {
    Vec3D vU = vV - m_tiTrans[iFace].vShift;
    //Vec3D *pW = m_tiTrans[iFace].qRotInv.apply(pV);
    Vec3D vW = m_tiTrans[iFace].qRotInv ^ vU;
    vW *= 1.0/m_tiTrans[iFace].dScale;
    return vW;
}


//-----------------------------------------------------------------------------
// planeToIco
//  move point from tegmark triangle to ico face:
//   - scale it to ico face size
//   - rotate it to ico face orientation
//   - shift it to ico face position
// 
Vec3D EQsahedron::planeToIco(int iFace, const Vec3D &vV) {
    Vec3D vU = m_tiTrans[iFace].dScale * vV;
    //orig    Vec3D *pW = m_tiTrans[iFace].qRot.apply(pV);
    Vec3D vW = m_tiTrans[iFace].qRot ^ vU;
    vW += m_tiTrans[iFace].vShift;
    return vW;
}


//-----------------------------------------------------------------------------
// sphereToIco
//  project a pV to the face intersected by the ray given by pV
//
Vec3D *EQsahedron::sphereToIco(Vec3D *pV, int *piFace) {
    int iSel = -1;
    for (int i =0; (iSel < 0) && (i < ICOFACES); i++) {
        PolyFace *pF = m_apMainFaces[i]->contains(pV);
        if (pF != NULL) {
            iSel = i;
        }
    }
    // need to intersect line given by pV with face:
    // d = (v0.n)/(v.n)
    // intersection: d*v
    Vec3D *pN = m_apMainFaces[iSel]->getNormal();
    Vec3D *pV0 = &m_apEQFaces[iSel]->getNodes()[0].v;
    // pV =pV0;
    double dL = pN->dotProduct(pV0)/pN->dotProduct(pV);
    pV->scale(dL);
    *piFace = iSel;
    return pV;
}


//-----------------------------------------------------------------------------
// findNode
//   find closest node to pV 
//     project it to the base triangle to find neighbors
//
gridtype EQsahedron::findNode(Vec3D *pV0) {
    gridtype lID = -1;
    bool bCheck = false;

    Vec3D vOrig(pV0);

    int iSel = -1;
    // move pV from sphere to ico face
    Vec3D *pV = sphereToIco(&vOrig, &iSel);

    if (iSel >= 0) {
        // move pV to the plane
        Vec3D *pW=icoToPlane(iSel, pV);
        
       
        //        printf("Vector should now be in xyplane:  [%f %f %f]\n", pW->m_fX, pW->m_fY, pW->m_fZ);

        gridtype aiN[4];
        int iNumN = m_apEQFaces[iSel]->findNeighbors(pW, aiN);

        delete pW;
        /*        
        printf("Found %d neighbors: ", iNumN);
        for (int i = 0; i < iNumN; i++) {
            int r = (int)(sqrt(1+8* aiN[i])-1)/2;
            printf("%lld (r%d,c%d) ", aiN[i], r, (int)aiN[i]-(r*(r+1)/2));
        }
        printf("\n");
        */
        int iMin = -1;
        double dMin = dPosInf;
        for (int i = 0; i < iNumN; i++) {
            int r = (int)(sqrt(1+8* aiN[i])-1)/2;
            int c = (int)(aiN[i]-(r*(r+1)/2));
            Vec3D vs;
            m_apEQFaces[iSel]->shearScale(c, r, &vs);
            Vec3D *pX = Tegmark::distort(&vs);
           
            pX = planeToIco(iSel, pX);
            pX->normalize();
            double d = pV0->dist(pX);
            if (d < dMin) {
                iMin = i;
                dMin = d;
            }
            //            printf("%lld (r%d,c%d)->%f\n", aiN[i], r, c, d);
            delete pX;
        }
        //  printf("closest: local %lld, global  %lld\n", aiN[iMin], m_apEQFaces[iSel]->getNodes()[aiN[iMin]].lID);
        lID =  m_apEQFaces[iSel]->getNodes()[aiN[iMin]].lID;
        
        if (bCheck) {
            iMin = -1;
            dMin = dPosInf;
            for (int i = 0; i < m_apEQFaces[iSel]->getNumNodes(); i++) {
                double d = vOrig.dist(&m_apEQFaces[iSel]->getNodes()[i].v);
                if (d < dMin) {
                    iMin = i;
                    dMin = d;
                }
            }
            Vec3D vpp(&m_apEQFaces[iSel]->getNodes()[iMin].v);
            printf("RealCLosest: %d\n", m_apEQFaces[iSel]->getNodes()[iMin].lID);
        }

    } else {
        printf("Couldn't find face for [%f %f %f]\n", pV->m_fX, pV->m_fY, pV->m_fZ);
    }
    return lID;
}


//-----------------------------------------------------------------------------
// findFace
//   find closest node to pV 
//     project it to the base triangle to find neighbors
//
PolyFace *EQsahedron::findFace(double dLon, double dLat) {
    IcoFace *pF = NULL;

    // do nothing for invalid values (face = NULL)
    if (!std::isnan(dLon) &&  !std::isnan(dLat)) {
        double c1=cos(dLat);
        Vec3D v(c1*cos(dLon), c1*sin(dLon), sin(dLat));

        int iSel = -1;
        // move pV from sphere to ico face
        Vec3D *pV = sphereToIco(&v, &iSel);
        
        if (iSel >= 0) {
            // move pV to the plane
            Vec3D *pW=icoToPlane(iSel, pV);
            
            // get Id of triangle in basetriangle
            gridtype lIDFace = m_apEQFaces[iSel]->findFaceID(pW);
            
            // the faces are stored by face number 
            // triangles of face 0, triangles of face 1, ... triangles of face 20
            // the triangles of a face are stored in the order given by EQTriangle
            gridtype lIDGlobal = iSel*(m_iSubDivs+1)*(m_iSubDivs+1)+lIDFace;
            
            pF = m_apIcoFaces[lIDGlobal];
            
            delete pW;
        } else {
            printf("Couldn't find face for [%f %f %f]\n", pV->m_fX, pV->m_fY, pV->m_fZ);
        }
    }
    return pF;
}


//-----------------------------------------------------------------------------
// convertTriangleToEQ
//   calculate global EQSahedron ID for node iIndex on specified face
//
int EQsahedron::convertTriangleToEQ(int iFaceNum, int iIndex) {
    int iEQID = -1;
    int iN = m_apEQFaces[iFaceNum]->getNumNodes();

    int v0 = s_aMainFaces[iFaceNum][0];
    int v1 = s_aMainFaces[iFaceNum][1];
    int v2 = s_aMainFaces[iFaceNum][2];

    if (iIndex == 0) {
        iEQID = v0;
    } else if (iIndex == iN-1) {
        iEQID = v1;
    } else if (iIndex == iN-m_iSubDivs-2) {
        iEQID = v2;
    } else {
        int r = floor((sqrt(8*iIndex+1)-1)/2);
        int c = iIndex - r*(r+1)/2;
        //                printf("index %d -> r %d, c%d\n", iIndex, r, c);

        bool bReversed = false;
        if (c == 0) {
            // 0 < r < subdiv+1
            // node on vertical side (left) (indexes for vertical side interior points: (i+1)*(i+2)/2 )
            int eID = findOrientedEdge(iFaceNum, v0, v2, &bReversed);
            int ebase = ICOVERTS + eID*m_iSubDivs;
            int iReal = (bReversed)?m_iSubDivs-r:r-1;
            iEQID = ebase + iReal;
            //            printf("vertical (%d - %d) e %d, base %d, real %d\n", v0, v2, eID, ebase, iReal);
        } else if (r == m_iSubDivs+1) {
            // 0 < c < subdiv+1
            // nodes on horizontal side (top) (indexes from iN-m_iSubDivs-1 to iN-1)
            int eID = findOrientedEdge(iFaceNum,  v2, v1, &bReversed);
            int ebase = ICOVERTS + eID*m_iSubDivs;
            int iReal = (bReversed)?m_iSubDivs-c:c-1;
            iEQID = ebase + iReal;
            //            printf("horizontal (%d - %d) e %d, base %d, real %d\n", v2, v1, eID, ebase, iReal);
        } else if (r == c) {
            // 0< r = c < subdiv+1
            // nodes on diagonal side (indexes for diagonal side interior points: (i+2)*(i+3)/2-1 )
            int eID = findOrientedEdge(iFaceNum, v0,  v1, &bReversed);
            int ebase = ICOVERTS + eID*m_iSubDivs;
            int iReal = (bReversed)?m_iSubDivs-r:r-1;
            iEQID = ebase + iReal;   
            //            printf("diagonal (%d - %d) e %d, base %d, real %d\n", v0, v1, eID, ebase, iReal);
        } else {
            //facies
            int fbase = ICOVERTS + ICOEDGES*m_iSubDivs + iFaceNum*m_iSubDivs*(m_iSubDivs-1)/2;
            int iInternalIndex = (r-2)*(r-1)/2+c-1;
            iEQID = fbase + iInternalIndex;   
        }
    }

    
    return iEQID;
}


//-----------------------------------------------------------------------------
// convertEQToTriangle
//   calculate index and facenumber from global EQSahedron ID
//
int EQsahedron::convertEQToTriangle(int iEQID, int *piFaceNum) {
    
    int iIndex = -1;

    
    if (*piFaceNum < 0) {
        // find first face containing vertex
        //        printf("Searching face & index\n");
        for (int iFace = 0; (iIndex < 0) && (iFace < ICOFACES); iFace++) {
            EQTriangle *pEQT = getFaceTriangle(iFace);
            int iNumNodes = pEQT->getNumNodes();
            node *pNodes = pEQT->getNodes();
            for (int iV = 0; (iIndex < 0) && (iV < iNumNodes); iV++) {
                if (pNodes[iV].lID == iEQID) {
                    iIndex     = iV;
                    *piFaceNum = iFace;
                }            
            }
        }   
    } else {
        //        printf("Using face %d\n", *piFaceNum);
        EQTriangle *pEQT = getFaceTriangle(*piFaceNum);
        int iNumNodes = pEQT->getNumNodes();
        node *pNodes = pEQT->getNodes();

        for (int iV = 0; (iIndex < 0) && (iV < iNumNodes); iV++) {
            if (pNodes[iV].lID == iEQID) {
                iIndex     = iV;  
            }
        }
    }

    return iIndex;
}


//-----------------------------------------------------------------------------
// getFaceTriangle
//
EQTriangle *EQsahedron::getFaceTriangle(int iFaceNum) {
    return m_apEQFaces[iFaceNum];
}


//-----------------------------------------------------------------------------
// show
//
void EQsahedron::show() {
    for (int iFace = 0; iFace < 5/*ICOFACES*/; iFace++) {
        printf("-- face %02d --\n", iFace);
        EQTriangle *pEQ = m_apEQFaces[iFace];
        int iNumNodes = pEQ->getNumNodes();
        node *pNodes = pEQ->getNodes();
        int k = 1;
        int s = 2;
        for (int iNode = 0; iNode < iNumNodes; iNode++) {
            printf("%3d ", pNodes[iNode].lID);
            if (iNode == k-1) {
                k += s;
                s++;
                printf("\n");
            }
        }
    }
}

//-----------------------------------------------------------------------------
// show
//
void EQsahedron::dump(std::string sOut) {
    FILE *fOut = fopen(sOut.c_str(), "wt");
    xha_fprintf(fOut, "%d\n", m_iSubDivs);
    for (int iFace = 0; iFace < ICOFACES; iFace++) {
        EQTriangle *pEQ = m_apEQFaces[iFace];
        int iNumNodes = pEQ->getNumNodes();
        node *pNodes = pEQ->getNodes();
        for (int iNode = 0; iNode < iNumNodes; iNode++) {
            xha_fprintf(fOut, "%7d %f %f %f\n", pNodes[iNode].lID,  pNodes[iNode].v.m_fX,  pNodes[iNode].v.m_fY,  pNodes[iNode].v.m_fZ);
        }
    }
    fclose(fOut);
}
