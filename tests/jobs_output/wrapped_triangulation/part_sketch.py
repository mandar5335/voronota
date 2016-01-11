from pymol.cgo import *
from pymol import cmd
sketch = [COLOR, 0.000, 1.000, 1.000,
SPHERE, 26.606, 11.082, 47.420, 1.550,
SPHERE, 27.821, 10.598, 46.726, 1.700,
SPHERE, 27.523, 9.590, 45.616, 1.700,
SPHERE, 27.850, 9.803, 44.444, 1.520,
SPHERE, 28.873, 10.053, 47.718, 1.700,
SPHERE, 30.337, 10.461, 47.425, 1.700,
SPHERE, 31.311, 9.584, 48.170, 1.700,
SPHERE, 31.508, 9.677, 49.381, 1.520,
SPHERE, 31.839, 8.653, 47.403, 1.520,
SPHERE, 19.406, 1.453, 37.244, 1.700,
SPHERE, 19.406, 1.453, 56.581, 1.700,
SPHERE, 19.406, 18.282, 37.244, 1.700,
SPHERE, 19.406, 18.282, 56.581, 1.700,
SPHERE, 39.039, 1.453, 37.244, 1.700,
SPHERE, 39.039, 1.453, 56.581, 1.700,
SPHERE, 39.039, 18.282, 37.244, 1.700,
SPHERE, 39.039, 18.282, 56.581, 1.700,
COLOR, 1.000, 1.000, 0.000,
BEGIN, LINE_STRIP,
VERTEX, 26.441, 8.300, 48.130,
VERTEX, 25.977, 12.393, 44.538,
END,
BEGIN, LINE_STRIP,
VERTEX, 26.165, 17.637, 42.312,
VERTEX, 25.977, 12.393, 44.538,
END,
BEGIN, LINE_STRIP,
VERTEX, 28.825, 13.822, 48.636,
VERTEX, 26.441, 8.300, 48.130,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.141, 24.084, 45.909,
VERTEX, 28.825, 13.822, 48.636,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 23.686, 44.500,
VERTEX, 30.141, 24.084, 45.909,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 23.686, 44.500,
VERTEX, 26.165, 17.637, 42.312,
END,
BEGIN, LINE_STRIP,
VERTEX, 20.748, 12.287, 42.352,
VERTEX, 26.165, 17.637, 42.312,
END,
BEGIN, LINE_STRIP,
VERTEX, 20.748, 12.287, 42.352,
VERTEX, 25.977, 12.393, 44.538,
END,
BEGIN, LINE_STRIP,
VERTEX, 23.979, 4.321, 50.588,
VERTEX, 26.441, 8.300, 48.130,
END,
BEGIN, LINE_STRIP,
VERTEX, 17.997, 5.335, 46.913,
VERTEX, 23.979, 4.321, 50.588,
END,
BEGIN, LINE_STRIP,
VERTEX, 17.563, 9.868, 42.928,
VERTEX, 20.748, 12.287, 42.352,
END,
BEGIN, LINE_STRIP,
VERTEX, 17.563, 9.868, 42.928,
VERTEX, 17.997, 5.335, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.023, 15.956, 52.597,
VERTEX, 28.825, 13.822, 48.636,
END,
BEGIN, LINE_STRIP,
VERTEX, 25.373, 9.612, 57.212,
VERTEX, 23.979, 4.321, 50.588,
END,
BEGIN, LINE_STRIP,
VERTEX, 25.373, 9.612, 57.212,
VERTEX, 29.023, 15.956, 52.597,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.332, 18.426, 53.630,
VERTEX, 29.023, 15.956, 52.597,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.287, 25.227, 46.913,
VERTEX, 29.332, 18.426, 53.630,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.287, 25.227, 46.913,
VERTEX, 30.141, 24.084, 45.909,
END,
BEGIN, LINE_STRIP,
VERTEX, 25.349, 9.868, 57.458,
VERTEX, 25.373, 9.612, 57.212,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 18.341, 53.845,
VERTEX, 29.332, 18.426, 53.630,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 18.341, 53.845,
VERTEX, 25.349, 9.868, 57.458,
END,
BEGIN, LINE_STRIP,
VERTEX, 12.024, 9.868, 46.913,
VERTEX, 25.349, 9.868, 57.458,
END,
BEGIN, LINE_STRIP,
VERTEX, 12.024, 9.868, 46.913,
VERTEX, 17.997, 5.335, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 12.024, 9.868, 46.913,
VERTEX, 12.024, 9.868, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 12.024, 9.868, 46.913,
VERTEX, 17.563, 9.868, 42.928,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 27.034, 46.913,
VERTEX, 29.223, 23.686, 44.500,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 27.034, 46.913,
VERTEX, 12.024, 9.868, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 27.034, 46.913,
VERTEX, 30.287, 25.227, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 27.034, 46.913,
VERTEX, 29.223, 18.341, 53.845,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 27.034, 46.913,
VERTEX, 29.223, 27.034, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.551, 10.433, 45.358,
VERTEX, 25.977, 12.393, 44.538,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.372, 9.893, 45.898,
VERTEX, 28.825, 13.822, 48.636,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.372, 9.893, 45.898,
VERTEX, 29.551, 10.433, 45.358,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.372, 9.893, 45.898,
VERTEX, 26.441, 8.300, 48.130,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.889, 18.261, 42.075,
VERTEX, 30.141, 24.084, 45.909,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.889, 18.261, 42.075,
VERTEX, 29.551, 10.433, 45.358,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 19.856, 41.396,
VERTEX, 29.223, 23.686, 44.500,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 19.856, 41.396,
VERTEX, 30.889, 18.261, 42.075,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 19.856, 41.396,
VERTEX, 26.165, 17.637, 42.312,
END,
BEGIN, LINE_STRIP,
VERTEX, 30.269, 8.619, 45.115,
VERTEX, 29.551, 10.433, 45.358,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.873, -0.139, 42.244,
VERTEX, 30.269, 8.619, 45.115,
END,
BEGIN, LINE_STRIP,
VERTEX, 19.254, 9.868, 41.288,
VERTEX, 17.563, 9.868, 42.928,
END,
BEGIN, LINE_STRIP,
VERTEX, 19.254, 9.868, 41.288,
VERTEX, 20.748, 12.287, 42.352,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -0.690, 41.885,
VERTEX, 29.873, -0.139, 42.244,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -0.690, 41.885,
VERTEX, 19.254, 9.868, 41.288,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.834, 8.275, 45.957,
VERTEX, 30.269, 8.619, 45.115,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.834, 8.275, 45.957,
VERTEX, 29.372, 9.893, 45.898,
END,
BEGIN, LINE_STRIP,
VERTEX, 26.864, 0.177, 49.648,
VERTEX, 23.979, 4.321, 50.588,
END,
BEGIN, LINE_STRIP,
VERTEX, 26.864, 0.177, 49.648,
VERTEX, 29.834, 8.275, 45.957,
END,
BEGIN, LINE_STRIP,
VERTEX, 27.250, -3.895, 46.913,
VERTEX, 17.997, 5.335, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 27.250, -3.895, 46.913,
VERTEX, 26.864, 0.177, 49.648,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -1.876, 43.037,
VERTEX, 29.223, -0.690, 41.885,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -1.876, 43.037,
VERTEX, 29.873, -0.139, 42.244,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -1.876, 43.037,
VERTEX, 27.250, -3.895, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 34.701, 11.817, 40.383,
VERTEX, 30.889, 18.261, 42.075,
END,
BEGIN, LINE_STRIP,
VERTEX, 34.701, 11.817, 40.383,
VERTEX, 30.269, 8.619, 45.115,
END,
BEGIN, LINE_STRIP,
VERTEX, 35.222, 9.867, 38.922,
VERTEX, 34.701, 11.817, 40.383,
END,
BEGIN, LINE_STRIP,
VERTEX, 35.222, 9.867, 38.922,
VERTEX, 29.873, -0.139, 42.244,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 29.738,
VERTEX, 29.223, -0.690, 41.885,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 29.738,
VERTEX, 19.254, 9.868, 41.288,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.867, 29.738,
VERTEX, 35.222, 9.867, 38.922,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.867, 29.738,
VERTEX, 29.223, 19.856, 41.396,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.867, 29.738,
VERTEX, 29.223, 9.868, 29.738,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.938, 10.255, 49.230,
VERTEX, 29.023, 15.956, 52.597,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.999, 8.460, 47.037,
VERTEX, 29.834, 8.275, 45.957,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.999, 8.460, 47.037,
VERTEX, 29.938, 10.255, 49.230,
END,
BEGIN, LINE_STRIP,
VERTEX, 28.294, 3.289, 50.868,
VERTEX, 29.999, 8.460, 47.037,
END,
BEGIN, LINE_STRIP,
VERTEX, 28.294, 3.289, 50.868,
VERTEX, 29.938, 10.255, 49.230,
END,
BEGIN, LINE_STRIP,
VERTEX, 27.641, 1.448, 51.712,
VERTEX, 26.864, 0.177, 49.648,
END,
BEGIN, LINE_STRIP,
VERTEX, 27.641, 1.448, 51.712,
VERTEX, 25.373, 9.612, 57.212,
END,
BEGIN, LINE_STRIP,
VERTEX, 27.641, 1.448, 51.712,
VERTEX, 28.294, 3.289, 50.868,
END,
BEGIN, LINE_STRIP,
VERTEX, 36.078, 16.653, 48.733,
VERTEX, 29.332, 18.426, 53.630,
END,
BEGIN, LINE_STRIP,
VERTEX, 36.078, 16.653, 48.733,
VERTEX, 29.938, 10.255, 49.230,
END,
BEGIN, LINE_STRIP,
VERTEX, 37.139, 13.806, 43.995,
VERTEX, 34.701, 11.817, 40.383,
END,
BEGIN, LINE_STRIP,
VERTEX, 37.139, 13.806, 43.995,
VERTEX, 29.999, 8.460, 47.037,
END,
BEGIN, LINE_STRIP,
VERTEX, 37.733, 16.943, 46.913,
VERTEX, 30.287, 25.227, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 37.733, 16.943, 46.913,
VERTEX, 36.078, 16.653, 48.733,
END,
BEGIN, LINE_STRIP,
VERTEX, 37.733, 16.943, 46.913,
VERTEX, 37.139, 13.806, 43.995,
END,
BEGIN, LINE_STRIP,
VERTEX, 41.434, 12.127, 48.492,
VERTEX, 36.078, 16.653, 48.733,
END,
BEGIN, LINE_STRIP,
VERTEX, 41.434, 12.127, 48.492,
VERTEX, 28.294, 3.289, 50.868,
END,
BEGIN, LINE_STRIP,
VERTEX, 41.681, 13.435, 46.913,
VERTEX, 37.733, 16.943, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 41.681, 13.435, 46.913,
VERTEX, 41.434, 12.127, 48.492,
END,
BEGIN, LINE_STRIP,
VERTEX, 41.681, 13.435, 46.913,
VERTEX, 37.139, 13.806, 43.995,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 0.022, 52.715,
VERTEX, 27.641, 1.448, 51.712,
END,
BEGIN, LINE_STRIP,
VERTEX, 42.677, 9.867, 49.870,
VERTEX, 41.434, 12.127, 48.492,
END,
BEGIN, LINE_STRIP,
VERTEX, 42.677, 9.867, 49.870,
VERTEX, 29.223, 0.022, 52.715,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 63.856,
VERTEX, 29.223, 18.341, 53.845,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 63.856,
VERTEX, 25.349, 9.868, 57.458,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 63.856,
VERTEX, 42.677, 9.867, 49.870,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 63.856,
VERTEX, 29.223, 9.868, 63.856,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, 9.868, 63.856,
VERTEX, 29.223, 0.022, 52.715,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -7.234, 46.913,
VERTEX, 29.223, 0.022, 52.715,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -7.234, 46.913,
VERTEX, 27.250, -3.895, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -7.234, 46.913,
VERTEX, 29.223, -7.234, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 29.223, -7.234, 46.913,
VERTEX, 29.223, -1.876, 43.037,
END,
BEGIN, LINE_STRIP,
VERTEX, 46.362, 9.867, 46.913,
VERTEX, 42.677, 9.867, 49.870,
END,
BEGIN, LINE_STRIP,
VERTEX, 46.362, 9.867, 46.913,
VERTEX, 29.223, -7.234, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 46.362, 9.868, 46.913,
VERTEX, 41.681, 13.435, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 46.362, 9.868, 46.913,
VERTEX, 46.362, 9.867, 46.913,
END,
BEGIN, LINE_STRIP,
VERTEX, 46.362, 9.868, 46.913,
VERTEX, 35.222, 9.867, 38.922,
END,
]
cmd.load_cgo(sketch, 'sketch')
cmd.set('two_sided_lighting', 'off')
