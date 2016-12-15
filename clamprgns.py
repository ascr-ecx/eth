#! /bin/env python 

import sys
import os

dset = sys.argv[1]
xmin = float(sys.argv[2])
xmax = float(sys.argv[3])
ymin = float(sys.argv[4])
ymax = float(sys.argv[5])
zmin = float(sys.argv[6])
zmax = float(sys.argv[7])

if not os.path.isfile(rgnfile)
	print 'No region file\n'
	sys.exit(0)

if rgnfile.rsplit('.')[1] != 'rgn':
	print 'Invalue region file (.rgn extension required)\n'
	sys.exit(0)

os.rename(dset, '%s.orig' % dset)

orgns = open('%s.orig' % dset)
nrgns = open(dset, 'w')
for rgn in orgns:
	rgn = [float(o) for o in rgn.strip().split()]
	for i in range(2):
		if rgn[i] < xmin: rgn[i] = xmin
		if rgn[i] > xmax: rgn[i] = xmax
	for i in range(2,4):
		if rgn[i] < ymin: rgn[i] = ymin
		if rgn[i] > ymax: rgn[i] =  ymax
	for i in range(4,6):
		if rgn[i] < zmin: rgn[i] = zmin
		if rgn[i] > zmax: rgn[i] =  zmax
	nrgns.write(' '.join([str(o) for o in rgn]) + '\n')

orgns.close()
nrgns.close()

	



