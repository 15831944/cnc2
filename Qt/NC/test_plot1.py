import numpy as np
import matplotlib.pyplot as plt
import os

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __str__(self):
        return "(" + str(self.x) + ", " + str(self.y) + ")"
    def __repr__(self):
        return self.__str__()

cur_dir = os.path.dirname(os.path.abspath(__file__))

data = np.fromfile(cur_dir + '\\motor_xy.dat', dtype = np.int32)

##with open('motor_xy.dat', "rb") as f:
##    read_data = f.read()
##file_size = int.from_bytes(fin.read(2), byteorder='big')

##data = [3, 0, 0, 3, -3, 0, 0, -3]

print("Length ", len(data), "Points", int(len(data)/2) + 1)
pts = [Point(0,0)]
print(len(pts), "Point:", pts[len(pts) - 1])

for i in range(int(len(data)/2)):
    pts.append( Point(pts[len(pts) - 1].x + data[2*i], pts[len(pts) - 1].y + data[2*i + 1]) )
    print(len(pts), "Point:", pts[len(pts) - 1])

x = [0] * len(pts)
y = [0] * len(pts)

for i in range(len(pts)):
    x[i] = pts[i].x
    y[i] = pts[i].y

plt.plot(x ,y)
plt.show()
