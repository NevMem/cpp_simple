from PIL import Image
import numpy as np
import subprocess as sp
import os


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
    process = sp.Popen(
        [file, *args],
        stdout=open(output_filename, 'w'),
        stderr=open('log.txt', 'w'))
    process.wait()
    assert process.returncode == 0
    return output_filename


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


def main():
    images = ['image_qhd.tiff', 'image_4k.tiff', 'image_8k.tiff']

    converter = Converter()

    run_cmake()
    filename = build(is_debug=False)

    for image in images:
        converted = converter.convert(image)
        output = run(filename, [converted])
        read_image(output, image.split('.')[0] + '_result.tiff')

if __name__ == '__main__':
    main()
