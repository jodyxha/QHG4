#ifndef __EQSAHEDRON_H__
#define __EQSAHEDRON_H__

#include "qhg_consts.h"
#include "Quat.h"
#include "IcoFace.h"
#include "Surface.h"
#include "IcoLoc.h"
#include "VertexLinkage.h"

class Vec3D;
class EQTriangle;
class ValReader;

const int ICOVERTS = 12;
const int ICOFACES = 20;
const int ICOEDGES = 30;

const int FACES_PER_VERT = 5;
const int FACES_PER_EDGE = 2;
const int VERTS_PER_EDGE = 2;
const int VERTS_PER_FACE = 3;
const int EDGES_PER_FACE = 3;
const int EDGES_PER_VERT = 5;

typedef struct {
    Vec3D   vShift;
    Quat    qRot;
    Quat    qRotInv;
    double  dScale;
} transinfo;

class EQsahedron : public Surface, public IcoLoc {

public:
    static int s_aMainFaces[ICOFACES][VERTS_PER_FACE];

    static int s_E2V[ICOEDGES][VERTS_PER_EDGE]; 
    static int s_V2E[ICOVERTS][EDGES_PER_VERT]; 
    static int s_F2E[ICOFACES][EDGES_PER_FACE];
    static int s_E2F[ICOEDGES][FACES_PER_EDGE];
    static int s_V2F[ICOVERTS][FACES_PER_VERT];

    static int calcNumVerts(int iSubDivs) { return 2+10*(iSubDivs+1)*(iSubDivs + 1);};
    static int calcNumEdges(int iSubDivs) { return   ICOEDGES*(iSubDivs+1)*(iSubDivs + 1);};
    static int calcNumFaces(int iSubDivs) { return   ICOFACES*(iSubDivs+1)*(iSubDivs + 1);};

    //    static EQsahedron *createInstance(int iSubDivs);    
    static EQsahedron *createInstance(int iSubDivs, bool bTegmark);
    static EQsahedron *createEmpty();
    ~EQsahedron();
 
    // from Surface
    int load(const std::string sFile);
    // from Surface
    int save(const std::string sFile);
    // from Surface
    virtual PolyFace *findFace(double dLon, double dLat);
    // slower version (brute force)
    virtual PolyFace *findFaceSlow(double dLon, double dLat);
    // from Surface
    virtual gridtype findNode(Vec3D *pV);
    // slower version
    virtual gridtype findNodeSlow(Vec3D *pv);
    // from Surface
    virtual gridtype findNode(double dLon, double dLat);
    // from Surface
    tbox *getBox() {return &m_curBox;};
    // from Surface
    virtual void display();
    // from Surface
    virtual Vec3D* getVertex(gridtype lID) { return m_pVL->getVertex(lID);};
    // from Surface
    virtual int collectNeighborIDs(gridtype lID, int iDist, std::set<gridtype> & sIds) { return m_pVL->collectNeighborIDs(lID, iDist, sIds);};
    // from IcoLoc
    virtual bool findCoords(int iNodeID,double *pdLon, double *pdLat);

    VertexLinkage *getLinkage() { return m_pVL;};

    PolyFace *getFirstFace();
    PolyFace *getNextFace();

    void relink();

    void setGlobalIDs(EQTriangle *pEQ, int iFaceNum);
    bool isPartial() { return m_bPartial;};
    /*
    void setLand(float fMinAlt) { m_fMinAlt = fMinAlt;};
    float getLand() { return m_fMinAlt;};
    */
    int getSubDivs() { return m_iSubDivs; };

    int convertTriangleToEQ(int iFaceNum, int iIndex);
    int convertEQToTriangle(int iEQID, int *piFaceNum);
    EQTriangle *getFaceTriangle(int iFaceNum);
    void show();
    void dump(std::string sOut);
protected:
    EQsahedron();
    int init(int iSubDivs, bool bTegmark);

    static double calcSinAngle(); 
    void calcIcoVerts();
    void createFaces();
    
    Quat calcTriangleMapping(const Vec3D &vA1,const Vec3D &vA2,const Vec3D &vA3,
                             const Vec3D &vB1,const Vec3D &vB2,const Vec3D &vB3);

    void mapTriangle(EQTriangle *pEQ, int iFaceNum);

    void makeIcoFaces();

    int findOrientedEdge(int iFaceNum, int iV0, int iV1, bool *pbReversed);

    Vec3D *icoToPlane(int iFace, Vec3D *pV);
    Vec3D *planeToIco(int iFace, Vec3D *pV);

    Vec3D icoToPlane(int iFace, const Vec3D &vV);
    Vec3D planeToIco(int iFace, const Vec3D &vV);
 
    Vec3D *sphereToIco(Vec3D *pV, int *piFace);
 
 
    int     m_iSubDivs;
    bool    m_bTegmark;
    Vec3D *m_apMainVertices[ICOVERTS];
    EQTriangle *m_apEQFaces[ICOFACES];
    IcoFace    **m_apIcoFaces;
    VertexLinkage *m_pVL;
    long m_iNumIcoFaces;
    long m_iCurFace;

    tbox m_curBox;

    IcoFace *m_apMainFaces[ICOFACES];
    transinfo m_tiTrans[ICOFACES];
    Quat m_qGlobal;

    //    float m_fMinAlt;
    bool m_bPartial;

};
#endif
