#!/usr/bin/python


from sys import argv
import subprocess as sp

def print_arary_short(a, max_els, border_els_l, border_els_r):
    
    if ((border_els_l + border_els_r) < len(a)) and(len(a) > max_els):
        s = "["
        for i in range(border_els_l):
            s +=  str(a[i])+", "
        if len(a) > max_els:
            s += "... "
        for i in range(len(a)-border_els_r, len(a)):
            s += ", " + str(a[i])
        s += "]"
        print(s)
    else:
        print(a)
    # -- end if
# -- end def

if __name__ == "__main__":

    if len(argv) > 1:
        print(argv[1])
        if argv[1].isdigit():
            num_dims = int(argv[1])
            l1 = num_dims *["*"]
            

            for i in range(num_dims):
                l1[i] = "#"
                pat = ":".join(l1)
                
                l1[i] = "*"

                extents = ":".join([str(x) for x in list(range(num_dims+1, 1, -1))])

                command = ["./SubSpaceTest",
                           "-e",
                           "%s"%extents,
                           "-s",
                           "%s"%pat]
                print("comand;\n%s"%" ".join(command))
                proc = sp.Popen(command,
                                stdout=sp.PIPE,
                                stderr=sp.PIPE)
                s_out, s_err = proc.communicate()
                res = proc.returncode
                print("return code: %d"%res)
                line =  s_err.decode(encoding="utf-8")
                a = line.splitlines()
                
                for x in a:
                    sss = str(x)
                    if (sss.find("final") >= 0) or \
                       (sss.find("SliceI") == 0) or \
                       (sss.find("m_vSubV") == 0) or \
                       (sss.find(":") == 0):
                        print(">> %s"%sss)
                

