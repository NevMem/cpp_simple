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

def generate_run_params_for_count_of_points(count: int) -> RunParams:
    length = 1.0
    start = 0.0
    finish = start + length
    point_delta = length / count

    time_delta = point_delta ** 2 * 0.9

    iterations_count = 200

    T = time_delta * iterations_count

    return RunParams(start=start, finish=finish, point_delta=point_delta, time_delta=time_delta, time=T)

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
    plt.close()

    processors = [1, 2, 4, 8, 16]
    points_counts = [2000, 10000, 50000]

    times = []
    procs = []
    labels = ['Count of points ' + str(count) for count in points_counts]
    names = [str(count) for count in points_counts]

    for count_of_points in points_counts:
        times.append([])
        procs.append([])
        for proc_count in processors:
            result = run_solution_with_processes_and_params(proc_count, generate_run_params_for_count_of_points(count_of_points))
            times[-1].append(result.run_time)
            procs[-1].append(proc_count)

    plt.figure(figsize=(10, 4))
    for x, y, label, name in zip(procs, times, labels, names):
        plt.plot(x, y, label=label)
        plt.xlabel('Num of processors')
        plt.ylabel('Time in seconds')
        plt.legend()
        plt.savefig('graph_' + name + '.png')
        plt.close()
    

if __name__ == '__main__':
    main()
