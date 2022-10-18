#ifndef __POPREADER_H__
#define __POPREADER_H__

#include <vector>
#include <string>
#include <hdf5.h>
#include "types.h"

const int NAMESIZE = 128;

class PopBase;
typedef struct {
    std::string m_sClassName;
    std::string m_sSpeciesName;
    int  m_iNumCells;
} popinfo;

typedef std::vector<popinfo> popinfolist;

const int POP_READER_ERR_NO_POP_GROUP      = -1;
const int POP_READER_ERR_CELL_MISMATCH     = -2;
const int POP_READER_ERR_READ_SPECIES_DATA = -3;
const int POP_READER_ERR_NO_SPECIES_GROUP  = -4;
class PopReader {
public:

    static PopReader *create(const std::string sFilename);
    static PopReader *create(hid_t hFile);
    ~PopReader();
    
    int open(const std::string sFilename);
    int open(hid_t hFile);
    const popinfolist &getPopList() { return m_vPopList;};
    int read(PopBase *pPB, const std::string sSpeciesName, int iNumCells, bool bRestore);

protected:
    PopReader();
   
    popinfolist m_vPopList;
    hid_t       m_hFile;

    
    hid_t       m_hPopGroup;
    hid_t       m_hSpeciesGroup;
   
    bool        m_bOpenedFile;
};


#endif
