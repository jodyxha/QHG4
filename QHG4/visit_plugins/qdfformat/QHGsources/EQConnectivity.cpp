#include <cstdio>
#include <cstring>
#include <cmath>


#include "EQConnectivity.h"

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

#define ICOVERTS 12
#define ICOFACES 20
#define ICOEDGES 30

//F2V
int s_aMainFaces[ICOFACES][3] = {
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
int s_F2E[ICOFACES][3] = {
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
int s_E2V[ICOEDGES][2] = {
    { 0, 1}, { 0, 2}, { 0, 3}, { 0, 4}, { 0, 5}, 
    { 1, 2}, { 2, 3}, { 3, 4}, { 4, 5}, { 1, 5},
    { 1, 6}, { 2, 6}, { 2, 7}, { 3, 7}, { 3, 8},
    { 4, 8}, { 4, 9}, { 5, 9}, { 5,10}, { 1,10},
    { 6, 7}, { 7, 8}, { 8, 9}, { 9,10}, { 6,10},
    { 6,11}, { 7,11}, { 8,11}, { 9,11}, {10,11},
};

// E2F[i]: faces sharing edge i
int s_E2F[ICOEDGES][2] = {
    { 0, 4}, { 0, 1}, { 1, 2}, { 2, 3}, { 3, 4},
    { 0, 5}, { 1, 7}, { 2, 9}, { 3,11}, { 4,13},

    { 5,14}, { 5, 6}, { 6, 7}, { 7, 8}, { 8, 9},
    { 9,10}, {10,11}, {11,12}, {12,13}, {13,14},

    { 6,15}, { 8,16}, {10,17}, {12,18}, {14,19},
    {15,19}, {15,16}, {16,17}, {17,18}, {18,19},
};



//-----------------------------------------------------------------------------
// calcNumNodes
//  Number of nodes for a EQsahedron with s subdivisions
//  N = 12 + 30s + 20*s(s-1)/2 
//
int EQConnectivity::calcNumNodes(int iSubDiv) {
    return 12 + 10*iSubDiv*(iSubDiv+2);
}


//-----------------------------------------------------------------------------
// calcNumEdges
//  Number of edges for a EQsahedron with s subdivisions
//  N = 30(s+1) + 20*3*s(s-1)/2 
//
int EQConnectivity::calcNumEdges(int iSubDiv) {
    return 30*(iSubDiv+1)*(iSubDiv+1);
}


//-----------------------------------------------------------------------------
// calcNumFaces
//
int EQConnectivity::calcNumFaces(int iSubDiv) {
    return 20*(iSubDiv+1)*(iSubDiv+1);
}


//-----------------------------------------------------------------------------
// createInstance
//
EQConnectivity *EQConnectivity::createInstance(int iSubDiv) {
    EQConnectivity *pEQC = new EQConnectivity(iSubDiv);
    int iResult = pEQC->init();
    if (iResult != 0) {
        delete pEQC; 
        pEQC = NULL;
    }
    return pEQC;
}


//-----------------------------------------------------------------------------
// destructor
//
EQConnectivity::~EQConnectivity() {
    if (m_aiTriangles != NULL) {
        delete[] m_aiTriangles;
    }

    if (m_aiBaseTriangles != NULL) {
        delete[] m_aiBaseTriangles;
    }

    if (m_aiTempIDs != NULL) {
        delete[] m_aiTempIDs;
    }
}


//-----------------------------------------------------------------------------
// constructor
//
EQConnectivity::EQConnectivity(int iSubDivs) 
    : m_iSubDivs(iSubDivs),
      m_iNumTriangles(0),
      m_aiTriangles(NULL),
      m_iNumBaseTriangles(0),
      m_aiBaseTriangles(NULL),
      m_iNumBaseNodes(0),
      m_aiTempIDs(NULL) {
}
 

//-----------------------------------------------------------------------------
// init
//
int EQConnectivity::init() {
    int iResult = 0;
    
    m_iNumTriangles = calcNumFaces(m_iSubDivs);
    m_aiTriangles = new int[3*m_iNumTriangles];

    // this fill create & fill m_aiBaseTriangles
    createSubdividedTriangle();

    int *pCur = m_aiTriangles;
    for (int i = 0; i < ICOFACES; i++) {
        pCur = setGlobalIDs(pCur, i);
    }

    
    return iResult;
};


//--------------------------------------------------------------
// createSubdividedTriangle
//  (from EQTriangle)
// 
//  creates a 0-centered equilateral triangle        ----
//  with horizontal top side and subdivide it,       \  /
//  keeping track of neighborship and subtriangles    \/ 
//  
//  This is done by starting from the point set      +--
//  {(x,y): 0 <= y <= N, 0 <= x <= y} (which forms   | /
//  a right triangle). In this setup neighborhood    |/
//  relations are easily determined.
//  At the same time the nodes' coordinates are calculated
//  to form the equilateral triangle.
//                                         
int EQConnectivity::createSubdividedTriangle() {
    int iN        = m_iSubDivs+1;
    m_iNumBaseNodes = (iN+1)*(iN+2)/2;

    m_iNumBaseTriangles = iN*iN;

    m_aiBaseTriangles = new int[3*m_iNumBaseTriangles];
    m_aiTempIDs       = new int[m_iNumBaseNodes];

    int i = 0;

    int iCurIndex = 0;
    memset(m_aiTriangles, 0, m_iNumBaseTriangles*3*sizeof(int));

    for (int y = 0; y < iN; y++) {
        for (int x = 0; x <= y; x++) {
       
            // define downward pointing triangle at current point (*)
            //  ___
            // |  /
            // | /
            // |/
            // *
            // (if exists)
            m_aiBaseTriangles[iCurIndex++] = i;
            m_aiBaseTriangles[iCurIndex++] = i + y + 2;
            m_aiBaseTriangles[iCurIndex++] = i + y + 1;

            // define upward pointing triangle at point (*)
            //          .
            //    /|    .
            //   / |    .  
            //  /  |    .
            // *---+    .
            // (if exists)
            if (x < y) {
                m_aiBaseTriangles[iCurIndex++] = i;
                m_aiBaseTriangles[iCurIndex++] = i + 1;
                m_aiBaseTriangles[iCurIndex++] = i + y + 2;
            }

            i++;
        }
        
    }

    /*    
    printf("Base triangles\n");
    for (int k = 0; k < m_iNumBaseTriangles; k++) {
        printf("%7d, %7d, %7d\n", m_aiBaseTriangles[3*k], m_aiBaseTriangles[3*k+1], m_aiBaseTriangles[3*k+2]);
    }

    showBaseTriangle();
    */
    return 0;
}


//-----------------------------------------------------------------------------
// findOrientedEdge
//   (from EQsahedron)
//   find base-ID for edge between V0->V1
//   set *pbReversed if edge is given as V1->V0
//  
//   assumes s_E2V[i] is ordered pair (first less than second)
//
int EQConnectivity::findOrientedEdge(int iFaceNum, int iV0, int iV1, bool *pbReversed) {
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
//  (from EQsahedron)
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
int *EQConnectivity::setGlobalIDs(int *pCurIndex, int iFaceNum) {
    int iN = m_iNumBaseNodes;

    // set final vertex IDs
    m_aiTempIDs[0]                = s_aMainFaces[iFaceNum][0];
    m_aiTempIDs[iN-1]             = s_aMainFaces[iFaceNum][1];
    m_aiTempIDs[iN-m_iSubDivs-2]  = s_aMainFaces[iFaceNum][2];

    // set final edge-node IDs (right)
    bool bReversed = false;

    // nodes on diagonal side (indexes for diagonal side interior points: (i+2)*(i+3)/2-1 )
    int eID = findOrientedEdge(iFaceNum, m_aiTempIDs[0],  m_aiTempIDs[iN-1], &bReversed);
    int ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = (i+2)*(i+3)/2-1;
        m_aiTempIDs[iIndex] = ebase + iReal;   
    }

    // nodes on vertical side (left) (indexes for vertical side interior points: (i+1)*(i+2)/2 )
    eID = findOrientedEdge(iFaceNum, m_aiTempIDs[0], m_aiTempIDs[iN-m_iSubDivs-2], &bReversed);
    ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = (i+1)*(i+2)/2;
        m_aiTempIDs[iIndex] = ebase + iReal;   
    }

    // nodes on horizontal side (top) (indexes from iN-m_iSubDivs-1 to iN-1)
    eID = findOrientedEdge(iFaceNum,  m_aiTempIDs[iN-m_iSubDivs-2], m_aiTempIDs[iN-1], &bReversed);
    ebase = ICOVERTS + eID*m_iSubDivs;
    for (int i = 0; i < m_iSubDivs; i++) {
        int iReal = (bReversed)?m_iSubDivs-1-i:i;
        int iIndex = iN - m_iSubDivs - 1 + i;
        m_aiTempIDs[iIndex] = ebase + iReal;
    }

    // set final face-node IDs
    int fbase = ICOVERTS + ICOEDGES*m_iSubDivs + iFaceNum*m_iSubDivs*(m_iSubDivs-1)/2;
    for (int i = 0; i < (m_iSubDivs)*(m_iSubDivs-1)/2; i++) {
        // find row an column of interior triangle for element i (some magic)
        int r = (int)floor((sqrt(1+8*i)-1)/2);
        int c = i - r*(r+1)/2;
        // calculate node index for row and column
        int iIndex = (r+2)*(r+3)/2+c+1;
        m_aiTempIDs[iIndex] = i+fbase;
    }

    // now fill triangles to the main triangle array
    for (int k = 0; k < m_iNumBaseTriangles; k++) {
        *pCurIndex++ = m_aiTempIDs[m_aiBaseTriangles[3*k]];
        *pCurIndex++ = m_aiTempIDs[m_aiBaseTriangles[3*k+1]];
        *pCurIndex++ = m_aiTempIDs[m_aiBaseTriangles[3*k+2]];
    }    

    return pCurIndex;
}


//-----------------------------------------------------------------------------
// showBaseTriangle
//
void EQConnectivity::showBaseTriangle() {
    int k = 0;
    for(int i = 0; i <= m_iSubDivs+1; i++) {
        for(int j = 0; j <= i; j++) {
            printf("%3d ", k++);
        }
        printf("\n");
    }
}


//-----------------------------------------------------------------------------
// showTempTriangle
//
void EQConnectivity::showTempTriangle() {
    int k = 0;
    for(int i = 0; i <= m_iSubDivs+1; i++) {
        for(int j = 0; j <= i; j++) {
            printf("%3d ", m_aiTempIDs[k++]);
        }
        printf("\n");
    }
}
