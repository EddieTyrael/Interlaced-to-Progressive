//
// Created by Eddie,Joe and Lu on 2023/12/22.
//

#ifndef FINAL_MAIN_H
#define FINAL_MAIN_H

#include <iostream>
#include <fstream>

using namespace std;

typedef unsigned char u8;

// Parameter List
const int Height = 288;
const int Width = 352;
const int pixelCount = 352 * 288;
const int nSize = pixelCount + pixelCount / 2;  // Data scale of a pixel by 4:2:0 Sampling
const string filePath = "coastguard_cif_interlaced.yuv";

/**
  Change odd rows when evenOdd=0, otherwise change even rows. \n
  Each Pointer points to a single full-scale image.
**/
/**
 * Time-domain interpolator
 * @param preFrame : The previous frame.
 * @param curFrame : The current frame.
 * @param nextFrame : The next frame.
 * @param evenOdd : The isEvenField of field in interlaced video(odd field/even field)
 */
void timeInter(const u8 *preFrame, u8 *curFrame, const u8 *nextFrame, bool evenOdd);

/**
 * Space-domain interpolator
 */
void spaceInter(u8 *curFrame, bool evenOdd);

/**
 * Space-and-time-domain interpolator
 */
void spTimeInter(const u8 *preFrame, u8 *curFrame, const u8 *nextFrame, bool evenOdd);

/**
 * Filter with motion detection
 * @return Intensity of moving
 */
double motionDetection(const u8 *preFrame, const u8 *curFrame, const u8 *nextFrame, bool evenOdd);

#endif // FINAL_MAIN_H