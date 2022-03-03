/************************************************************************
 * Copyright 2022 MulticoreWare, Inc.
 *
 * Author: Keshav E <keshav@multicorewareinc.com>
 *         Pooja Venkatesan <pooja@multicorewareinc.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***********************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>

using namespace std;
using namespace cv;

#define CLIP3(min, max, x) (((x) > (max)) ? (max) :(((x) < (min))? (min):(x)))

class Detection
{
  public:
    void EdgeDetection(Mat data, Mat* canny, int bits);
    void NonEdge64x64Blocks(Mat canny, int* index, int* total);
  private:
    int canny_min_threshold, canny_max_threshold;
};

struct CompModelIntensityValues
{
    unsigned char intensityIntervalLowerBound;
    unsigned char intensityIntervalUpperBound;
    int compModelValue[6]; // sigma, h, v for frequency filtering
};

struct CompModel
{
    bool  bPresentFlag;
    unsigned char numModelValues; // this must be the same as intensityValues[*].compModelValue.size()
    unsigned char m_filmGrainNumIntensityIntervalMinus1;
    CompModelIntensityValues intensityValues[3]; // 3 set of intensity values for frequency filtering
};

struct SEIFilmGrainCharacteristics
{
  public:
    bool      m_filmGrainCharacteristicsCancelFlag;
    unsigned char     m_filmGrainModelId;
    bool      m_separateColourDescriptionPresentFlag;
    unsigned char     m_filmGrainBitDepthLumaMinus8;
    unsigned char     m_filmGrainBitDepthChromaMinus8;
    bool      m_filmGrainFullRangeFlag;
    unsigned char     m_filmGrainColourPrimaries;
    unsigned char     m_filmGrainTransferCharacteristics;
    unsigned char     m_filmGrainMatrixCoeffs;
    unsigned char     m_blendingModeId;
    unsigned char     m_log2ScaleFactor;
    struct CompModel  m_compModel[3]; // can be set for 3 if chroma is considered for film grain
    bool      m_filmGrainCharacteristicsPersistenceFlag;
};

struct intensityValues
{
    unsigned int* avg;
    uint32_t standardDeviation;
    uint32_t mean_diff;
    unsigned int min_intensity;
    unsigned int max_intensity;
    unsigned int model_intensity;
    unsigned int intensity_count[256];
};

int combined_avg(Mat &dst1, Mat&dst2, Mat& final_dst, char* filter, int dst_weight, int dst_kalman_weight, int bits);
void getResidual(Mat src1, Mat src2, Mat* difference_image, int* index, struct intensityValues* int_values, int bits);
void writeFilmGrainFrequencyModel(struct SEIFilmGrainCharacteristics filmgrain, FILE* fout);
#endif