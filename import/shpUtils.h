#ifndef __SHPUTILS_H__
#define __SHPUTILS_H__

#ifndef uchar
typedef unsigned char uchar;
#endif
#ifndef uint
typedef unsigned int  uint;
#endif

const bool BIGENDIAN    = true;
const bool LITTLEENDIAN = false;


const int SHP_NULL        = 0;
const int SHP_POINT       = 1;
const int SHP_POLYLINE    = 3;
const int SHP_POLYGON     = 5;
const int SHP_MULTIPOINT  = 8;
const int SHP_POINTZ      = 11;
const int SHP_POLYLINEZ   = 13;
const int SHP_POLYGONZ    = 15;
const int SHP_MULTIPOINTZ = 18;
const int SHP_POINTM      = 21;
const int SHP_POLYLINEM   = 23;
const int SHP_POLYGONM    = 25;
const int SHP_MULTIPOINTM = 28;
const int SHP_MULTIPATCH  = 31;

typedef struct numname {
    int         iNum;
    const char *pName;
} numname;

typedef struct mbr {
    double dXmin;
    double dYmin;
    double dXmax;
    double dYmax;
    mbr():dXmin(0),dYmin(0),dXmax(0),dYmax(0){};
} mbr;

class shpUtils {
public:
    static uchar *getNum(uchar *p, short *piNum, bool bBigEndian);
    static uchar *getNum(uchar *p, int *piNum, bool bBigEndian);
    static uchar *getNum(uchar *p, double *pdNum);
    static int toLittleEndian(short iNum);
    static int toLittleEndian(int iNum);
    static const char *getShapeName(int iShapeNum);
   
    static uchar *getMBR(uchar *p, mbr mMBR);
 
};

#endif
