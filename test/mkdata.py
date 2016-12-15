#! /usr/bin/env pvpython

from paraview.simple import *

w = Wavelet()
t = Tetrahedralize(Input=w)
SaveData("wavelet.vti", proxy=w)
SaveData("wavelet.vtu", proxy=t)
