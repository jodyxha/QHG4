#include <cstdio>
#include <cstring>

#include <hdf5.h>

#include "stdstrutils.h"
#include "stdstrutilsT.h"
#include "MessLogger.h"
#include "ExecuteCommand.h"
#include "SCellGrid.h"
#include "Geography.h"
#include "Climate.h"
#include "Vegetation.h"

#include "GridFactory.h"
#include "StatusWriter.h"


//-----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - convert a grid definition file to QDF file\n", pApp);
    printf("usage:\n");
    printf("  %s \"<grid_def_line>\" [<specifiers>] <output-QDF-name>\n", pApp);
    printf("or\n");
    printf("  %s <grid-def-file> [<specifiers>] <output-QDF-name>\n", pApp);
    printf("where\n");
    printf("  grid_def_line    commands to build grid and add environment (':'-separated list of commands)\n");
    printf("                   format: see below under \"grid_def_line\"\n");
    printf("  grid-def-file    name of grid definition file (one command per line)\n");
    printf("                   format: see below under \"grid_def_file\"\n");
    printf("  output-QDF-name  name for output QDF file\n");
    printf("  specifiers       codes for groups to add:\n");
    printf("                     S : grid data\n");
    printf("                     G : geography data\n");
    printf("                     C : climate data\n");
    printf("                     V : vegetation data\n");
    printf("                     N : empty naviigation data\n");
    printf("                   if ignored, all groups are added.\n");
    printf("\n");
    printf("Grid file/line format:\n");
    printf("  grid_def_file ::= <grid_cmd>[<NL> <def_cmd>]*\n");
    printf("  grid_def_line ::= <grid_cmd>[:<def_cmd>]*\n");
    printf("  grid_cmd      ::= <ico_cmd> | <flat_cmd>\n");
    printf("  ico_cmd       ::= \"GRID_TYPE ICO\" (<type>:<subdivs> | <ignfile>) [<radius>] \n");
    printf("    subdivs     :   number of subdivisions\n");
    printf("    type        ::= \"eq\" | \"std\"\n");
    printf("    ignfile     :   name of ico gridnode file\n");
    printf("    radius      :   radius of icosahedron, default:6371.3\n");
    printf("  flat_cmd      ::= \"GRID_TYPE\" <conn> <width>\"x\"<height> [\"PERIODIC\"]\n");
    printf("    conn        ::= \"HEX\" | \"RECT\"\n");
    printf("    width       :   width of grid\n");
    printf("    height      :   height of grid\n");
    printf("    PERIODIC    :   use periodic boundary conditions\n");
    printf("  def_cmd ::= <ddir_cmd> | <env_cmd>\n");
    printf("  ddir_cmd ::= \"DATA_DIR\" <path>[:<path>]*\n");
    printf("  env_cmd ::= <env_type> \"FLAT\" <value> | <env_type> \"NETCDF\"  (<nc_file> | \"DEFAULT\") <time>  | <env_type> \"QMAP\"  <valqmap>\n");
    printf("  env_type ::= \"ALT\"  | \"ICE\" | \"NPP\" | \"TEMP\" | \"RAIN\"\n");
    printf("\n");
    printf("Notes:\n");
    printf("  - the \"grid_def_line parameter should be enclosed in quotes if the first version of the call is used\n");
    printf("  - the \"type\" parameter in the ico-command specifies whether to equalize the triangle areas (\"eq\") or not (\"std\")\n");
    printf("  - the paths specified in DATA_DIR are search paths for applications and data\n");
    printf("  - if \"DEFAULT\" is specified in any of the env-commands, the files used by prepare_grids will be used.\n");
    printf("\n");
    printf("Example:\n");
    printf("  ./Def2QDF \"GRID_TYPE ICO eq:3\" SG blob.qdf\n"); 
}

//-----------------------------------------------------------------------------
// parseSpecs
//   the specs describe whiich environment groups to aadd to the QDF file
//
int parseSpecs(char *pSpecs) {
    int iSpecs = WR_NONE;
    char *p = pSpecs;
    bool bOK = true;
    while (bOK && (*p != '\0')) {
        switch (*p) {
        case 'S':
            iSpecs |= WR_GRID;
            break;
        case 'G':
            iSpecs |= WR_GEO;
            break;
        case 'C':
            iSpecs |= WR_CLI;
            break;
        case 'V':
            iSpecs |= WR_VEG;
            break;
        case 'N':
            iSpecs |= WR_NAV;
            break;
        default:
            printf("Unknown specifier: [%c]\n", *p);
            bOK = false;
            iSpecs = WR_NONE;
        }
        p++;
    }
    return iSpecs;
}

