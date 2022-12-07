# LSB-Image-Steganography-on-.bmp-file
Implementing steganography (the art and science of hiding information) using LSB mechanism in C programming on a .bmp image file.

This is a command-line application that accepts an image file .bmp along with a text file that contains the secret message to be steged
and gives steged image as output. This application can also be used to decode the secret message by giving the steged image as input and we will get the secret message as output. 

For encoding, first it analyzes the size of the message file to check whether the message could fit in the provided .bmp image. Then a magic string is encoded to the steged image which could be useful to identify whether the image is steged or not while decoding, its followed by encoding the secret message and a steged .bmp file is given as output.

For decoding, first the magic string is decoded and checked, if the magic string matches then proceeds further and decodes the secret message. We will get the secret message as output file.

Sample Input    :   For encoding:
                    ./a.out -e <image.bmp> <secret file.txt or .c or .sh> <steged image name.bmp (optional)>
                    For decoding:
                    ./a.out -d <steged image.bmp> <decoded file name.txt or .c or .sh (optional)>
                    
Sample Output   :   Encoding:
                    Data will be encoded in a .bmp file created as ouput
                    Decoding:
                    Data will be decoded to a .txt or .sh or .c file as per input given
