#include <cstdio>
#include <hdf5.h>

#include "QDFUtils.h"
#include "QDFUtilsT.h"


#define SPOP_DT_LIFE_STATE "LifeState"
#define SPOP_DT_CELL_INDEX "CellIdx"
#define SPOP_DT_CELL_ID    "CellID"
#define SPOP_DT_AGENT_ID   "AgentID"
#define SPOP_DT_GENDER     "Gender"
#define SPOP_DT_BIRTH_TIME "BirthTime"
#define SPOP_DT_MASS       "Mass"

const char *errmess[] = {"no error",
                 "unknown error",
                 "different number of members",
                 "different offsets",
                 "different class",
                 "different type",
                 "different name",};

struct Agent0 {
    int      m_iInt;
};

struct Agent1 {
    uint     m_iLifeState;
    int      m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;
};

struct Agent2 {
    uint     m_iLifeState;
    int      m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;
};

struct Agent3 {
    int      m_iCellIndex;
    idtype   m_ulID;
    float    m_fBirthTime;
    uint     m_iLifeState;
    gridtype m_ulCellID;
    uchar    m_iGender;
};

struct Agent4 {
    uint     m_iLifeState;
    int      m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;
    double   m_dMass;
};

struct Agent6 {
    uint     m_iLifeState;
    float    m_iCellIndex;
    idtype   m_ulID;
    gridtype m_ulCellID;
    float    m_fBirthTime;
    uchar    m_iGender;
    double   m_dMass;
};


int test1() {
    int iResult = 0;

    hid_t ahAgentDataType[6];
    hid_t hAgentDataType = H5P_DEFAULT;

    Agent1 ta1;
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta1));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta1, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta1, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta1, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta1, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta1, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta1, m_iGender),    H5T_NATIVE_UCHAR);
    ahAgentDataType[0] = hAgentDataType;
 

    Agent2 ta2;
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta2));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta2, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta2, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta2, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta2, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta2, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta2, m_iGender),    H5T_NATIVE_UCHAR);
    ahAgentDataType[1] = hAgentDataType;

    Agent3 ta3;
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta3));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta3, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta3, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta3, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta3, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta3, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta3, m_iGender),    H5T_NATIVE_UCHAR);
    ahAgentDataType[2] = hAgentDataType;

    Agent4 ta4;
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta4));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta4, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta4, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta4, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta4, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta4, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta4, m_iGender),    H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, SPOP_DT_MASS,        qoffsetof(ta4, m_dMass),      H5T_NATIVE_DOUBLE);
    ahAgentDataType[3] = hAgentDataType;



    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta1));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta1, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta1, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta1, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "gurgeli",           qoffsetof(ta1, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta1, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta1, m_iGender),    H5T_NATIVE_UCHAR);
    ahAgentDataType[4] = hAgentDataType;
    

    Agent6 ta6;
    hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta6));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta6, m_iLifeState), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta6, m_iCellIndex), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta6, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "gurgeli",           qoffsetof(ta6, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta6, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta6, m_iGender),    H5T_NATIVE_UCHAR);
    ahAgentDataType[5] = hAgentDataType;
    printf("types:\n");
    for (int i = 0; i < 6; i++) {
        printf("%d: %016lx\n",  0, H5Tget_member_type(ahAgentDataType[i], 0));
    }
    for (int i = 0; i < 6; i++) {
        printf("------------\n");
        for (int j = i; j < 6; j++) {
            printf("%d - %d : ", i, j);
            iResult = qdf_compareDataTypes(ahAgentDataType[i], ahAgentDataType[j]);
            if (iResult >= 0) {
                printf("equal\n");
            } else {
                printf("%s (%d)\n", errmess[-iResult], iResult);
            }
        }
    }
    printf("------------\n");

    for (int i = 0; i < 6; i++) {
        H5Tclose(ahAgentDataType[i]);
    }
    return iResult;
}

