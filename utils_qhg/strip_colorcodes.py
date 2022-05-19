#!/usr/bin/python

from sys import argv,stdin
## remove text-color codes (i.e. everything between "esc,']'" and 'm'


if (argv):
    for s in stdin:
        
        #print(s.rstrip())
        s1=""
        bInCtrl = False
        i = 0
        while i < len(s):
            if (not bInCtrl):
                # ctrl-'[' (1B 5b) starts control sequence
                if (ord(s[i]) == 27) and (ord(s[i+1])==91):
                    bInCtrl = True
                else:
                    s1 = s1 + s[i]
                #-- end if
            else:
                # 'm' ends the control sequence
                if (ord(s[i]) == 109):
                    bInCtrl = False
                #-- end if
            #-- end if
            i = i+1
        #-- end while
        print(s1.rstrip())
    #-- end for
