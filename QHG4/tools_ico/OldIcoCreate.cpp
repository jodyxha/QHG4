#include <cstdio>
#include <cstdlib>
#include <map>

#include "types.h"
#include "geomutils.h"
#include "VertexLinkage.h"
#include "IcoNode.h"
#include "IcoGridNodes.h"
#include "Icosahedron.h"

//-----------------------------------------------------------------------------
// createEQGrid
//    pVL = EQsahedron::getLinkage()
//
IcoGridNodes *createEQGrid(VertexLinkage *pVL) {
    
    IcoGridNodes *pIGN = new IcoGridNodes();
    
    std::map<gridtype, Vec3D *>::const_iterator iti;
    for (iti = pVL->m_mI2V.begin(); iti != pVL->m_mI2V.end(); iti++) {
        Vec3D *pV = iti->second;

        // calc theta and phi
        double dPhi = 0;
        double dTheta = 0;
        if (pV != NULL) {
            double dArea = 0; // get it from a vertexlinkage thingy
            cart2Sphere(pV, &dTheta, &dPhi);
            IcoNode *pIN = new IcoNode(iti->first, dTheta, dPhi, dArea);
            intset vLinks = pVL->getLinksFor(iti->first);
            if (vLinks.size() > 6) {
                intset::iterator st;
                printf("Vertex #%d (%f,%f) has %zd links\n", iti->first, pV->m_fX, pV->m_fY,vLinks.size());
                for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                    printf("  %d", *st);
                }
                printf("\n");
            }
            
                    
            intset::iterator st;
            for (st = vLinks.begin(); st != vLinks.end(); ++st) {
                Vec3D *pN = pVL->getVertex(*st);
                double dDist = spherdist(pV, pN, 1.0);
                pIN->m_dArea = pVL->calcArea(pIN->m_lID);

                pIN->addLink(*st, dDist);

            }
            pIGN->m_mNodes[iti->first] = pIN;
        } else {
            pIGN = NULL;
        }
    }

    return pIGN;
}

int main (int iArgC, char *apArgV[]) {
    int iResult = -1;

    
    int iSubDiv = 0;
    
    iSubDiv = atoi(apArgV[1]);
    Icosahedron *pIco = Icosahedron::create(1, 0);
    pIco->subdivide(iSubDiv);
    pIco->relink();
    pIco->save(apArgV[2]);
    IcoGridNodes *pIGN = createEQGrid(pIco->getLinkage());
    stringmap sm ={{"type","oldico"}};
    pIGN->write(apArgV[2], 6, false, sm); 

    return iResult;
}
