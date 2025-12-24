from sys import argv
import os
from PIL import Image
from pyperclip import copy

def getArgPrefixIndex(prefix):
    for i in range(len(argv)):
        if argv[i] == "-" + prefix:
            return i
    
    return -1
    
def getOptionalArg(prefix, default):
    for i in range(len(argv)):
        if argv[i] == "-" + prefix:
            return argv[i + 1]
    
    prefixIndex = getArgPrefixIndex(prefix)
    if (prefixIndex == -1):
        return default
    
    return argv[prefixIndex + 1]

def getBooleanOpArg(prefix):
    return getArgPrefixIndex(prefix) != -1

def checkC(c):
    return max(min(c, 255), 0)

def main():
    alphaOffset = int(getOptionalArg("a", 0))
    makeWhiteTransp = getBooleanOpArg("t")
    cutOffWhite = getBooleanOpArg("s")
    toClipboard = getBooleanOpArg("c")

    def printif(*args, **kw):
        if toClipboard:
            print(*args, **kw)

    def printInfo():
        print("""Args:
1. Path to image
2. Height percent as C value
(optional):
-a Alpha offset
-t Change white to transparent
-s Cut off transparent space
-c copy text to clipboard instead of stdout""")

    try:
        if os.path.exists(argv[1]):
            imagePath = argv[1]
        else:
            printif("Not a valid path")
            exit(1)
    except:
        printInfo()
        exit(1)

    try:
        heightPercent = argv[2]
    except:
        printInfo()
        exit(1)

    image = Image.open(imagePath).convert("RGBA")

    width, height = image.size
    pixels = image.load()

    colsCutOffLeft = width
    colsCutOffRight = width
    rowsCutOffTop = 0
    rowsCutOffBottom = 0
    listOfRgba = list()
    for row in range(height):
        numWhiteLeft = 0
        numWhiteRight = 0
        listOfRgba.append(list())

        for col in range(width):
            rgba = list(pixels[col, row])
            rgba[3] += alphaOffset
            rgba[3] = checkC(rgba[3])
            listOfRgba[row].append(tuple(rgba))

            if cutOffWhite:
                # Keep track of white pixels
                if rgba[3] == 0:
                    numWhiteRight += 1
                    if numWhiteLeft == col:
                        numWhiteLeft += 1

                else:
                    numWhiteRight = 0
        
        if numWhiteLeft == width:
            # Cut off row
            if rowsCutOffTop == row:
                rowsCutOffTop += 1
            rowsCutOffBottom += 1
        else:
            rowsCutOffBottom = 0

        colsCutOffLeft = min(numWhiteLeft, colsCutOffLeft)
        colsCutOffRight = min(numWhiteRight, colsCutOffRight)  

    if cutOffWhite:
        # Cut off rows
        listOfRgba = listOfRgba[rowsCutOffTop : height - rowsCutOffBottom]
        height -= rowsCutOffBottom + rowsCutOffTop

        for i in range(len(listOfRgba)):
            listOfRgba[i] = listOfRgba[i][colsCutOffLeft : width - colsCutOffRight]
        
        width -= colsCutOffLeft + colsCutOffRight
        
        printif(f"removed rows: {rowsCutOffTop} top {rowsCutOffBottom} bottom")
        printif(f"removed cols: {colsCutOffLeft} left {colsCutOffRight} right")

        printif(f"new dimens: {width}x{height}")

    final = "\t{" + f"{width}, {height}, {heightPercent}, (rgba[]) " + "{\n\t\t"

    transpCount = 0
    for row in range(height):
        for col in range(width):
            rgba = listOfRgba[row][col]

            final += "{"
            if makeWhiteTransp and rgba[:3] == (255, 255, 255):
                final += "0,0,0,0"
                transpCount += 1
            else:
                final += ",".join(map(lambda c: str(c), rgba))
            final += "}" + ("" if row == height - 1 and col == width - 1 else ",")

    final += "\n\t}}"
    if makeWhiteTransp:
        printif(f"{transpCount} pixels made transparent")
    
    #final = f"\t#if !__VSCODE__\n{final}\n\t#endif"
        
    if toClipboard:
        copy(final)
        print("done: copied")
    else:
        print(final, end="")

if __name__ == "__main__":
    main()