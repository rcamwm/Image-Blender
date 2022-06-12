#include <iostream>
#include <cmath>
#include <cstring>
#include <ctype.h>
using namespace std;

typedef unsigned short WORD; // 2 Bytes
typedef unsigned int DWORD; // 4 Bytes
typedef unsigned int LONG; // 4 Bytes
typedef unsigned char BYTE; // 1 Byte

struct tagBITMAPFILEHEADER { // 16 Bytes
    WORD bfType; //specifies the file type
    DWORD bfSize; //specifies the size in bytes of the bitmap file
    WORD bfReserved1; //reserved; must be 0
    WORD bfReserved2; //reserved; must be 0
    DWORD bfOffBits; //specifies the offset in bytes from the bitmapfileheader to the bitmap bits
};

struct tagBITMAPINFOHEADER { // 40 Bytes
    DWORD biSize; //specifies the number of bytes required by the struct
    LONG biWidth; //specifies width in pixels
    LONG biHeight; //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage; //size of image in bytes
    LONG biXPelsPerMeter; //number of pixels per meter in x axis
    LONG biYPelsPerMeter; //number of pixels per meter in y axis
    DWORD biClrUsed; //number of colors used by th ebitmap
    DWORD biClrImportant; //number of colors that are important
};

struct pixel { // 3 Bytes - no padding due to same data type members
    BYTE blue;
    BYTE green;
    BYTE red;
};

class bmp {
    public:
        bmp(char *filename);
        ~bmp();
        bmp(const bmp& ogBMP); // Copy constructor

        void writeToFile(char *filename);

        static bmp combine(const bmp &bmpOne, const bmp &bmpTwo, const float ratio);
        static void printManual();

    private:
        static const int BLUE_OFFSET = 0, GREEN_OFFSET = 1, RED_OFFSET = 2; // BGR

        tagBITMAPFILEHEADER fileHeader;
        tagBITMAPINFOHEADER infoHeader;
        BYTE *imageData;
        LONG rowSize;
        LONG imageDataSize;

        bmp();
        bmp(tagBITMAPFILEHEADER newFileHeader, tagBITMAPINFOHEADER newInfoHeader);
        FILE* openFile(const char *filename, const char *readWrite) const;
        bool filenameIsDotBMP(const char *filename) const;
        void fillFileHeader(FILE *file);
        void fillInfoHeader(FILE *file) { fread(&infoHeader, sizeof(infoHeader), 1, file); }
        void setRowSize() { rowSize = 4 * ceil(8 * sizeof(pixel) * infoHeader.biWidth/32.0); };
        void setImageDateSize() { imageDataSize = (rowSize * infoHeader.biHeight) + 2; }
        void fillImageData(FILE *file);

        static bmp combineImageData(const bmp &largeBMP, const bmp &smallBMP, const float ratio);
        pixel getColorAt(const float x, const float y) const;
        pixel interpolateColor(const float x, const float y) const;
        pixel mixColors(const pixel colorOne, const pixel colorTwo, const float ratio) const;
        void changeColorAt(const int index, pixel newColor);
};

bmp::bmp(char *filename) {
    FILE *bmpFile = openFile(filename, "rb");
    fillFileHeader(bmpFile);
    fillInfoHeader(bmpFile);
    setRowSize();
    setImageDateSize();
    fillImageData(bmpFile);
    fclose(bmpFile);
}

bmp::~bmp() {
    delete[] imageData;
}

bmp::bmp(const bmp& ogBMP) {
    fileHeader = ogBMP.fileHeader;
    infoHeader = ogBMP.infoHeader;
    setRowSize();
    setImageDateSize();
    imageData = new BYTE[imageDataSize];
    copy(ogBMP.imageData, ogBMP.imageData + imageDataSize, imageData);
}

