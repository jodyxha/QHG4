#!/usr/bin/python

# creates nicely indented version of the output of
#   h5dump -n 1 <qdf_file>
#
import sys
import subprocess as sp

#-----------------------------------------------------------------------------
# format_line
#
def format_line(line):
    output = line
    entries = line.split()
    if (len(entries) == 2) and (entries[1] != '{'):
        if entries[1] == '/':
            c = 1
        else:
            c = 1+entries[1].count('/')
            #--end if
        output = "%s%-16s%s"%(2*c*" ", entries[0], entries[1])
        #-- end if
    return output;
#-- end def

    
#-----------------------------------------------------------------------------
# handle_stdin
#
def handle_stdin():
    for line in sys.stdin:
        sys.stdout.write("%s\n"%format_line(line.strip())) 
    #-- end for
#-- end def



#-----------------------------------------------------------------------------
# main
#
if (len(sys.argv) > 1):
    fOut = sys.stdout
    command = ["h5dump", "-n", "1", sys.argv[1]]

    proc = sp.Popen(command,
                    stdout=sp.PIPE,
                    stderr=sp.PIPE)
    sOut, sError = proc.communicate()
    iResult = proc.returncode

    if (iResult == 0):
        lines = sOut.decode().split('\n')
        for line in lines:
            sys.stdout.write("%s\n"%format_line(line.strip())) 
        #-- end for
    else:
        print("Erros:%s"%sError)
    #-- end if
else:
    handle_stdin()
#-- end if
