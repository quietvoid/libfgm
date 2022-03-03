/************************************************************************
 * Copyright 2022 MulticoreWare, Inc.
 *
 * Author: Keshav E <keshav@multicorewareinc.com>
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

#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Gradient.h"

using namespace cv;
class CannyEdgeDetector
{
    int lowThreshold = 0;
    int highThreshold = 0;
    int bit_10 = 0;
    Mat nonMaxSuppression(const Mat& magnitude, const Mat& slope);
    Mat doubleThreshold(const Mat& src);
    Mat Hysteresis(const Mat& src);

  public:
    int Apply(const Mat& srcImage, Mat& dstImage);
    CannyEdgeDetector(int lowThreshold, int hightThreshold, int bit_10 = 0)
    {
        this->lowThreshold = lowThreshold;
        this->highThreshold = hightThreshold;
        this->bit_10 = bit_10;
    };
    ~CannyEdgeDetector() {};
};