void bmp::writeToFile(char *filename) {
    FILE *outputFILE = openFile(filename, "wb");

    // File Header
    fwrite(&fileHeader.bfType, sizeof(fileHeader.bfType), 1, outputFILE);
    fwrite(&fileHeader.bfSize, sizeof(fileHeader.bfSize), 1, outputFILE);
    fwrite(&fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1), 1, outputFILE);
    fwrite(&fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2), 1, outputFILE);
    fwrite(&fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits), 1, outputFILE);

    // Info Header
    fwrite(&infoHeader, sizeof(infoHeader), 1, outputFILE);

    // Image Data
    fwrite(imageData, imageDataSize, 1, outputFILE);

    fclose(outputFILE);
}

bmp bmp::combine(const bmp &bmpOne, const bmp &bmpTwo, const float ratio) {
    if (bmpOne.infoHeader.biWidth >= bmpTwo.infoHeader.biWidth) {
        return combineImageData(bmpOne, bmpTwo, ratio);
    }
    return combineImageData(bmpTwo, bmpOne, 1 - ratio);
}

void bmp::printManual() {
    cout << "Please include the following parameters:" << endl;
    cout << "[program name] [image file 1] [image file 2] [ratio] [output file]" << endl;
}

// Private constructor for bmp::combineImageData() function
bmp::bmp(tagBITMAPFILEHEADER newFileHeader, tagBITMAPINFOHEADER newInfoHeader) {
    fileHeader = newFileHeader;
    infoHeader = newInfoHeader;
    setRowSize();
    setImageDateSize();
    imageData = new BYTE[imageDataSize];
    imageData[imageDataSize - 1] = 0;
    imageData[imageDataSize - 2] = 0;
}

FILE* bmp::openFile(const char *filename, const char *readWrite) const {
    const char *wb = "wb";
    if (filenameIsDotBMP(filename) || readWrite == wb) {
        FILE *file = fopen(filename, readWrite);
        if (!file) {
            cout << "Error: Cannot open file: \"" << filename << "\"" << endl;
            cout << "Please check that the filename is spelled correctly ";
            cout << "and that the file is not currently in use by another program." << endl;
            printManual();
            exit(1);
        }
        return file;
    }
    cout << "Error: Cannot open file: \"" << filename << "\"" << endl;
    cout << "Expected file with .bmp filename extension." << endl;
    printManual();
    exit(1);
}

bool bmp::filenameIsDotBMP(const char *filename) const {
    int length = 0;
    while (filename[length] != 0) {
        length++;
    }
    if (length < 4) return 0;
    return (filename[length - 4] == '.'
        && tolower(filename[length - 3]) == 'b'
        && tolower(filename[length - 2]) == 'm' 
        && tolower(filename[length - 1]) == 'p');
}

void bmp::fillFileHeader(FILE *file) {
    fread(&fileHeader.bfType, sizeof(fileHeader.bfType), 1, file);
    fread(&fileHeader.bfSize, sizeof(fileHeader.bfSize), 1, file);
    fread(&fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1), 1, file);
    fread(&fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2), 1, file);
    fread(&fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits), 1, file);
}

void bmp::fillImageData(FILE *file) {
    imageData = new BYTE[imageDataSize];
    fread(imageData, imageDataSize, 1, file);
}

bmp bmp::combineImageData(const bmp &largeBMP, const bmp &smallBMP, const float ratio) {
    bmp combinedBMP(largeBMP.fileHeader, largeBMP.infoHeader);

    float heightRatio = smallBMP.infoHeader.biHeight / (float)largeBMP.infoHeader.biHeight;
    float widthRatio = smallBMP.infoHeader.biWidth / (float)largeBMP.infoHeader.biWidth;
    pixel thisColor, otherColor, combinedColor;

    for (int y = 0; y < largeBMP.infoHeader.biHeight; y++) {
        for (int x = 0; x < largeBMP.infoHeader.biWidth; x++) {
            thisColor = largeBMP.interpolateColor(x, y);
            otherColor = smallBMP.interpolateColor(x * widthRatio, y * heightRatio);

            combinedColor.blue = (thisColor.blue * ratio) + (otherColor.blue * (1- ratio));
            combinedColor.green = (thisColor.green * ratio) + (otherColor.green * (1- ratio));
            combinedColor.red = (thisColor.red * ratio) + (otherColor.red * (1- ratio));
            combinedBMP.changeColorAt((y * combinedBMP.rowSize) + (x * 3), combinedColor);
        }
    }
    return combinedBMP;
}