void showH5TConsts() {
    printf("H5T_NATIVE_INT:   %016lx\n", H5T_NATIVE_INT);
    printf("H5T_NATIVE_FLOAT: %016lx\n", H5T_NATIVE_FLOAT);
    printf("H5T_NATIVE_LONG:  %016lx\n", H5T_NATIVE_LONG);
    printf("H5T_NATIVE_UCHAR: %016lx\n", H5T_NATIVE_UCHAR);
} 

int test2() {
    /*
    struct Agent0 {
        int      m_iInt;
    } ta0;
    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta0));
    H5Tinsert(hAgentDataType, "my_int",       qoffsetof(ta0, m_iInt),    H5T_NATIVE_INT);

    */
    Agent4 ta4;
    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta4));

    H5Tinsert(hAgentDataType, SPOP_DT_LIFE_STATE,  qoffsetof(ta4, m_iLifeState), H5T_NATIVE_UINT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_INDEX,  qoffsetof(ta4, m_iCellIndex), H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_CELL_ID,     qoffsetof(ta4, m_ulCellID),   H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, SPOP_DT_AGENT_ID,    qoffsetof(ta4, m_ulID),       H5T_NATIVE_LONG);
    H5Tinsert(hAgentDataType, SPOP_DT_BIRTH_TIME,  qoffsetof(ta4, m_fBirthTime), H5T_NATIVE_FLOAT);
    H5Tinsert(hAgentDataType, SPOP_DT_GENDER,      qoffsetof(ta4, m_iGender),    H5T_NATIVE_UCHAR);
    H5Tinsert(hAgentDataType, SPOP_DT_MASS,        qoffsetof(ta4, m_dMass),      H5T_NATIVE_DOUBLE);



    printf("Number of members: %d\n", H5Tget_nmembers(hAgentDataType));
    for (int i = 0; i <  H5Tget_nmembers(hAgentDataType); i++) {
        printf("--------\n");
        printf("name   of member %d: %s\n",  i, H5Tget_member_name(hAgentDataType, i));
        printf("offset of member %d: %zd\n", i, H5Tget_member_offset(hAgentDataType, i));
        printf("class  of member %d: %d\n",  i, H5Tget_member_class(hAgentDataType, i));
        printf("type   of member %d: %016lx\n",  i, H5Tget_member_type(hAgentDataType, i));
    }

   
    
    return 0;

}

int test3() {
    struct Agent0 {
        int      m_iInt1;
        int      m_iInt2;
    } ta0;
    hid_t hAgentDataType = H5Tcreate (H5T_COMPOUND, sizeof(ta0));
    H5Tinsert(hAgentDataType, "my_int1",       qoffsetof(ta0, m_iInt1),    H5T_NATIVE_INT);
    H5Tinsert(hAgentDataType, "my_int2",       qoffsetof(ta0, m_iInt2),    H5T_NATIVE_INT);

    printf("type   of member %d: %016lx\n",  0, H5Tget_member_type(hAgentDataType, 0));
    printf("type   of member %d: %016lx\n",  1, H5Tget_member_type(hAgentDataType, 1));
    

    printf("again  of member %d: %016lx\n",  0, H5Tget_member_type(hAgentDataType, 0));

    printf("H5T_NATIVE_INT:     %016lx\n", H5T_NATIVE_INT);
    
    printf("call #1 for member %d: %016lx\n",  0, H5Tget_member_type(hAgentDataType, 0));
   

    printf("call #2 for member %d: %016lx\n",  0, H5Tget_member_type(hAgentDataType, 0));



    return 0;

}

int main(int iArgC, char *apArgV[]) {
    test1();
    printf("+++++++++++++++++++++++++++++++\n");
    showH5TConsts();
    printf("+++++++++++++++++++++++++++++++\n");
    test2();
    printf("+++++++++++++++++++++++++++++++\n");
    test3();
    return 0;
}
