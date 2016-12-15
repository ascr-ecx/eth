#! /bin/env pvpython

import sys
from paraview.simple import *

if len(sys.argv) != 4:
	print 'syntax: %s basename xres zres' % sys.argv[0]
	sys.exit(1)

basename = sys.argv[1]
xres = int(sys.argv[2])
zres = int(sys.argv[3])

f = open(basename + '.rgn')
rgns = []

for j, rgn in enumerate(f):
	rgn = [float(i) for i in rgn.strip().split()]
	if j == 0:
		mm = rgn[0::2]
		MM = rgn[1::2]
	else:
		mm = [mm[i] if mm[i] < rgn[2*i] else rgn[2*i] for i in range(3)]
		MM = [MM[i] if MM[i] > rgn[(2*i)+1] else rgn[(2*i)+1] for i in range(3)]
	rgns.append(rgn)

f.close()

yres = ((MM[1] - mm[1])/(MM[0] - mm[0])) * xres

for i, rgn in enumerate(rgns):
	reader = XMLUnstructuredGridReader(FileName=['%s-%d.vtu' % (basename, i)])
	resampler = ResampleToImage(Input=reader)
	resampler.UseInputBounds = 0
	resampler.SamplingBounds = rgn
	resampler.SamplingDimensions = [(int)(xres * (rgn[1] - rgn[0])/(MM[0] - mm[0])), (int)(xres * (rgn[3] - rgn[2])/(MM[1] - mm[1])), zres]
	SaveData('%s-%d.vti' % (basename, i), proxy=resampler)
