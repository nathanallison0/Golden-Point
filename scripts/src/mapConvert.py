import tkinter as tk
import os, sys
from pyperclip import copy

try:
    imagePath = sys.argv[1]
except:
    print("Error: must provide a path to an image")
    quit()

if (not os.path.exists(imagePath)):
    print("Error: file does not exist")
    quit()

def main():
    root = tk.Tk()

    image = tk.PhotoImage(master=root, file=imagePath)

    width = image.width()
    height = image.height()

    startRow = 0
    endRow = height

    startCol = 0
    endCol = width

    getIsWall = lambda x, y, img: not all(img.get(x, y))

    # Check for columns that are entirely walls and offset start and end appropriately
    colIsAllWalls = lambda x: all(getIsWall(x, checkY, image) for checkY in range(height))

    while colIsAllWalls(startCol):
        startCol += 1
    
    while colIsAllWalls(endCol - 1):
        endCol -= 1
    
    # Do the same for rows
    rowIsAllWalls = lambda y: all(getIsWall(checkX, y, image) for checkX in range(width))

    while rowIsAllWalls(startRow):
        startRow += 1
    
    while rowIsAllWalls(endRow - 1):
        endRow -= 1
    
    finalWidth = endCol - startCol
    finalHeight = endRow - startRow

    # Convert, while checking for rows that are entire walls
    array = str()
    for row in range(startRow, endRow):
        array += "\t{"
        for col in range(startCol, endCol):
            isWall = getIsWall(col, row, image)
            array += ("1" if isWall else "0") + ("," if col != endCol - 1 else "")
        array += "}" + ("," if row != endRow - 1 else "") + "\n"
    
    print()
    print(array)
    
    print(f"Width: {finalWidth}")
    print(f"Height: {finalHeight}")

if __name__ == "__main__":
    main()