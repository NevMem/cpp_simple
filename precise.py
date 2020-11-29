import numpy as np

def u(x, t):
    result = np.zeros(x.shape[0])
    k = 1.0
    l = 1.0
    for m in range(10000):
        result += 4 / np.pi * np.exp(-k * (np.pi ** 2) * ((2 * m + 1) ** 2) * t / (l ** 2)) / (2 * m + 1) * np.sin(np.pi * (2 * m + 1) * x / l)
    return result

def precise_solution_for_11_points():
    points = np.arange(0.0, 1.01, 0.1)
    assert(points.shape[0] == 11)
    result = u(points, 0.1)
    return points, result

def main():
    print(precise_solution_for_11_points())

if __name__ == '__main__':
    main()