/*============================================================================
| colors
| 
|  Constants for terminal colors.
| 
|  Used by MessLogger
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __COLORS_H__
#define __COLORS_H__

namespace colors {
// text colors
// regular
const std::string BLACK        = "\e[0;30m";   // Black
const std::string RED          = "\e[0;31m";   // Red
const std::string GREEN        = "\e[0;32m";   // Green
const std::string YELLOW       = "\e[0;33m";   // Yellow
const std::string BLUE         = "\e[0;34m";   // Blue
const std::string PURPLE       = "\e[0;35m";   // Purple
const std::string CYAN         = "\e[0;36m";   // Cyan
const std::string WHITE        = "\e[0;37m";   // White

// bold
const std::string BOLDBLACK    = "\e[1;30m";   // Black
const std::string BOLDRED      = "\e[1;31m";   // Red
const std::string BOLDGREEN    = "\e[1;32m";   // Green
const std::string BOLDYELLOW   = "\e[1;33m";   // Yellow
const std::string BOLDBLUE     = "\e[1;34m";   // Blue
const std::string BOLDPURPLE   = "\e[1;35m";   // Purple
const std::string BOLDCYAN     = "\e[1;36m";   // Cyan
const std::string BOLDWHITE    = "\e[1;37m";   // White

// underline
const std::string UNDBLACK     = "\e[4;30m";   // Black
const std::string UNDRED       = "\e[4;31m";   // Red
const std::string UNDGREEN     = "\e[4;32m";   // Green
const std::string UNDYELLOW    = "\e[4;33m";   // Yellow
const std::string UNDBLUE      = "\e[4;34m";   // Blue
const std::string UNDPURPLE    = "\e[4;35m";   // Purple
const std::string UNDCYAN      = "\e[4;36m";   // Cyan
const std::string UNDWHITE     = "\e[4;37m";   // White

// background
const std::string BGBLACK      = "\e[0;40m";   // Black
const std::string BGRED        = "\e[0;41m";   // Red
const std::string BGGREEN      = "\e[0;42m";   // Green
const std::string BGYELLOW     = "\e[0;43m";   // Yellow
const std::string BGBLUE       = "\e[0;44m";   // Blue
const std::string BGPURPLE     = "\e[0;45m";   // Purple
const std::string BGCYAN       = "\e[0;46m";   // Cyan
const std::string BGWHITE      = "\e[0;47m";   // White

// high intensity
const std::string HIBLACK      = "\e[0;90m";   // Black
const std::string HIRED        = "\e[0;91m";   // Red
const std::string HIGREEN      = "\e[0;92m";   // Green
const std::string HIYELLOW     = "\e[0;93m";   // Yellow
const std::string HIBLUE       = "\e[0;94m";   // Blue
const std::string HIPURPLE     = "\e[0;95m";   // Purple
const std::string HICYAN       = "\e[0;96m";   // Cyan
const std::string HIWHITE      = "\e[0;97m";   // White

// bold high intensity
const std::string BOLDHIBLACK  = "\e[1;90m";   // Black
const std::string BOLDHIRED    = "\e[1;91m";   // Red
const std::string BOLDHIGREEN  = "\e[1;92m";   // Green
const std::string BOLDHIYELLOW = "\e[1;93m";   // Yellow
const std::string BOLDHIBLUE   = "\e[1;94m";   // Blue
const std::string BOLDHIPURPLE = "\e[1;95m";   // Purple
const std::string BOLDHICYAN   = "\e[1;96m";   // Cyan
const std::string BOLDHIWHITE  = "\e[1;97m";   // White

// high intensity backgrounds
const std::string BGHIBLACK    = "\e[0;100m";  // Black
const std::string BGHIRED      = "\e[0;101m";  // Red
const std::string BGHIGREEN    = "\e[0;102m";  // Green
const std::string BGHISYELLOW  = "\e[0;103m";  // Yellow
const std::string BGHIBLUE     = "\e[0;104m";  // Blue
const std::string BGHIPURPLE   = "\e[0;105m";  // Purple
const std::string BGHICYAN     = "\e[0;106m";  // Cyan
const std::string BGHIWHITE    = "\e[0;107m";  // White

const std::string BOLD         = "\e[1m";
const std::string BLINK        = "\e[5m";
const std::string OFF          = "\e[0m";      // Text Reset
};

#endif
