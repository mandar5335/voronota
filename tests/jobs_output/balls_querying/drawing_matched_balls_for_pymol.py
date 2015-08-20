from pymol.cgo import *
from pymol import cmd
CRO_balls = [COLOR, 0.000, 1.000, 1.000,
SPHERE, 24.077, 27.513, 36.610, 1.600,
SPHERE, 25.011, 26.478, 37.078, 1.800,
SPHERE, 25.931, 26.035, 35.930, 1.800,
SPHERE, 25.155, 25.422, 34.796, 1.800,
SPHERE, 26.679, 27.129, 35.461, 1.500,
SPHERE, 25.730, 27.106, 38.245, 1.800,
SPHERE, 26.975, 27.732, 38.216, 1.600,
SPHERE, 25.274, 27.124, 39.509, 1.600,
SPHERE, 26.043, 27.875, 40.370, 1.800,
SPHERE, 26.022, 27.962, 41.566, 1.500,
SPHERE, 27.197, 28.245, 39.512, 1.800,
SPHERE, 23.919, 26.721, 39.842, 1.800,
SPHERE, 23.745, 25.326, 40.360, 1.800,
SPHERE, 22.885, 25.116, 41.193, 1.500,
SPHERE, 28.329, 28.822, 39.960, 1.800,
SPHERE, 29.437, 29.370, 39.124, 1.800,
SPHERE, 29.541, 29.103, 37.742, 1.800,
SPHERE, 30.487, 30.110, 39.805, 1.800,
SPHERE, 30.707, 29.546, 37.033, 1.800,
SPHERE, 31.614, 30.563, 39.085, 1.800,
SPHERE, 31.718, 30.300, 37.721, 1.800,
SPHERE, 32.894, 30.804, 36.971, 1.500,
]
cmd.load_cgo(CRO_balls, 'CRO_balls')
cmd.set('two_sided_lighting', 'on')
