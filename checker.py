import sys
import os
import subprocess as sp
import time

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
    test_dir = '/Users/yaigor/Desktop/lab_1/small_tests'
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


def runLargeTests(runParams=[]):
    test_dir = '/Users/yaigor/Desktop/lab_1/tests_100'
    input_files = []
    for file in os.listdir(test_dir):
        if os.path.isfile(test_dir + '/' + file):
            if file[-3:] == '.in':
                input_files.append(file)

    for input_file in input_files:
        args = ["./result.o"]
        for param in runParams:
            args.append(param)
        
        start = time.time()
        proc = sp.Popen(args, stdin=open(test_dir + "/" + input_file, 'r'), stdout=open("output.txt", 'w'))
        try:
            proc.wait(500)
            print("Done", time.time() - start)
        except:
            print("Timeout")
            proc.terminate()

def main():
    make()
    runs = 1

    runTime, oks = runSmallTests(runs, ["mode=single"])
    print(runTime, oks)

    runTime, oks = runSmallTests(runs, ["mode=single_opt"])
    print(runTime, oks)

    runTime, oks = runSmallTests(runs, ["mode=multi"])
    print(runTime, oks)

    runLargeTests(["mode=single_opt"])
    runLargeTests(["mode=single"])
    runLargeTests(["mode=multi"])

if __name__ == '__main__':
    main()
