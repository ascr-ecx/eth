#! /usr/bin/env vtkpython

import sys
from vtk import *

parts = [2, 2, 2]

r = vtkXMLImageDataReader()
r.SetFileName(sys.argv[1])

if (len(sys.argv) > 2):
  r.UpdateInformation()
  for i in range(r.GetNumberOfPointArrays()):
    n = r.GetPointArrayName(i)
    if n in sys.argv[2:]:
      r.SetPointArrayStatus(n, 1)
    else:
      r.SetPointArrayStatus(n, 0)

r.Update()

extent = r.GetOutput().GetExtent()
origin = [extent[2*i] for i in range(3)]
span   = [extent[2*i + 1] - extent[2*i] for i in range(3)]
step  = [span[i] / parts[i] for i in range(3)]

clp = vtkImageClip()
clp.SetInputConnection(r.GetOutputPort())
clp.ClipDataOn()

wrtr = vtkXMLImageDataWriter()
wrtr.SetInputConnection(clp.GetOutputPort())

e = [0]*6
p = 0
for i in range(2):
	e[0] = origin[0] + i*step[0]
	e[1] = (origin[0] + (i+1)*step[0]) if i != parts[0] else extent[1]
	for j in range(2):
		e[2] = origin[1] + j*step[1]
		e[3] = (origin[1] + (j+1)*step[1]) if i != parts[1] else extent[3]
		for k in range(2):
			e[4] = origin[2] + k*step[2]
			e[5] = (origin[2] + (k+1)*step[2]) if i != parts[2] else extent[5]
			clp.SetOutputWholeExtent(e)
			wrtr.SetFileName('%s-%d.vti' % (sys.argv[1].rsplit('.',1)[0], p))
			wrtr.Update()
			p = p + 1
