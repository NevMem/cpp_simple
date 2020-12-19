from PIL import Image
import numpy as np


def get_RGB(image,text):
    img = Image.open(image)
    rgbimg = img.convert('RGB')
    A = []
    for i in range(np.shape(rgbimg)[0]):
        for j in range(np.shape(rgbimg)[1]):
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

    
def read_image(text,image):
    file = open(text,"r")
    w,h = map(int,file.readline().split())
    A = []
    for i in range(3):
        B = []
        for k in range(w):
            B.append(list(map(np.uint8,file.readline().split())))
        A.append(B)
    A = np.array(A)
    A = A.transpose(1,2,0)
    file.close()
    new_im = Image.fromarray(A,'RGB')
    new_im.save(image)


def main():
    pass

if __name__ == '__main__':
    main()
