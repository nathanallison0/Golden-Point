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

    if (image.width() != 50 or image.height() != 50):
        print(f"Cannot process image of dimensions ({image.width()}, {image.height()})")
        quit()
    
    arrayText = "{\n"
    for row in range(50):
        arrayText += "\t{"
        for col in range(50):
            colors = image.get(col, row)
            colors = tuple(map(lambda x: str(x).ljust(3), colors))
            r, b, g = colors
            arrayText += "{" + f"{r},{b},{g}" + "}" + ("" if col == 49 else ",")
        arrayText += "}" + ("" if row == 49 else ",") + "\n"
    arrayText += "}"
    
    print(arrayText)
    copy(arrayText)

if __name__ == "__main__":
    main()