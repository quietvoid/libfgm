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

#include "Gradient.h"


void Gradient::gradientOperator(const Mat& src, Mat& Gx, Mat& Gy, int xArr[], int yArr[])
{

    Convolution convolutionX(this->bit_10), convolutionY(this->bit_10);
    convolutionX.SetKernel(xArr, 3, 3);
    convolutionY.SetKernel(yArr, 3, 3);

    /* Do convolution */
    convolutionX.DoConvolution(src, Gx);
    convolutionY.DoConvolution(src, Gy);
}

Mat Gradient::magnitudeCalculation(const Mat& Gx, const Mat& Gy)
{
    Mat magnitude = Mat(Gx.size(), CV_32FC1);

    int row, col, nRow, nCol;
    nRow = Gx.rows;
    nCol = Gx.cols;
    double x, y;
    /* calculating magnitude */
    for (row = 0; row < nRow; row++)
    {
        for (col = 0; col < nCol; col++)
        {
            x = Gx.at<float>(row, col);
            y = Gy.at<float>(row, col);
            magnitude.at<float>(row, col) = sqrt(x * x + y * y);
        }
    }

    return magnitude;
}

Mat Gradient::slopeCalculation(const Mat& Gx, const Mat& Gy)
{
    Mat slope;
    if(this->bit_10 == 0)
        slope = Mat(Gx.size(), CV_8UC1);
    else
        slope = Mat(Gx.size(), CV_16UC1);
    int row, col, nRow, nCol;
    nRow = Gx.rows;
    nCol = Gx.cols;
    double x, y;
    float theta, PI = 3.141592;

    /* calculating slope */
    for (row = 0; row < nRow; row++)
    {
        for (col = 0; col < nCol; col++)
        {
            x = Gx.at<float>(row, col);
            y = Gy.at<float>(row, col);
            theta = atan2f(y, x) * 180 / PI;
            if (theta < 0)
                theta += 180;
            if ((0 <= theta && theta < 22.5) || (157.5 <= theta && theta <= 180))
                theta = 0;
            else if (22.5 <= theta && theta < 67.5)
                theta = 45;
            else if (67.5 <= theta && theta < 112.5)
                theta = 90;
            else if (112.5 <= theta && theta < 157.5)
                theta = 135;
        if (this->bit_10 == 0)
            slope.at<uchar>(row, col) = theta;
        else
            slope.at<ushort>(row, col) = theta;

        }
    }

    return slope;
}

int Gradient::getMagnitudeAndSlope(const Mat& sourceImage, Mat& magnitude, Mat& slope, int method)
{
    if (sourceImage.data == NULL)
        return 1;

    Mat src;
    if (sourceImage.channels() > 1)
        cvtColor(sourceImage, src, COLOR_RGB2GRAY);
    else
        src = sourceImage.clone();

    Mat grayX, grayY;
    int xArr[9] = { -1 , 0 , 1, -2 , 0 , 2, -1 , 0 , 1 };
    int yArr[9] = { 1 , 2 , 1, 0 , 0 , 0, -1 , -2 , -1 };
    gradientOperator(src, grayX, grayY, xArr, yArr);
    magnitude = magnitudeCalculation(grayX, grayY);
    slope = slopeCalculation(grayX, grayY);
    return 0;
}
