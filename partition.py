#! /bin/env vtkpython

import sys
from vtk import *

dset = sys.argv[1]
num = int(sys.argv[2])

base = dset.rsplit('.', 1)[0]
ext = dset.rsplit('.', 1)[1]

if ext == 'nc':

	r = vtkMPASReader()
	r.SetFileName(dset)
	r.SetLayerThickness(1)
	r.SetCenterLon(0)
	r.SetProjectLatLon(True)
	r.SetShowMultilayerView(True)
	r.SetVerticalLevel(0)

	r.UpdateInformation()

	for i in range(r.GetNumberOfCellArrays()):
		var = r.GetCellArrayName(i)
		if var in sys.argv[3:]:
			r.SetCellArrayStatus(var, True)
		else:
			r.SetCellArrayStatus(var, False)

	for i in range(r.GetNumberOfPointArrays()):
		var = r.GetPointArrayName(i)
		if var in sys.argv[3:]:
			r.SetPointArrayStatus(var, True)
		else:
			r.SetPointArrayStatus(var, False)
		
elif ext == 'vtu':

	r = vtkXMLUnstructuredGridReader()
	r.SetFileName(dset)

else:

	print 'unknown file type: ', ext
	sys.exit(1)

print 'loading data...'
r.Update()
print 'data loaded, building Kd Tree ...'

kd = vtkKdTree()
kd.SetDataSet(r.GetOutput())
kd.SetNumberOfRegionsOrMore(num)
kd.OmitZPartitioning()
print '... building Locator'
kd.BuildLocator()
print '... building cell lists'
kd.IncludeRegionBoundaryCellsOn()
kd.CreateCellLists()

kd.Update()

print 'Kd Tree built'

b = open(base + '.rgn', 'w')
for i in range(kd.GetNumberOfRegions()):
	d = [0] * 6
	kd.GetRegionBounds(i, d)
	b.write('%f %f %f %f %f %f\n' % tuple(d))
b.close()

ec = vtkExtractCells()
ec.SetInputConnection(r.GetOutputPort())

pa = vtkPassArrays()
pa.SetInputConnection(ec.GetOutputPort())
pa.AddCellDataArray('vtkOriginalCellIds')
pa.RemoveArraysOn()

to_tets = vtkDataSetTriangleFilter()
to_tets.SetInputConnection(pa.GetOutputPort())

aa = vtkAssignAttribute()
aa.SetInputConnection(to_tets.GetOutputPort())
aa.Assign('salinity', 'SCALARS', 'POINT_DATA')

trim = vtkThreshold()
trim.SetInputConnection(aa.GetOutputPort())
trim.ThresholdByUpper(-1e30)
trim.AllScalarsOn()

w = vtkXMLUnstructuredGridWriter()
w.SetInputConnection(trim.GetOutputPort())

for i in range(kd.GetNumberOfRegions()):
	i0 = kd.GetCellList(i)
	i1 = kd.GetBoundaryCellList(i)
	pids = vtkIdList()
	pids.SetNumberOfIds(i0.GetNumberOfIds() + i1.GetNumberOfIds())
	pids.SetNumberOfIds(i0.GetNumberOfIds())
	for j in range(i0.GetNumberOfIds()):
		pids.InsertId(j, i0.GetId(j))
	for j in range(i1.GetNumberOfIds()):
		pids.InsertId(j + i0.GetNumberOfIds(), i1.GetId(j))
	ec.SetCellList(pids)
	w.SetFileName('%s-%d.vtu' % (dset.rsplit('.', 1)[0], i))
	w.Update()
	print 'wrote part %d of %d' % (i, kd.GetNumberOfRegions())
