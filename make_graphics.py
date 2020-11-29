from precise import precise_solution_for_11_points
import matplotlib.pyplot as plt
import subprocess as sp
import time
import typing

class RunParams:
    def __init__(self, start: float, finish: float, point_delta: float, time_delta: float, time: float):
        self.start: float = start
        self.finish: float = finish
        self.point_delta: float = point_delta
        self.time_delta: float = time_delta
        self.time: float = time

class RunResults:
    def __init__(self, run_time: float, x=None, y=None):
        self.run_time: float = run_time
        self.x = x
        self.y = y

def generate_cmake():
    process = sp.Popen(['cmake', '.'], stdin=sp.PIPE, stdout=sp.PIPE)
    process.wait()
    assert(process.returncode == 0)

def make():
    process = sp.Popen(['make'], stdin=sp.PIPE)
    process.wait()
    assert(process.returncode == 0)

def write_run_params(filename: str, run_params: RunParams):
    with open(filename, 'w') as out:
        out.write(str(run_params.start) + ' ' + str(run_params.finish) + '\n')
        out.write(str(run_params.point_delta) + '\n')
        out.write(str(run_params.time_delta) + '\n')
        out.write(str(run_params.time) + '\n')

def run_solution_with_processes_and_params(proc_num: int, params: RunParams, parse_run_results=False) -> RunResults:
    write_run_params('input.txt', params)
    start = time.time()
    process = sp.Popen(['mpirun', '-n', str(proc_num), 'result.o'], stdout=open('output.txt', 'w'))
    process.wait()
    delta = time.time() - start
    assert(process.returncode == 0)
    if parse_run_results:
        x, y = [], []
        with open('output.txt', 'r') as inp:
            for line in inp.readlines():
                if len(line.split()) != 2:
                    continue
                x_, y_ = list(map(float, line.split()))
                x.append(x_)
                y.append(y_)
        return RunResults(delta, x, y)
    else:
        return RunResults(delta)

def main():
    generate_cmake()
    make()

    result: RunResults = run_solution_with_processes_and_params(4, RunParams(0, 1, 0.02, 0.0002, 0.1), parse_run_results=True)
    real_x, real_y = precise_solution_for_11_points()
    plt.figure(figsize=(10, 4))
    plt.plot(real_x, real_y, label='Real')
    plt.plot(result.x, result.y, label='Solution with MPI')
    plt.xlabel('X')
    plt.ylabel('Temperature')
    plt.legend()
    plt.savefig('comparison.png')

if __name__ == '__main__':
    main()
