import matplotlib.pyplot as plt
import subprocess as sp
import numpy as np
from typing import List
import time

def regenerate_cmake() -> bool:
    process = sp.Popen(['cmake', '.'])
    process.wait()
    return process.returncode == 0

def compile_solution() -> bool:
    if not regenerate_cmake():
        print('Cmake regeneration failed!')
        return False
    process = sp.Popen(['make'])
    process.wait()
    return process.returncode == 0

def generate_colors() -> List[str]:
    possibleParts = ['00', '20', '40', '60', '80', 'A0', 'C0', 'D0', 'F0']
    colors = []
    for part_1 in possibleParts:
        for part_2 in possibleParts:
            for part_3 in possibleParts:
                colors.append('#' + part_1 + part_2 + part_3)
    np.random.seed(0)
    np.random.shuffle(colors)
    return colors

class Answer:
    def __init__(self):
        self.centroids = []
        self.assignment = []

def get_answer(count_centroids, count_points):
    answer = Answer()
    with open('output.txt', 'r') as inp:
        for _ in range(count_centroids):
            x, y = list(map(float, inp.readline().split()))
            answer.centroids.append((x, y))
        for _ in range(count_points):
            answer.assignment.append(int(inp.readline()))
    return answer

class RunResult:
    def __init__(self, exit_code: int, delta: float):
        self.exit_code = exit_code
        self.delta = delta

def run_solution(inputFile, outputFile):
    start = time.time()
    process = sp.Popen(['./result.o'], stdin=inputFile, stdout=outputFile)
    process.wait()
    delta = time.time() - start
    return RunResult(process.returncode, delta)

def main():
    assert(compile_solution())

    # inputFile = input("Enter input file path: ")
    inputFile = '../lab_2/data/data3.in'
    
    run_result = run_solution(open(inputFile, 'r'), open('output.txt', 'w'))
    assert(run_result.exit_code == 0)

    print('Run in:', run_result.delta, 'seconds')

    with open(inputFile, 'r') as inp:
        n, k = list(map(int, inp.readline().split()))
        points = []
        for i in range(n):
            x, y = list(map(float, inp.readline().split()))
            points.append((x, y))
        colors = generate_colors()[:k]
        answer = get_answer(k, n)

        plt.scatter(
            list(map(lambda x: x[0], points)),
            list(map(lambda x: x[1], points)),
            c=list(map(lambda x: colors[x], answer.assignment)),
            marker='.',
            s=7,
            linewidths=0)
        plt.scatter(
            list(map(lambda x: x[0], answer.centroids)),
            list(map(lambda x: x[1], answer.centroids)),
            c=colors,
            marker='1')
        plt.savefig('output_image.png', transparent=True, dpi=600)

if __name__ == '__main__':
    main()
