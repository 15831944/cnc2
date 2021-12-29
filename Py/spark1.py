import numpy as np
import matplotlib.pyplot as plt
import os

##class Point:
##    def __init__(self, x, y):
##        self.x = x
##        self.y = y
##    def __str__(self):
##        return "(" + str(self.x) + ", " + str(self.y) + ")"
##    def __repr__(self):
##        return self.__str__()
##
##cur_dir = os.path.dirname(os.path.abspath(__file__))
##
##data = np.fromfile(cur_dir + '\\motor_xy.dat', dtype = np.int32)
##
####with open('motor_xy.dat', "rb") as f:
####    read_data = f.read()
####file_size = int.from_bytes(fin.read(2), byteorder='big')
##
####data = [3, 0, 0, 3, -3, 0, 0, -3]

R_lim = 1
R1 = 10
L1 = 1 # mm
V_pwr = 120 # V
K_ui = 10 # um / sec
U_cut = 200 #um / sec

F_wire = 200 # um/s
x = -2 # mm
dt = 0.1 # sec

def R_gap(l):
    return R1 / L1 * l

def I_gap(R):
    return V_pwr / (R + R_lim)

def U_cut(I):
    return K_ui * I

def L_cut(U, t):
    return U * t

V[0] = V_pwr
t[0] = 0
x[0] = -1 # mm - wire position
d[0] = 0 # mm - 

for i in range(1, 1000):
    t[i] = i * dt
    L_wire[i] = 

##print("Length ", len(data), "Points", int(len(data)/2) + 1)
##pts = [Point(0,0)]
##print(len(pts), "Point:", pts[len(pts) - 1])
##
##for i in range(int(len(data)/2)):
##    pts.append( Point(pts[len(pts) - 1].x + data[2*i], pts[len(pts) - 1].y + data[2*i + 1]) )
##    print(len(pts), "Point:", pts[len(pts) - 1])
##
##x = [0] * len(pts)
##y = [0] * len(pts)
##
##for i in range(len(pts)):
##    x[i] = pts[i].x
##    y[i] = pts[i].y
##
##plt.plot(x ,y)
##plt.show()
