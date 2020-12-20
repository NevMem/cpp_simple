from PIL import Image
import json
import matplotlib.pyplot as plt
import numpy as np
import os
import subprocess as sp


def run_cmake():
    process = sp.Popen(['cmake', '.'], stdout=sp.PIPE)
    process.wait()
    assert process.returncode == 0


def build(is_debug=True) -> str:
    print('Building')
    configuration = 'Debug'
    if not is_debug:
        configuration = 'Release'
    ret = sp.call(
        ['msbuild', 'main.sln', '/property:Configuration={}'.format(configuration)])
    assert ret == 0
    print('Built!')
    if is_debug:
        return 'Debug/result.exe'
    return 'Release/result.exe'


def run(file, args) -> str:
    output_filename = 'output.txt'
    log_filename = 'log.txt'
    process = sp.Popen(
        [file, *args],
        stdout=open(output_filename, 'w'),
        stderr=open(log_filename, 'w'))
    process.wait()
    assert process.returncode == 0
    return output_filename, log_filename


def get_RGB(image, text):
    img = Image.open(image)
    rgbimg = img.convert('RGB')
    A = []
    height = np.shape(rgbimg)[0]
    width = np.shape(rgbimg)[1]
    for i in range(height):
        for j in range(width):
            A.append(rgbimg.getpixel((j,i)))
    A = np.array(A)
    file1 = open(text,"w")
    file1.write(str(np.shape(rgbimg)[0]) + " " + str(np.shape(rgbimg)[1]) + "\n")
    for i in range(3):
        B = A[:, i].reshape(np.shape(rgbimg)[0], np.shape(rgbimg)[1])
        for k in range(B.shape[0]):
            for l in range(B.shape[1]):
                file1.write(str(B[k,l]) + " ");
            file1.write("\n")
    file1.close()

    
def read_image(text, image):
    file = open(text,"r")
    w,h = map(int, file.readline().split())
    A = []
    for i in range(3):
        B = []
        for k in range(w):
            B.append(list(map(np.uint8,file.readline().split())))
        A.append(B)
    A = np.array(A)
    A = A.transpose(1, 2, 0)
    file.close()
    new_im = Image.fromarray(A, 'RGB')
    new_im.save(image)


class Converter:
    def convert(self, image_name: str) -> str:
        """
        Converts image to txt file
        Uses cache to prevent long operation each time
        """
        name = image_name.split('.')[0]
        convert_name = name + '.txt'
        if os.path.exists(convert_name):
            return convert_name
        get_RGB(image_name, convert_name)
        return convert_name


def read_log(log_filename):
    transforming_time = 0
    count_trnasforms = 0
    with open(log_filename, 'r') as inp:
        for line in inp.readlines():
            split = line.split()
            if split[0] == '[Transforming]':
                transforming_time += int(split[1])
                count_trnasforms += 1
    return transforming_time / count_trnasforms


def main():
    images = ['image_720p.tiff', 'image_fullhd.tiff', 'image_qhd.tiff', 'image_4k.tiff']

    converter = Converter()

    run_cmake()
    filename = build(is_debug=False)

    for_graphs = dict()
    sizes = [1, 3, 5, 7, 11, 17, 29, 31]

    with open('graphs.json', 'r') as inp:
        for_graphs = json.loads(inp.read())

    for image in images:
        converted = converter.convert(image)
        for mode in ['cpu', 'cuda']:
            for size in sizes:
                output, log_file = run(filename, [converted, mode, str(size)])
                transforming_time = read_log(log_file)
                label = mode + ' ' + image
                if label not in for_graphs:
                    for_graphs[label] = []
                for_graphs[label].append(transforming_time)
                print(label, transforming_time)
        read_image(output, image.split('.')[0] + '_result.tiff')

    with open('graphs.json', 'w') as out:
        out.write(json.dumps(for_graphs))

    plt.figure(figsize=(14, 10))
    for label in for_graphs:
        plt.plot(sizes, for_graphs[label], label=label)
    plt.xlabel('Kernel size')
    plt.ylabel('Milliseconds on transformation')
    plt.legend()
    plt.savefig('graphics.png')


if __name__ == '__main__':
    main()
