import os
import numpy as np
import cv2

for fpath in os.listdir("./"):
    if (fpath.endswith(".txt")):

        f = open(os.path.join("./", fpath), 'r')
        txt = f.read()
        f.close()
        print(txt)

        name = fpath.replace(".txt", "")
        maxLen = 0
        numLines = 0
        for line in txt.split('\n'):
            numLines += 1
            lineLen = 0
            for ch in line:
                lineLen += 1
            if (lineLen > maxLen):
                maxLen = lineLen
                
        img = np.zeros((numLines, maxLen))
        i = 0

        for line in txt.split('\n'):
            j = 0
            for ch in line:
                # print(ord(ch))
                if (ch == '1'):
                    img[i][j] = 255
                elif (ch == '5'):
                    img[i][j] = 128
                else:
                    img[i][j] = ord(ch)
                j += 1
            i += 1

        print(fpath, img.shape)
        cv2.imwrite(os.path.join("./", name + ".png"), img)