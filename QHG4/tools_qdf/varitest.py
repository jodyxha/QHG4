#!/usr/bin/python

#-- some experiments for the hilighting of comp_attrs.py

import random
import colors

arr_size   = 12
num_arrays = 10
num_vals   = 8

varcols=[colors.OFF,
         colors.TBGHIRED,
         colors.TBGHIGREEN,
         colors.TBGHIYELLOW,
         colors.TBGHICYAN,
         colors.TBGHIPURPLE,
         colors.TBGCYAN,
         colors.TBGYELLOW,
         colors.TBGPURPLE,
         colors.TBGHIBLUE,
         colors.TBGRED,
         colors.TBGGREEN,
         colors.TBGBLUE,
         ]

#-----------------------------------------------------------------------------
#-- group_cols
#--
def group_cols(row_args):
    cols = [-1]*len(row_args)
    curcol = 0
    for icol in range(1, len(row_args)):
        if cols[icol] < 0:
            cols[icol] = curcol
            curcol = curcol + 1
        #-- end if
        
        for icol2 in range(icol+1, len(row_args)):
            if row_args[icol] ==  row_args[icol2]:
                if   cols[icol2] < 0:
                    cols[icol2] = cols[icol]
                #-- end if
            #-- end if
        #-- end for
    #-- end for
    return cols
#-- end def 



random.seed(a=32)
# setup
arrs = []
for i in range(num_arrays):
    a = [random.randint(0, num_vals-1) for x in range(arr_size)]
    arrs.append(a)
#-- end for
cols = [[-1]*arr_size for x in range(num_arrays)]

curcol = 0
#cols[0] = arr_size*[0]

for irow in range(arr_size):
    curcol = 0
    '''
    for icol in range(num_arrays):
        if   cols[icol][irow] < 0:
            cols[icol][irow] = curcol
            curcol = curcol + 1
        #-- end if

        for icol2 in range(icol+1, num_arrays):
            #print("doing r %d, c1 %d c2 %d"%(irow, icol, icol2))
            if arrs[icol][irow] ==  arrs[icol2][irow]:
                if   cols[icol2][irow] < 0:
                    cols[icol2][irow] = cols[icol][irow]
                #-- end if
            #-- end for
        #-- end for
    #-- end for
    '''
    rows = [arrs[i][irow] for i in range(num_arrays)]
    rcols = group_cols(rows)
    for i in range(num_arrays):
        cols[i][irow] = rcols[i]
#-- end for

print(arrs)
for irow in range(arr_size):
    s=""
    for icol in range(num_arrays):
        s=s+("%d[%d] "%(arrs[icol][irow], cols[icol][irow]))
       
    #-- end for
    print("%s"%s)
for irow in range(arr_size):
    s=""
    for icol in range(num_arrays):
        #s=s+("%d[%d] "%(arrs[icol][irow], cols[icol][irow]))
        s=s+("%s%d%s "%(varcols[cols[icol][irow]], arrs[icol][irow], varcols[0]))
    #-- end for
    print("%s"%s)


a = [["0.0", "0.0", "0.0", "0.0", "0.0"],
    ["0.0", "0.0", "0.0", "0.0", "0.0001"],
    ["70.0", "70.0", "70.0", "70.0", "70.0"],
     ["67.51953125", "60.029296875", "60.107421875", "60.107421875", "64.482421875"]]

c = []
for i in range(len(a)):
    c.append(group_cols(a[i]))
             

for j in range(len(a)):
    s=""
    for i in range(len(a[j])):
        s = s + "%s%s%s "%(varcols[c[j][i]], a[j][i], varcols[0])
    print(s)

