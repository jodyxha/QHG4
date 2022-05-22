#============================================================================
# colors
# 
#  Constants for terminal colors.
# 
#  Author: Jody Weissmann
#===========================================================================*/ 

# text colors
# regular
BLACK        = "\x1b[0;30m"   # Black
RED          = "\x1b[0;31m"   # Red
GREEN        = "\x1b[0;32m"   # Green
YELLOW       = "\x1b[0;33m"   # Yellow
BLUE         = "\x1b[0;34m"   # Blue
PURPLE       = "\x1b[0;35m"   # Purple
CYAN         = "\x1b[0;36m"   # Cyan
WHITE        = "\x1b[0;37m"   # White

# bold
BOLDBLACK    = "\x1b[1;30m"   # Black
BOLDRED      = "\x1b[1;31m"   # Red
BOLDGREEN    = "\x1b[1;32m"   # Green
BOLDYELLOW   = "\x1b[1;33m"   # Yellow
BOLDBLUE     = "\x1b[1;34m"   # Blue
BOLDPURPLE   = "\x1b[1;35m"   # Purple
BOLDCYAN     = "\x1b[1;36m"   # Cyan
BOLDWHITE    = "\x1b[1;37m"   # White

# underline
UNDBLACK     = "\x1b[4;30m"   # Black
UNDRED       = "\x1b[4;31m"   # Red
UNDGREEN     = "\x1b[4;32m"   # Green
UNDYELLOW    = "\x1b[4;33m"   # Yellow
UNDBLUE      = "\x1b[4;34m"   # Blue
UNDPURPLE    = "\x1b[4;35m"   # Purple
UNDCYAN      = "\x1b[4;36m"   # Cyan
UNDWHITE     = "\x1b[4;37m"   # White

# background
BGBLACK      = "\x1b[0;40m"   # Black
BGRED        = "\x1b[0;41m"   # Red
BGGREEN      = "\x1b[0;42m"   # Green
BGYELLOW     = "\x1b[0;43m"   # Yellow
BGBLUE       = "\x1b[0;44m"   # Blue
BGPURPLE     = "\x1b[0;45m"   # Purple
BGCYAN       = "\x1b[0;46m"   # Cyan
BGWHITE      = "\x1b[0;47m"   # White

# background with readable text
TBGBLACK      = "\x1b[0;40;1;37m"   # Black
TBGRED        = "\x1b[0;41;1;37m"   # Red
TBGGREEN      = "\x1b[0;42;1;37m"   # Green
TBGYELLOW     = "\x1b[0;43;1;37m"   # Yellow
TBGBLUE       = "\x1b[0;44;1;37m"   # Blue
TBGPURPLE     = "\x1b[0;45;1;37m"   # Purple
TBGCYAN       = "\x1b[0;46;1;37m"   # Cyan
TBGWHITE      = "\x1b[0;47;1;30m"   # White

# high intensity
HIBLACK      = "\x1b[0;90m"   # Black
HIRED        = "\x1b[0;91m"   # Red
HIGREEN      = "\x1b[0;92m"   # Green
HIYELLOW     = "\x1b[0;93m"   # Yellow
HIBLUE       = "\x1b[0;94m"   # Blue
HIPURPLE     = "\x1b[0;95m"   # Purple
HICYAN       = "\x1b[0;96m"   # Cyan
HIWHITE      = "\x1b[0;97m"   # White

# bold high intensity
BOLDHIBLACK  = "\x1b[1;90m"   # Black
BOLDHIRED    = "\x1b[1;91m"   # Red
BOLDHIGREEN  = "\x1b[1;92m"   # Green
BOLDHIYELLOW = "\x1b[1;93m"   # Yellow
BOLDHIBLUE   = "\x1b[1;94m"   # Blue
BOLDHIPURPLE = "\x1b[1;95m"   # Purple
BOLDHICYAN   = "\x1b[1;96m"   # Cyan
BOLDHIWHITE  = "\x1b[1;97m"   # White

# high intensity backgrounds
BGHIBLACK    = "\x1b[0;100m"  # Black
BGHIRED      = "\x1b[0;101m"  # Red
BGHIGREEN    = "\x1b[0;102m"  # Green
BGHIYELLOW   = "\x1b[0;103m"  # Yellow
BGHIBLUE     = "\x1b[0;104m"  # Blue
BGHIPURPLE   = "\x1b[0;105m"  # Purple
BGHICYAN     = "\x1b[0;106m"  # Cyan
BGHIWHITE    = "\x1b[0;107m"  # White

# high intensity backgrounds with readable text
TBGHIBLACK   = "\x1b[0;100;37m"  # BG Black, T white
TBGHIRED     = "\x1b[0;101;30m"  # Red
TBGHIGREEN   = "\x1b[0;102;30m"  # Green
TBGHIYELLOW  = "\x1b[0;103;30m"  # Yellow
TBGHIBLUE    = "\x1b[0;104;30m"  # Blue
TBGHIPURPLE  = "\x1b[0;105;30m"  # Purple
TBGHICYAN    = "\x1b[0;106;30m"  # Cyan
TBGHIWHITE   = "\x1b[0;107;30m"  # White

BOLD         = "\x1b[1m"
BLINK        = "\x1b[5m"
OFF          = "\x1b[0m"      # Text Reset


INVERTBW     = "\x1b[0;40;97m" 
