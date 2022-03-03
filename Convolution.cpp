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

#include "Convolution.h"

void Convolution::SetKernel(int kernel[], int kWidth, int kHeight)
{
    int i;
    this->kernelHeight = kHeight;
    this->kernelWidth = kWidth;
    for (i = 0; i < 9; i++)
        this->kernel[i] = kernel[i];
}

int Convolution::DoConvolution(const Mat& sourceImage, Mat& destinationImage)
{
    if (sourceImage.data == NULL)
        return 1;

    int row, col, nRow, nCol, sum;
    nRow = sourceImage.rows;
    nCol = sourceImage.cols;
    int indexR, indexC, i, j, count, value;

    /* Initialize dst matrix */
    destinationImage = Mat(sourceImage.size(), CV_32FC1);

    /* Do convolution */
    for (row = 0; row < nRow; row++)
    {
        for (col = 0; col < nCol; col++)
        {
            sum = 0;
            count = 0;
            for (i = -(this->kernelWidth/2); i <=(this->kernelWidth/2); i++)
            {
                for (j = -(this->kernelHeight/2); j <= (this->kernelHeight/2); j++)
                {
                    indexR = row - i;
                    indexC = col - j;
                    if ((indexR < 0 || indexR > nRow - 1) || (indexC < 0 || indexC > nCol - 1))
                        value = 0;
                    else
                    {
                        if (this->bit_10 == 0)
                            value = sourceImage.at<uchar>(indexR, indexC);
                        else
                            value =  sourceImage.at<ushort>(indexR, indexC);
                    }
                    sum += this->kernel[count] * value;
                    count++;
                }
            }
            destinationImage.at<float>(row, col) = sum;
        }
    }
    return 0;
}
