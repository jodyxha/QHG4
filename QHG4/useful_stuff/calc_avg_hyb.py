#!/usr/bin/python

from sys import argv
import h5py

if len(argv) > 2:
    hybname = argv[2]
else:
    hybname = 'Hybridization'
#-- end if
    
f = h5py.File(argv[1],"r")
a=f['Populations/sapiens/AgentDataSet']
ch={}
cn={}
n0=0
n1=0
hmax=-1
hmin=10

for i in range(int(len(a))):
    c = a[i]['CellIdx']
    ch[c] = 0
    cn[c] = 0
#-- end for
for i in range(int(len(a))):
    h = a[i][hybname]
    c = a[i]['CellIdx']
    ch[c] = ch[c] + h
    cn[c] = cn[c] + 1
    hmax = max(hmax,h)
    hmin = min(hmin,h)
    if (h==0):
      n0 = n0 + 1
    #-- end if  
    if (h==1):
      n1 = n1 + 1
    #-- end if  
#-- end for

for i in ch:
  ch[i] = (1.0*ch[i])/cn[i]
g=f['Populations/sapiens']
minhyb = g.attrs['HybBirthDeathRel_hybminprob'][0]

print("hybminprob: %f"%minhyb)
print("min hyb:    %f"%hmin)
print("max hyb:    %f"%hmax)
#if (hmax == hmin):
#  print("dieout hmin %f, hmax %f"%(hmin,hmax))
#else:
#print("alive  sap  %d, nea  %d"%(n0,n1))
#for i in ch:
#    print("%d; %f"%(i, ch[i]))
f.close()
