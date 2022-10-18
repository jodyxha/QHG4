/*============================================================================
| EventConsts
| 
|  Various constants for events and their parameters
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __EVENTCONSTS_H__
#define __EVENTCONSTS_H__

#include <string>

const int EVENT_ID_NONE     = -1;
const int EVENT_ID_WRITE    =  1;

const int EVENT_ID_GEO      =  2;
const int EVENT_ID_CLIMATE  =  3;
const int EVENT_ID_VEG      =  4;
const int EVENT_ID_NAV      =  5;
const int EVENT_ID_OCC      =  6;

const int EVENT_ID_ENV      =  7;
const int EVENT_ID_ARR      =  8;
const int EVENT_ID_POP      =  9;
const int EVENT_ID_COMM     = 10;
const int EVENT_ID_CHECK    = 11;
const int EVENT_ID_DUMP     = 12;
const int EVENT_ID_INTERPOL = 13;
const int EVENT_ID_SCRAMBLE = 14;

const int EVENT_ID_FLUSH    = 20;
const int EVENT_ID_USER     = 21;

const int EVENT_ID_USR_MIN  = 1000;
const int EVENT_ID_USR_MAX  = 9999;

const std::string EVENT_TYPE_WRITE          = "write";
/*
const std::string EVENT_TYPE_GEO            = "geo";
const std::string EVENT_TYPE_CLIMATE        = "climate";
const std::string EVENT_TYPE_VEG            = "veg";
const std::string EVENT_TYPE_NAV            = "nav";
*/

const std::string EVENT_TYPE_ENV            = "env";
const std::string EVENT_TYPE_ARR            = "arr";
const std::string EVENT_TYPE_POP            = "pop";
const std::string EVENT_TYPE_COMM           = "comm";
const std::string EVENT_TYPE_FILE           = "file";
const std::string EVENT_TYPE_CHECK          = "check";
const std::string EVENT_TYPE_DUMP           = "dump";
const std::string EVENT_TYPE_USER           = "user";
const std::string EVENT_TYPE_INTERPOL       = "interpol";
const std::string EVENT_TYPE_SCRAMBLE       = "scramble";

const std::string EVENT_PARAM_NAME_GEO      = "geo";
const std::string EVENT_PARAM_NAME_CLIMATE  = "climate";
const std::string EVENT_PARAM_NAME_VEG      = "veg";
const std::string EVENT_PARAM_NAME_NAV      = "nav";
const std::string EVENT_PARAM_NAME_OCC      = "occ";
const std::string EVENT_PARAM_NAME_ALL      = "all";

const std::string EVENT_PARAM_SCRAMBLE_CONN = "connections";
const std::string EVENT_PARAM_SCRAMBLE_ALL  = "all";

const std::string EVENT_PARAM_WRITE_GRID    = "grid";
const std::string EVENT_PARAM_WRITE_GEO     = "geo";
const std::string EVENT_PARAM_WRITE_CLIMATE = "climate";
const std::string EVENT_PARAM_WRITE_VEG     = "veg";
const std::string EVENT_PARAM_WRITE_POP     = "pop";
const std::string EVENT_PARAM_WRITE_STATS   = "stats";
const std::string EVENT_PARAM_WRITE_NAV     = "nav";
const std::string EVENT_PARAM_WRITE_ENV     = "env";
const std::string EVENT_PARAM_WRITE_OCC     = "occ";

/*
const std::string EVENT_PARAM_GEO_SEA       = "sea";
const std::string EVENT_PARAM_GEO_ALT       = "alt";
const std::string EVENT_PARAM_GEO_ICE       = "ice";
const std::string EVENT_PARAM_GEO_WATER     = "water";
*/

const std::string EVENT_PARAM_GEO_QDF       = "qdf";
const std::string EVENT_PARAM_CLIMATE_QDF   = "qdf";
const std::string EVENT_PARAM_VEG_QDF       = "qdf";
const std::string EVENT_PARAM_NAV_QDF       = "qdf";

const std::string EVENT_PARAM_CHECK_LISTS   = "lists";

const std::string EVENT_PARAM_INTERPOL_FILE  = "file";
const std::string EVENT_PARAM_INTERPOL_CMD   = "cmd";
const std::string EVENT_PARAM_INTERPOL_START = "start";
const std::string EVENT_PARAM_INTERPOL_STOP  = "stop";


const std::string CMD_SET_ITERS             = "SET ITERS";
const std::string CMD_REMOVE_ACTION         = "REMOVE ACTION";
const std::string CMD_MOD_POP               = "MOD POP";
const std::string CMD_FREEZE                = "FREEZE";


bool hasInputFile(int iEventType);
bool hasInputFile(char *pEventType);


#endif
