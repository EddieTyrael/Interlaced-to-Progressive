//
// Created by Eddie,Joe and Lu on 2023/12/22.
//
#include "main.hpp"

int frameCounter = 0;
bool isEvenField = false;
auto *YUVBuffer = new unsigned char[nSize];

int main() {
    auto *preFrame = new unsigned char[nSize];
    auto *curFrame = new unsigned char[nSize];
    auto *nextFrame = new unsigned char[nSize];
    auto *Res = new unsigned char[nSize];

    ifstream fin;
    fin.open(filePath, ios::in | ios::binary);

    ofstream fo1;
    ofstream fo2;
    ofstream fo3;
    ofstream fo4;
    fo1.open("spaceInter.yuv", ios::out | ios::ate | ios::app | ios::binary);
    fo2.open("timeInter.yuv", ios::out | ios::ate | ios::app | ios::binary);
    fo3.open("spTimeInter.yuv", ios::out | ios::ate | ios::app | ios::binary);
    fo4.open("non_linear.yuv", ios::out | ios::ate | ios::app | ios::binary);

    printf("converting...\n");

    while (!fin.eof()) {
        delete[](preFrame);
        preFrame = curFrame;
        curFrame = nextFrame;
        nextFrame = new unsigned char[nSize];
        auto tmpFrame = new unsigned char[nSize];
        auto tmpFrame_time = new unsigned char[nSize];
        auto tmpFrame_space = new unsigned char[nSize];

        // Read a frame
        for (int m = 0; m < nSize; m++) {
            YUVBuffer[m] = fin.get();
            nextFrame[m] = YUVBuffer[m];
            tmpFrame[m] = curFrame[m];
            tmpFrame_space[m] = curFrame[m];
        }

        // Inherit color
        for (int i = 0; i < pixelCount / 2; i++)
            Res[pixelCount + i] = YUVBuffer[pixelCount + i];

        /**
         * Space Interpolator
         */
        if (frameCounter >= 1)
            spaceInter(tmpFrame_space, isEvenField);

        if (frameCounter != 300) {  // Last frame deleted
            // Putout image
            for (int i = 0; i < Height; i++) {
                for (int j = 0; j < Width; j++)
                    fo1 << (unsigned char) tmpFrame_space[i * Width + j];
            }
            for (int i = 0; i < pixelCount / 2; i++)
                fo1 << (unsigned char) tmpFrame_space[pixelCount + i];
        }

        // Read a frame
        for (int m = 0; m < nSize; m++)
            tmpFrame_time[m] = curFrame[m];

        /**
         * Time Interpolator
         */
        if (frameCounter >= 2)
            timeInter(preFrame, tmpFrame_time, nextFrame, isEvenField);

        if (frameCounter != 300) {
            for (int i = 0; i < Height; i++) {
                for (int j = 0; j < Width; j++)
                    fo2 << (unsigned char) tmpFrame_time[i * Width + j];
            }
            for (int i = 0; i < pixelCount / 2; i++)
                fo2 << (unsigned char) tmpFrame_time[pixelCount + i];
        }

        for (int m = 0; m < nSize; m++) {
            tmpFrame[m] = curFrame[m];
        }

        /**
         * Space-and-time Hybrid Interpolator
         */
        if (frameCounter >= 2)
            spTimeInter(preFrame, tmpFrame, nextFrame, isEvenField);

        if (frameCounter != 300) {
            for (int i = 0; i < Height; i++) {
                for (int j = 0; j < Width; j++)
                    fo3 << (unsigned char) tmpFrame[i * Width + j];
            }

            for (int i = 0; i < pixelCount / 2; i++)
                fo3 << (unsigned char) tmpFrame[pixelCount + i];
        }

        /**
         *  Filter with motion detection
         */
        double motion = 10 * motionDetection(preFrame, curFrame, nextFrame, isEvenField);
        if (motion >= 1) motion = 0.99;

        for (int m = 0; m < nSize; m++)
            tmpFrame[m] = (u8) ((double) tmpFrame_time[m] * (1 - motion) + (double) tmpFrame_space[m] * (motion));

        if (frameCounter != 300) {
            for (int i = 0; i < Height; i++) {
                for (int j = 0; j < Width; j++) {
                    if (i >= Height - 20 && j >= Width - 20) tmpFrame[i * Width + j] = (u8) (motion * 255);
                    fo4 << (unsigned char) tmpFrame[i * Width + j];
                }
            }
            for (int i = 0; i < pixelCount / 2; i++)
                fo4 << (unsigned char) tmpFrame[pixelCount + i];
        }

        isEvenField = !isEvenField;
        frameCounter++;
    }

    fo1.close();
    fo2.close();
    fo3.close();
    fo4.close();

    return 0;

}

