#ifndef __EQCONNECTIVITY_H__
#define __EQCONNECTIVITY_H__


class EQConnectivity {
public:
    static EQConnectivity *createInstance(int iSubDiv);

    static int calcNumNodes(int iSubDiv);
    static int calcNumEdges(int iSubDiv);
    static int calcNumFaces(int iSubDiv);

    virtual ~EQConnectivity();
    int getNumTriangles() {return m_iNumTriangles;};
    int *getTriangles() { return m_aiTriangles;};
protected:
    EQConnectivity(int iSubDivs);
    int init();

    int createSubdividedTriangle();
    int findOrientedEdge(int iFaceNum, int iV0, int iV1, bool *pbReversed);
    int *setGlobalIDs(int *pCurIndex, int iFaceNum);

    void showBaseTriangle();
    void showTempTriangle();
    int  m_iSubDivs;
    int  m_iNumTriangles;
    int *m_aiTriangles;

    int  m_iNumBaseTriangles;
    int *m_aiBaseTriangles;
    
    int  m_iNumBaseNodes;
    int *m_aiTempIDs;
};


#endif