//-----------------------------------------------------------------------------
// createGridFactory
//
GridFactory *createGridFactory(const std::string sInput) {
    int iResult = 0;
    GridFactory *pGF = new GridFactory(sInput);
    if (pGF->isReady()) {
        iResult = pGF->readDef();
        if (iResult == 0) {
            stdprintf("Successfully read\n");
            if (pGF->getCellGrid() != NULL) {
                stdprintf("  - grid data\n");
            }
            if (pGF->getGeography() != NULL) {
                stdprintf("  - geo data\n");
            }
            if (pGF->getClimate() != NULL) {
                pGF->getCellGrid()->setClimate(pGF->getClimate());
                stdprintf("  - climate data\n");
            }
            if (pGF->getVeg() != NULL) {
                pGF->getCellGrid()->setVegetation(pGF->getVeg());
                stdprintf("  - vegetation data\n");
            }
            if (pGF->isFromFile()) {
                stdprintf("from def file [%s]\n", sInput);
            } else {
                stdprintf("from input string\n");
            }
        } else {
            stdprintf("Couldn't read definintion file [%s]\n", sInput);
            iResult = -1;
        }
    } else {
        stdprintf("Couldn't open [%s] for reading\n", sInput);
        iResult = -1;
    }
    if (iResult != 0) {
        delete pGF;
        pGF = NULL;
    }
    return pGF;
}

//-----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    MessLogger::create("def2qdf_log");
    if (iArgC > 2) {
        char *pDef = apArgV[1];
        char *pOutput = apArgV[2];
        int iSpecs = WR_ALL;
        
        if (iArgC > 3) {
            pOutput = apArgV[3];
            iSpecs = parseSpecs(apArgV[2]);
        }
        if (iSpecs != WR_NONE) {
            GridFactory *pGF = createGridFactory(pDef);
            if (pGF != NULL) {
                float fNoTime = -1.0;
                std::vector<PopBase *> vDummy;
                StatusWriter *pSW = StatusWriter::createInstance(pGF->getCellGrid(), vDummy);
                if (pSW != NULL) {
                    iResult = pSW->write(pOutput, -1, fNoTime, "Def2QDF", iSpecs, -1);
                    if (iResult < 0) {
                        printf("Error: %s\n", pSW->getError().c_str());
                    }
 
                    if (iResult >= 0) {
                        /*
                        if (iResult > 0) {
                            printf("couldn't write:\n");
                            if ((iResult & WR_POP) != 0) {
                                printf("- Populations\n");
                            }
                            if ((iResult & WR_GRID) != 0) {
                                printf("- Grid\n");
                            }
                            if ((iResult & WR_GEO) != 0) {
                                printf("- Geography\n");
                            }
                            if ((iResult & WR_CLI) != 0) {
                                printf("- Climate\n");
                            }
                            if ((iResult & WR_VEG) != 0) {
                                printf("- Vegetation\n");
                            }
                            if (iResult < (int)iSpecs) {
                                iResult = 0;
                            }
                        }
                        */                         
                        if (iResult == 0) {
                            uint iNumCommands = pGF->getNumShellCommands();
                            if (iNumCommands > 0) {
                                printf("----- executing %u commands\n", iNumCommands);
                                iResult = pGF->applyShellCommands(pOutput);
                                /*
                                const std::string sPlaceHolder = "##QDF##";
                                for (unsigned int i = 0; i < vCommands.size(); i++) {
                                    std::string sComm(vCommands[i]);
                                    size_t start_pos = sComm.find(sPlaceHolder);
                                    if(start_pos != std::string::npos) {
                                        sComm.replace(start_pos, sPlaceHolder.size(), pOutput);
                                        char *pComm = new char[sComm.size()+1];
                                        strcpy(pComm, sComm.c _str());
                                        printf("Command %02u: [%s]\n", i, pComm);
                                        std::vector<std::string> vLines;
                                        iResult =  executeCommand(pComm, vLines);
                                        if (iResult == 0) {
                                        for (unsigned int i = 0; i < vLines.size(); i++) {
                                            printf("%s\n", vLines[i].c _str());
                                        }
                                        }
                                        delete[] pComm;
                                    }
                                } 
                                */   
                            } else {
                                printf("No commands to execute\n");
                            }
                        }
                        if (iResult == 0) {
                            printf("Successfully exported data to QDF format\n");
                            printf("+++success+++\n");
                        }
                    } else {
                        printf("failed to write\n");
                    }
                    delete pSW;
                } else {
                    printf("Couldn't create status writer\n");
                    iResult = -1;
                }
                delete pGF;
            }
        } else {
            printf("At least one data item must be specified\n");
            iResult = -1;
        }
        
    } else {
        usage(apArgV[0]);
    }

    return iResult;
}
