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

#include "CannyEdgeDetector.h"

Mat CannyEdgeDetector::nonMaxSuppression(const Mat& magnitude, const Mat& slope)
{
    Mat result = magnitude.clone();
    int row, col, nRow, nCol;
    nRow = magnitude.rows;
    nCol = magnitude.cols;
    float q, r;
    /* For 8 bit input */
    if (this->bit_10 == 0)
    {
        for (row = 1; row < nRow - 1; row++)
        {
            for (col = 1; col < nCol - 1; col++)
            {
                q = r = 255;
                if (slope.at<uchar>(row, col) == 0) // pixel right and left
                {
                    q = magnitude.at<float>(row, col + 1);
                    r = magnitude.at<float>(row, col - 1);
                }
                else if (slope.at<uchar>(row, col) == 45) // pixel above_left and bottom_right
                {
                    q = magnitude.at<float>(row + 1, col - 1);
                    r = magnitude.at<float>(row - 1, col + 1);
                }
                else if (slope.at<uchar>(row, col) == 90) // pixel above and bottom
                {
                    q = magnitude.at<float>(row + 1, col);
                    r = magnitude.at<float>(row - 1, col);
                }
                else if (slope.at<uchar>(row, col) == 135) // pixel above_right and bottom_left
                {
                    q = magnitude.at<float>(row - 1, col - 1);
                    r = magnitude.at<float>(row + 1, col + 1);
                }

                if (magnitude.at<float>(row, col) >= q && magnitude.at<float>(row, col) >= r)
                    result.at<float>(row, col) = magnitude.at<float>(row, col);
                else
                    result.at<float>(row, col) = 0;
            }
        }
    }
    /* For 10 bit input */
    else
    {
        for (row = 1; row < nRow - 1; row++)
        {
            for (col = 1; col < nCol - 1; col++)
            {
                q = r = 1023;
                if (slope.at<ushort>(row, col) == 0) // pixel right and left
                {
                    q = magnitude.at<float>(row, col + 1);
                    r = magnitude.at<float>(row, col - 1);
                }
                else if (slope.at<ushort>(row, col) == 45) // pixel above_left and bottom_right
                {
                    q = magnitude.at<float>(row + 1, col - 1);
                    r = magnitude.at<float>(row - 1, col + 1);
                }
                else if (slope.at<ushort>(row, col) == 90) // pixel above and bottom
                {
                    q = magnitude.at<float>(row + 1, col);
                    r = magnitude.at<float>(row - 1, col);
                }
                else if (slope.at<ushort>(row, col) == 135) // pixel above_right and bottom_left
                {
                    q = magnitude.at<float>(row - 1, col - 1);
                    r = magnitude.at<float>(row + 1, col + 1);
                }

                if (magnitude.at<float>(row, col) >= q && magnitude.at<float>(row, col) >= r)
                    result.at<float>(row, col) = magnitude.at<float>(row, col);
                else
                    result.at<float>(row, col) = 0;
            }
        }
    }


    return result;
}

Mat CannyEdgeDetector::doubleThreshold(const Mat& src)
{
    Mat result;
    if (this->bit_10 == 0)
        result = Mat::zeros(src.size(), CV_8UC1);
    else
        result = Mat::zeros(src.size(), CV_16UC1);
    int row, col, nRow, nCol;
    nRow = src.rows;
    nCol = src.cols;
    float q, r;
    for (row = 0; row < nRow; row++)
    {
        for (col = 0; col < nCol; col++)
        {
            if (src.at<float>(row, col) >= this->highThreshold){
                if (this->bit_10 == 0)
                    result.at<uchar>(row, col) = 255 ;
                else
                    result.at<ushort>(row, col) = 1023 ;
            }
            else if (src.at<float>(row,col) < this->highThreshold && src.at<float>(row,col) >= this->lowThreshold){
                if (this->bit_10 == 0)
                    result.at<uchar>(row, col) = 25 ;
                else
                    result.at<ushort>(row, col) = 100 ;
            }
        }
    }
    return result;
}

Mat CannyEdgeDetector::Hysteresis(const Mat& src)
{
    Mat result = src.clone();
    int row, col, nRow, nCol;
    nRow = src.rows;
    nCol = src.cols;
    float q, r;
    /* For 8 bit input */
    if (this->bit_10 == 0)
    {
        for (row = 1; row < nRow - 1; row++)
        {
            for (col = 1; col < nCol - 1; col++)
            {
                if (src.at<uchar>(row,col) == 25)
                {
                    if ((src.at<uchar>(row + 1, col - 1) == 255) || (src.at<uchar>(row + 1, col) == 255) || (src.at<uchar>(row + 1, col + 1) == 255) || (src.at<uchar>(row, col - 1) == 255)
                        || (src.at<uchar>(row, col + 1) == 255) || (src.at<uchar>(row - 1, col - 1) == 255) || (src.at<uchar>(row - 1, col) == 255) || (src.at<uchar>(row - 1, col + 1) == 255))
                        result.at<uchar>(row, col) = 255;
                    else
                        result.at<uchar>(row, col) = 0;
                }
            }
        }
    }
    /* For 10 bit input */
    else
    {
        for (row = 1; row < nRow - 1; row++)
        {
            for (col = 1; col < nCol - 1; col++)
            {
                if (src.at<ushort>(row,col) == 100)
                {
                    if ((src.at<ushort>(row + 1, col - 1) == 1023) || (src.at<ushort>(row + 1, col) == 1023) || (src.at<ushort>(row + 1, col + 1) == 1023) || (src.at<ushort>(row, col - 1) == 1023)
                        || (src.at<ushort>(row, col + 1) == 1023) || (src.at<ushort>(row - 1, col - 1) == 1023) || (src.at<ushort>(row - 1, col) == 1023) || (src.at<ushort>(row - 1, col + 1) == 1023))
                        result.at<ushort>(row, col) = 1023;
                    else
                        result.at<ushort>(row, col) = 0;
                }
            }
        }
    }

    return result;
}

int CannyEdgeDetector::Apply(const Mat& srcImage, Mat& dstImage)
{
    if (srcImage.data == NULL)
        return 1;

    Mat src;
    if (srcImage.channels() > 1)
        cvtColor(srcImage, src, COLOR_RGB2GRAY);
    else
        src = srcImage.clone();

    /* Noise reduction */
    GaussianBlur(src, dstImage, Size(5, 5), BORDER_DEFAULT);

    /* Gradient calculation */
    Mat magnitude, slope;
    Gradient gradient(this->bit_10);
    gradient.getMagnitudeAndSlope(dstImage, magnitude, slope, 1);

    /* Non-maximum suppression */
    dstImage = nonMaxSuppression(magnitude, slope);

    /* Double threshold */
    dstImage = doubleThreshold(dstImage);

    /* Edge Tracking by Hysteresis */
    dstImage = Hysteresis(dstImage);

    return 0;
}