pixel bmp::interpolateColor(const float x, const float y) const {
    int x1 = floor(x), y1 = floor(y);
    int x2 = ceil(x), y2 = ceil(y);
    float dx = x - x1, dy = y - y1;
    
    pixel upperLeft = getColorAt(x1, y2), upperRight = getColorAt(x2, y2);
    pixel lowerLeft = getColorAt(x1, y1), lowerRight = getColorAt(x2, y1);
    pixel left = mixColors(upperLeft, lowerLeft, dy);
    pixel right = mixColors(upperRight, lowerRight, dy);
    return mixColors(left, right, dx);
}

pixel bmp::getColorAt(const float x, const float y) const {
    pixel color;
    int imageDataIndex = (y * rowSize) + (x * 3);
    if (imageDataIndex > imageDataSize - 3) { 
        if (y == infoHeader.biHeight) {
            imageDataIndex = ((y - 1) * rowSize) + (x * 3);
        }
    }
    if (imageDataIndex > imageDataSize - 3) { 
        cout << infoHeader.biWidth << " x " << infoHeader.biHeight << ": " << x << ", " << y << endl;
    }

    color.blue = imageData[imageDataIndex + BLUE_OFFSET];
    color.green = imageData[imageDataIndex + GREEN_OFFSET];
    color.red = imageData[imageDataIndex + RED_OFFSET];
    return color;
}

pixel bmp::mixColors(const pixel colorOne, const pixel colorTwo, const float ratio) const {
    pixel newColor;
    newColor.blue = (colorOne.blue * (1 - ratio)) + (colorOne.blue * ratio);
    newColor.green = (colorOne.green * (1 - ratio)) + (colorOne.green * ratio);
    newColor.red = (colorOne.red * (1 - ratio)) + (colorOne.red * ratio);
    return newColor;
}

void bmp::changeColorAt(const int index, pixel newColor) {
    imageData[index + BLUE_OFFSET] = newColor.blue;
    imageData[index + GREEN_OFFSET] = newColor.green;
    imageData[index + RED_OFFSET] = newColor.red;
}

void debugMode() {
    char filenameOne[] = "wolf.bmp";
    char filenameTwo[] = "lion.bmp";
    char outputFile[] = "debugTest.bmp";

    bmp imageOne(filenameOne);
    bmp imageTwo(filenameTwo);
    float ratio = 0.0;
    bmp combined = bmp::combine(imageOne, imageTwo, ratio);
    combined.writeToFile(outputFile);
}

int main(int argc, char* argv[]) {
    const bool DEBUG = false;
    if (DEBUG) debugMode();
    else {
        if (argc != 5) {
            cout << "Error: Insufficient number of arguments." << endl;
            bmp::printManual();
            return 1;
        }

        bmp imageOne(argv[1]);
        bmp imageTwo(argv[2]);

        float ratio;
        try {
            ratio = stof(argv[3]);
            if (ratio > 1 || ratio < 0)
                throw(ratio);
        }
        catch (const std::invalid_argument &ia) {
            cout << "Error: " << argv[3] << " not a valid argument. Expected argument of type float." << endl;
            bmp::printManual();
            return 1;
        }
        catch (float r) {
            cout << "Error: " << r << " not a valid argument. Expected argument within range of 0.0 to 1.0." << endl;
            bmp::printManual();
            return 1;
        }

        if (!strcmp(argv[4], argv[1]) || !strcmp(argv[4], argv[2])) {
            cout << "Error: Output file \"" << argv[4] << "\" should be different from input files." << endl; 
            bmp::printManual();
            return 1;
        }
        bmp combined = bmp::combine(imageOne, imageTwo, ratio);
        combined.writeToFile(argv[4]);
    }
    return 0;
}