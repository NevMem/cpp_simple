import sys
import os
import subprocess as sp
import time
import numpy as np
import json

def make():
    proc = sp.Popen(["cmake", "."])
    proc.wait()

    proc = sp.Popen(["make"])
    result = proc.wait()
    if result != 0:
        sys.exit(1)

def read_file(file):
    with open(file, 'r') as inp:
        output_answer = int(inp.readline())
        output_parts = sorted(list(map(int, inp.readline().split())))
        return output_answer, output_parts

def check(output_file, real_file, input_file):
    output_answer, output_parts = read_file(output_file)
    real_answer, real_parts = read_file(real_file)
    
    validation = True
    reason = ""

    with open(input_file, 'r') as inp:
        n, capacity = list(map(int, inp.readline().split()))
        k_cost, k_size = 0, 0
        for i in range(n):
            cost, size = list(map(int, inp.readline().split()))
            if i + 1 in output_parts:
                k_cost += cost
                k_size += size
        if k_size > capacity:
            validation = False
            reason = "overflow "
        if k_cost != output_answer:
            validation = False
            reason += "no proof " + str(k_cost) + " " + str(output_answer)
    
    if output_answer == real_answer and output_parts == real_parts and validation:
        # print("ok")
        return True
    else:
        if not validation:
            print("Validation failed", reason, input_file)
            return False
        if output_answer != real_answer:
            print("Answers doesn't match, found:", output_answer, "expected:", real_answer, input_file)
            print(output_parts, real_parts)
        else:
            # print('Parts doesn`t match')
            # print(output_parts, real_parts)
            return True
        return False
   

def runSmallTests(runs=5, runParams=[]):
    test_dir = '../lab_1/small_tests'
    input_files = []
    output_files = []
    for file in os.listdir(test_dir):
        if os.path.isfile(test_dir + '/' + file):
            if file[-3:] == '.in':
                input_files.append(file)
                output_files.append(file[:-3] + '.out')

    oks = 0

    whole_delta = 0

    for (input_file, output_file) in zip(input_files, output_files):
        args = ["./result.o"]
        for param in runParams:
            args.append(param)
        delta = 0
        for i in range(runs):
            start = time.time()
            proc = sp.Popen(args, stdin=open(test_dir + "/" + input_file, 'r'), stdout=open("output.txt", 'w'))
            proc.wait()
            delta += time.time() - start
        delta /= runs
        whole_delta += delta

        if check("output.txt", test_dir + "/" + output_file, test_dir + "/" + input_file):
            oks += 1
    return (whole_delta, (oks * 1.0) / len(input_files))

def scale_test(from_file, to_file, scale):
    with open(from_file, 'r') as inp:
        count, capacity = list(map(int, inp.readline().split()))
        scaled_count = int(count * scale)
        scaled_capacity = int(capacity * scale)
        lines = []
        for i in range(scaled_count):
            lines.append(inp.readline())
        with open(to_file, 'w') as out:
            out.write(str(scaled_count) + " " + str(scaled_capacity) + "\n")
            for line in lines:
                out.write(line)

def runLargeTests(runParams=[], scale=1.0):
    test_dir = '../lab_1/tests_100'

    pre_input_files = []
    for file in os.listdir(test_dir):
        if os.path.isfile(test_dir + '/' + file):
            if file[-3:] == '.in' and file.find("100") != -1:
                pre_input_files.append(file)
    
    input_files = []
    for file in pre_input_files:
        input_files.append('/scaled_' + file.replace("100", "---"))
        scale_test(test_dir + '/' + file, test_dir + input_files[-1], scale)

    sumDiff = 0
    timeouts = 0

    for input_file in input_files:
        args = ["./result.o"]
        for param in runParams:
            args.append(param)
        
        start = time.time()
        proc = sp.Popen(args, stdin=open(test_dir + "/" + input_file, 'r'), stdout=open("output.txt", 'w'))
        try:
            proc.wait(500)
            if proc.returncode != 0:
                print("Return code", proc.returncode)
                sys.exit(0)
            sumDiff += time.time() - start
            # print("Done", time.time() - start)
        except KeyboardInterrupt as keyboard:
            print("Keyboard")
            proc.terminate()
            sys.exit(0)
        except:
            print("Timeout")
            timeouts += 1
            proc.terminate()
    print(runParams, sumDiff, timeouts)

def createLargeTest(filename, size):
    items = []
    total_weight = 0
    for i in range(size):
        w = np.random.randint(1, 101)
        p = w + 10
        items.append((p, w))
        total_weight += w
    with open(filename, "w") as inp:
        inp.write(str(size) + " " + str(int(total_weight / 2)) + "\n")
        for (p, w) in items:
            inp.write(str(p) + " " + str(w) + "\n")

def runUltraLargeTests(input_files, runParams=[]):
    sumDiff = 0
    timeouts = 0

    for input_file in input_files:
        args = ["./result.o"]
        for param in runParams:
            args.append(param)
        
        start = time.time()
        proc = sp.Popen(args, stdin=open("./" + input_file, 'r'), stdout=open("output.txt", 'w'))
        try:
            proc.wait(50000)
            sumDiff += time.time() - start
            if proc.returncode != 0:
                print('Return code:', proc.returncode)
                sys.exit(0)
            # print("Done", time.time() - start)
        except KeyboardInterrupt as keyboard:
            print("Keyboard")
            proc.terminate()
            sys.exit(0)
        except:
            print("Timeout")
            timeouts += 1
            proc.terminate()
    print(runParams, sumDiff, timeouts)
    return sumDiff


def main():
    make()
    runs = 1

    runTime, oks = runSmallTests(runs, ["mode=single_opt"])
    print(runTime, oks)

    runTime, oks = runSmallTests(runs, ["mode=single"])
    print(runTime, oks)

    runTime, oks = runSmallTests(runs, ["mode=multi"])
    print(runTime, oks)

    sizes = []
    for i in range(7, 31):
        sizes.append(i * 100)

    result = dict()
    result["multi"] = []
    result["single_opt"] = []

    for size in sizes:
        count_runs = 5
        multi_whole = 0
        sinle_opt_whole = 0
        for runs in range(count_runs):
            print(size, "run:", runs, '/', count_runs)
            createLargeTest("input.txt", size)
            sinle_opt_whole += runUltraLargeTests(["input.txt"], ["mode=single_opt"])
            multi_whole += runUltraLargeTests(["input.txt"], ["mode=multi"])
            print()
        
        result["single_opt"].append({ "size": size, "time": sinle_opt_whole / count_runs })
        result["multi"].append({ "size": size, "time": multi_whole / count_runs })
    
    with open("result.json", "w") as out:
        out.write(json.dumps(result))

    # for scale in [0.1, 0.25, .5, .75, 1.0]:
    #     print(scale)
    #     runLargeTests(["mode=single_opt"], scale)
    #     runLargeTests(["mode=multi"], scale)

if __name__ == '__main__':
    main()
