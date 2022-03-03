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

#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "yuv.h"
#include <iostream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define SC_N_DIFFS 5
class KalmanFiltr;

typedef struct _Scenechange
{
    int n_diffs;
    double diffs[SC_N_DIFFS];
    int count;
} Scenechange;

class KalmanFiltr
{
  public:
    KalmanFiltr(Mat firstFrame, float q, float r, int maskSize, int d, double sigmaValue, int bits);
    Mat KalmanFiltredFrame(const Mat &frame, int bits);
    int frameCount;
    double bilateralTime, blurTime, modifiedKalmanTime;
    Mat xPredicted;
    Mat pPredicted;
    Mat xCorrection;
    Mat pCorrection;
    Mat K;
    Mat previous_blured;
    Mat current_blured;
    Mat floatFrame;
    Mat delta_Q;

    Mat bilateral_or_fastguided_filtered;
    Mat kalman_filterd;
    Mat R;

    float q;
    double sigmas;
    int d;
    int maskSize;
    int height, width;
};

bool is_change(Mat* oldframe, Mat* frame, Scenechange* sc);

#endif