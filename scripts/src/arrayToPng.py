import tkinter as tk
from json import loads as getJson
import sys

def display(root, image):
    # Create canvas
    canvas = tk.Canvas(root, width=image.width() * 2, height=image.height() * 2)
    
    canvas.create_image(0, 0, image=image, anchor=tk.NW)
    canvas.pack()

    root.mainloop()

def setPixel(image, x, y, color):
    image.put("{#%02x%02x%02x}" % color, (x, y))

def main():
    try:
        saveDir = sys.argv[1]
    except:
        print("Error must provide the file name and full path to save the file to")
        quit()
    
    # Get array as python 3d list
    with open("array.txt", "r") as f:
        fileText = f.read()
    
    fileText = fileText.replace("{", "[")
    fileText = fileText.replace("}", "]")

    fileAsList = getJson(fileText)
    
    # Create and fill image
    root = tk.Tk()
    image = tk.PhotoImage(master=root, width=50, height=50)

    for row in range(50):
        for col in range(50):
            setPixel(image, col, row, tuple(fileAsList[row][col]))
    
    image.write(saveDir, format="png")

if __name__ == "__main__":
    main()