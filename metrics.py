from cmake_generator import CMakeGenerator
from typing import List
import json
import matplotlib.pyplot as plt
import numpy as np
import os
import shutil
import subprocess as sp
import sys
import time

def regenerate_cmake(generator: CMakeGenerator, use_omp, omp_threads) -> bool:
    generator.generate(use_o2=True, use_omp=use_omp, omp_threads=omp_threads)
    process = sp.Popen(['cmake', '.'])
    process.wait()
    return process.returncode == 0

def compile_solution(generator: CMakeGenerator, use_omp, omp_threads) -> bool:
    if not regenerate_cmake(generator, use_omp, omp_threads):
        print('Cmake regeneration failed!')
        return False
    process = sp.Popen(['make'])
    process.wait()
    return process.returncode == 0

def generate_colors() -> List[str]:
    possibleParts = ['00', '20', '40', '60', '70', '80', 'A0', 'C0', 'D0', 'F0']
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

def get_answer(count_centroids, count_points, answer_input):
    answer = Answer()
    inp = answer_input
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

class FullTestRunResult:
    def __init__(self, run_result: RunResult, output_file_name: str, output_image_file_name: str):
        self.run_result: RunResult = run_result
        self.output_file_name: str = output_file_name
        self.output_image_file_name: str = output_image_file_name

def run_solution(inputFile, outputFile):
    start = time.time()
    process = sp.Popen(['./result.o'], stdin=inputFile, stdout=outputFile)
    process.wait()
    delta = time.time() - start
    return RunResult(process.returncode, delta)

def run_test_with_threads(test: str, omp_threads=None):
    use_omp = False
    if omp_threads is not None:
        use_omp = True

    output_file_name = 'output.txt'
    output_image_file_name = 'output_image.png'

    cmake_generator = CMakeGenerator()
    compilcation_result = compile_solution(cmake_generator, use_omp, omp_threads)
    # cmake_generator.restore_backup()
    assert(compilcation_result)

    # inputFile = input("Enter input file path: ")
    inputFile = '../lab_2/data/data' + test + '.in'
    
    run_result = run_solution(open(inputFile, 'r'), open(output_file_name, 'w'))
    assert(run_result.exit_code == 0)

    with open(inputFile, 'r') as inp:
        n, k = list(map(int, inp.readline().split()))
        points = []
        for i in range(n):
            x, y = list(map(float, inp.readline().split()))
            points.append((x, y))
        colors = generate_colors()[:k]
        answer = get_answer(k, n, open(output_file_name, 'r'))

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
        plt.savefig(output_image_file_name, transparent=True, dpi=600)
        plt.close()
    return FullTestRunResult(run_result, output_file_name, output_image_file_name)

def prettify_delta(delta: float) -> str:
    return str(int(delta * 1000) / 1000)

def main():
    tests = ['1', '2', '3', '4', '5']
    omp_params = [None, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    descriptions = ['No omp threads', '1 OMP thread', '2 OMP threads', '3 OMP threads',
        '4 OMP threads', '5 OMP threads', '6 OMP threads', '7 OMP threads', '8 OMP threads',
        '9 OMP threads']

    output_dirs = ['outputs/outs', 'outputs/images', 'outputs/charts']
    for output_dir in output_dirs:
        os.makedirs(output_dir, exist_ok=True)

    metrics_for_tests = dict()

    for test in tests:
        metrics_for_tests[test] = []
        for (omp_param, description) in zip(omp_params, descriptions):
            result = run_test_with_threads(test, omp_threads=omp_param)
            shutil.copyfile(result.output_file_name, 'outputs/outs/result' + test + '.out')
            shutil.copyfile(result.output_image_file_name, 'outputs/images/image_result' + test + '.png')
            metrics_for_tests[test].append({'description': description, 'time': result.run_result.delta})
    
    with open('outputs/metrics.json', 'w') as out:
        out.write(json.dumps(metrics_for_tests))

    for test in metrics_for_tests:
        indices = [i for i, _ in enumerate(metrics_for_tests[test])]
        plt.figure(figsize=(16, 7))
        deltas = list(map(lambda x: x['time'], metrics_for_tests[test]))
        plt.bar(indices, deltas, color='#ff7c43', width=0.5)
        for i in range(len(deltas)):
            plt.annotate(prettify_delta(deltas[i]), xy=(indices[i], deltas[i]), ha='center', va='bottom')
        plt.xticks(indices, descriptions)
        plt.ylabel('Time in seconds')
        plt.xlabel('Run description')
        plt.grid(color='#95a5a6', linestyle='--', linewidth=2, axis='y', alpha=0.5)
        plt.title('Time distribution for test ' + test + ' (with different run parameters)')
        plt.savefig('outputs/charts/chart_for_test_' + test + '.png')
        plt.close()

if __name__ == '__main__':
    main()
