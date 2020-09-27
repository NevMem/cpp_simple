import sys
import os
import subprocess as sp

def make():
    proc = sp.Popen(["cmake", "."], stdout=sp.PIPE)
    proc.wait()

    proc = sp.Popen(["make"], stdout=sp.PIPE)
    result = proc.wait()
    if result != 0:
        sys.exit(1)

def read_file(file):
    with open(file, 'r') as inp:
        output_answer = int(inp.readline())
        output_parts = sorted(list(map(int, inp.readline().split())))
        return output_answer, output_parts

def check(output_file, real_file):
    output_answer, output_parts = read_file(output_file)
    real_answer, real_parts = read_file(real_file)
    if output_answer == real_answer and output_parts == real_parts:
        # print("ok")
        return True
    else:
        if output_answer != real_answer:
            print("Answers doesn't match, found:", output_answer, "expected:", real_answer)
            print(output_parts, real_parts)
        else:
            print('Parts doesn`t match')
            print(output_parts, real_parts)
        return False
   

def runSmallTests():
    test_dir = '/Users/yaigor/Desktop/lab_1/small_tests'
    input_files = []
    output_files = []
    for file in os.listdir(test_dir):
        if os.path.isfile(test_dir + '/' + file):
            if file[-3:] == '.in':
                input_files.append(file)
                output_files.append(file[:-3] + '.out')

    oks = 0

    for (input_file, output_file) in zip(input_files, output_files):
        args = ["./result.o"]
        proc = sp.Popen(args, stdin=open(test_dir + "/" + input_file, 'r'), stdout=open("output.txt", 'w'))
        proc.wait()

        oks += check("output.txt", test_dir + "/" + output_file)
    print(oks, '/', len(input_files))


def runLargeTests():
    test_dir = '/Users/yaigor/Desktop/lab_1/tests_100'
    input_files = []
    for file in os.listdir(test_dir):
        if os.path.isfile(test_dir + '/' + file):
            if file[-3:] == '.in':
                input_files.append(file)

    for input_file in input_files:
        args = ["./result.o"]
        proc = sp.Popen(args, stdin=open(test_dir + "/" + input_file, 'r'), stdout=open("output.txt", 'w'))
        try:
            proc.wait(1)
            print("Done")
        except:
            print("Timeout")
            proc.terminate()

def main():
    make()
    runSmallTests()
    runLargeTests()

if __name__ == '__main__':
    main()
