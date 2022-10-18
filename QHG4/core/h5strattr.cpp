/************************************************************

  This example shows how to read and write string datatypes
  to an attribute.  The program first writes strings to an
  attribute with a dataspace of DIM0, then closes the file.
  Next, it reopens the file, reads back the data, and
  outputs it to the screen.

  This file is intended for use with HDF5 Library version 1.8

 ************************************************************/

#include "hdf5.h"
#include <cstdio>
#include <cstdlib>

#include "xPopBase.h"
#include "xPopLooper.h"
#include "OccChecker.h"

#define NUM_CELLS 5
#define NUM_TIMES 6
int gaga1[NUM_TIMES][NUM_CELLS] = {
    {1,0,0,0,0},
    {1,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0}
};

int nunu1[NUM_TIMES][NUM_CELLS] = {
    {0,0,0,0,1},
    {0,1,0,1,1},
    {0,0,0,1,1},
    {0,0,1,1,1},
    {0,1,1,1,1},
    {1,1,1,1,1}
};

int rere1[NUM_TIMES][NUM_CELLS] = {
    {0,0,1,0,0},
    {0,0,1,1,0},
    {0,1,1,0,1},
    {0,1,0,0,0},
    {1,0,0,0,0},
    {0,0,0,0,0}
};


uchar *makeOcc(std::vector<std::string> &vPopNames) {
    int iResult = 0;
    uchar *p1 = NULL;

    int iNumCells = NUM_CELLS;
    xPopLooper *pPL = new xPopLooper;
    xPopBase *pGaga = new xPopBase("gaga", iNumCells);

    pPL->addPop(pGaga);
    vPopNames.push_back("gaga");
    xPopBase *pNunu = new xPopBase("nunu", iNumCells);
    pPL->addPop(pNunu);
    vPopNames.push_back("nunu");
    xPopBase *pRere = new xPopBase("rere", iNumCells);
    pPL->addPop(pRere);;
    vPopNames.push_back("rere");

    std::vector<int> vCellIDs;
    for (int i = 0; i < iNumCells; i++) {
        vCellIDs.push_back(i);
    }
    OccTracker *pOT = OccTracker::createInstance(vCellIDs, pPL);
    if (pOT != NULL) {
        
        for (int i = 0; i < NUM_TIMES; i++) {
            pGaga->setNumAgents(gaga1[i], iNumCells);
            pNunu->setNumAgents(nunu1[i], iNumCells);
            pRere->setNumAgents(rere1[i], iNumCells);
            
            pOT->updateCounts(i);
        }
        p1 = pOT->serialize();
    }
    delete pRere;
    delete pNunu;
    delete pGaga;
    delete pOT;
    return p1;
}

#define FILE            "h5ex_t_stringatt.h5"
#define DATASET         "DS1"
#define ATTRIBUTE       "A1"
#define DIM0            4
#define SDIM            8

int
main (void)
{
    hid_t       file, filetype, memtype, space, dset, attr;
                                            /* Handles */
    herr_t      status;
    hsize_t     dims[1] = {DIM0};
    size_t      sdim;
    char        wdata[DIM0][SDIM] = {"Parting", "is such", "sweet", "sorrow."},
                                            /* Write buffer */
                **rdata;                    /* Read buffer */
    int         ndims, i;

    std::vector<std::string> &vPopNames;
    uchar *p = makeOcc(vPopNames);
    
    


    /*
     * Create a new file using the default properties.
     */
    file = H5Fcreate (FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /*
     * Create file and memory datatypes.  For this example we will save
     * the strings as FORTRAN strings, therefore they do not need space
     * for the null terminator in the file.
     */
    filetype = H5Tcopy (H5T_FORTRAN_S1);
    status = H5Tset_size (filetype, SDIM - 1);
    memtype = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (memtype, SDIM);

    /*
     * Create dataset with a null dataspace.
     */
    space = H5Screate (H5S_NULL);
    dset = H5Dcreate (file, DATASET, H5T_STD_I32LE, space, H5P_DEFAULT,
                H5P_DEFAULT, H5P_DEFAULT);
    status = H5Sclose (space);

    /*
     * Create dataspace.  Setting maximum size to NULL sets the maximum
     * size to be the current size.
     */
    space = H5Screate_simple (1, dims, NULL);

    /*
     * Create the attribute and write the string data to it.
     */
    attr = H5Acreate (dset, ATTRIBUTE, filetype, space, H5P_DEFAULT,
                H5P_DEFAULT);
    status = H5Awrite (attr, memtype, wdata[0]);

    /*
     * Close and release resources.
     */
    status = H5Aclose (attr);
    status = H5Dclose (dset);
    status = H5Sclose (space);
    status = H5Tclose (filetype);
    status = H5Tclose (memtype);
    status = H5Fclose (file);


    /*
     * Now we begin the read section of this example.  Here we assume
     * the attribute and string have the same name and rank, but can
     * have any size.  Therefore we must allocate a new array to read
     * in data using malloc().
     */

    /*
     * Open file, dataset, and attribute.
     */
    file = H5Fopen (FILE, H5F_ACC_RDONLY, H5P_DEFAULT);
    dset = H5Dopen (file, DATASET, H5P_DEFAULT);
    attr = H5Aopen (dset, ATTRIBUTE, H5P_DEFAULT);

    /*
     * Get the datatype and its size.
     */
    filetype = H5Aget_type (attr);
    sdim = H5Tget_size (filetype);
    sdim++;                         /* Make room for null terminator */

    /*
     * Get dataspace and allocate memory for read buffer.  This is a
     * two dimensional attribute so the dynamic allocation must be done
     * in steps.
     */
    space = H5Aget_space (attr);
    ndims = H5Sget_simple_extent_dims (space, dims, NULL);

    /*
     * Allocate array of pointers to rows.
     */
    rdata = (char **) malloc (dims[0] * sizeof (char *));

    /*
     * Allocate space for integer data.
     */
    rdata[0] = (char *) malloc (dims[0] * sdim * sizeof (char));

    /*
     * Set the rest of the pointers to rows to the correct addresses.
     */
    for (i=1; i<dims[0]; i++)
        rdata[i] = rdata[0] + i * sdim;

    /*
     * Create the memory datatype.
     */
    memtype = H5Tcopy (H5T_C_S1);
    status = H5Tset_size (memtype, sdim);

    /*
     * Read the data.
     */
    status = H5Aread (attr, memtype, rdata[0]);

    /*
     * Output the data to the screen.
     */
    for (i=0; i<dims[0]; i++)
        printf ("%s[%d]: %s\n", ATTRIBUTE, i, rdata[i]);

    /*
     * Close and release resources.
     */
    free (rdata[0]);
    free (rdata);
    status = H5Aclose (attr);
    status = H5Dclose (dset);
    status = H5Sclose (space);
    status = H5Tclose (filetype);
    status = H5Tclose (memtype);
    status = H5Fclose (file);

    return 0;
}
