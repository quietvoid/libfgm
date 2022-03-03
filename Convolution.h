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
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;
class Convolution
{
    float kernel[9];
    int kernelWidth = 0;
    int kernelHeight = 0;
    int bit_10=0;

  public:
    void SetKernel(int kernel[], int kWidth, int kHeight);
    int DoConvolution(const Mat& sourceImage, Mat& destinationImage);
    Convolution(int bit_10)
    {
        this->bit_10 = bit_10;
    };
    ~Convolution() {};
};