void spaceInter(unsigned char *curFrame, bool evenOdd) {
    double g1, g2, g3, g4;
    for (int i = 0; i < Height; i++)
        for (int j = 0; j < Width; j++) {
            if ((i % 2) != evenOdd) {
                if (i == 0 || i == (Height - 1))
                    continue;
                else if (i == 1 || i == (Height - 2)) {
                    g2 = curFrame[(i - 1) * Width + j];
                    g3 = curFrame[(i + 1) * Width + j];
                    curFrame[i * Width + j] = (u8) ((g2 + g3) * 0.5);
                } else {
                    g1 = curFrame[(i - 2) * Width + j];
                    g2 = curFrame[(i - 1) * Width + j];
                    g3 = curFrame[(i + 1) * Width + j];
                    g4 = curFrame[(i + 2) * Width + j];
                    curFrame[i * Width + j] = (u8) (0.0625 * (g1 + g4) + 0.4375 * (g2 + g3));
                }
            }
        }
}

void timeInter(const u8 *preFrame, u8 *curFrame, const u8 *nextFrame, bool evenOdd) {
    double g1, g2;
    for (int i = 0; i < Height; i++)
        for (int j = 0; j < Width; j++) {
            if ((i % 2) != evenOdd) {
                g1 = preFrame[i * Width + j];
                g2 = nextFrame[i * Width + j];
                curFrame[i * Width + j] = (u8) ((g1 + g2) * 0.5);
            }
        }
}

void spTimeInter(const u8 *preFrame, u8 *curFrame, const u8 *nextFrame, bool evenOdd) {
    double g1,g2,g3,g4;
    for (int i = -1; i < Height; i++)
        for (int j = 0; j < Width; j++) {
            if ((i % 2) != evenOdd) {
                if (i == 0 || i == (Height - 1)) {
                    curFrame[i * Width + j] = (u8) ((g1 + g2) * 0.5);
                } else {
                    g1 = curFrame[(i - 1) * Width + j];
                    g2 = curFrame[(i + 1) * Width + j];
                    g3 = preFrame[i * Width + j];
                    g4 = nextFrame[i * Width + j];
                    curFrame[i * Width + j] = (u8) ((g1 + g2 + g3 + g4) * 0.25);
                }
            }
        }
}

double motionDetection(const u8 *preFrame, const u8 *curFrame, const u8 *nextFrame, bool evenOdd) {
    double g1, g2, g3, g4;
    double T1 = 40, T2 = 40;
    double M;
    int sum = 0;
    for (int i = 0; i < Height; i++)
        for (int j = 0; j < Width; j++) {
            if ((i % 2) != evenOdd) {
                if (i == 0 || i == (Height - 1)) {
                    g3 = preFrame[i * Width + j];
                    g4 = nextFrame[i * Width + j];
                    if (abs(g3 - g4) >= T1) {
                        sum++;
                    }
                } else {
                    g1 = curFrame[(i - 1) * Width + j];
                    g2 = curFrame[(i + 1) * Width + j];
                    g3 = preFrame[i * Width + j];
                    g4 = nextFrame[i * Width + j];
                    if (abs(g3 - g4) >= T1 || abs((g3 + g4) * 0.5 - (g1 + g2) * 0.5) >= T2)
                        sum++;
                }
            }
        }
    M = sum / 144.0 / Width;
    return M;
}