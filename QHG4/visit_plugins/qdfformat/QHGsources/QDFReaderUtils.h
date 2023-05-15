#include <hdf5.h>

#include "QDFVisitUtils.h"
#include "SCell.h"

class QDFReaderUtils {

public:
    static int readCellData(hid_t, int, SCell*);
    static hid_t createCellDataType();

};

