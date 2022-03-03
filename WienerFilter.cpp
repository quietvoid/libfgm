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

#include "WienerFilter.h"

using namespace cv;

double WienerFilter(const Mat& src, Mat& dst, int bits, const Size& block)
{
    assert(("Invalid block dimensions", block.width % 2 == 1 && block.height % 2 == 1 && block.width > 1 && block.height > 1));
    assert(("src and dst must be one channel grayscale images", src.channels() == 1, dst.channels() == 1));
    int height = src.rows;
    int width = src.cols;
    double noiseVar = -1;
    if (bits == 1)
        dst = Mat(height, width, CV_16UC1);
    else
        dst = Mat(height, width, CV_8UC1);

    Mat1d means, sqrMeans, variances;
    Mat1d avgVarianceMat; 

    boxFilter(src, means, CV_64F, block, Point(-1, -1), true, BORDER_REPLICATE);
    sqrBoxFilter(src, sqrMeans, CV_64F, block, Point(-1, -1), true, BORDER_REPLICATE);

    Mat1d means2 = means.mul(means);
    variances = sqrMeans - (means.mul(means));

    reduce(variances, avgVarianceMat, 1, REDUCE_SUM, CV_64F);
    reduce(avgVarianceMat, avgVarianceMat, 0, REDUCE_SUM, CV_64F);
    noiseVar = avgVarianceMat(0,0) / (height*width);

    if (bits == 1)
    {
        for (int row = 0; row < height; ++row)
        {
            ushort const * const srcRow = src.ptr<ushort>(row);
            ushort* const dstRow = dst.ptr<ushort>(row);
            double* const varRow = variances.ptr<double>(row);
            double* const meanRow = means.ptr<double>(row);
            for (int col = 0; col < width; ++col)
            {
                dstRow[col] = saturate_cast<ushort>(
                    meanRow[col] + max(0., varRow[col] - noiseVar) / max(varRow[col], noiseVar) * (srcRow[col] - meanRow[col])
                );
            }
        }
    }
    else
    {
        for (int row = 0; row < height; ++row)
        {
            uchar const * const srcRow = src.ptr<uchar>(row);
            uchar* const dstRow = dst.ptr<uchar>(row);
            double* const varRow = variances.ptr<double>(row);
            double* const meanRow = means.ptr<double>(row);
            for (int col = 0; col < width; ++col)
            {
                dstRow[col] = saturate_cast<uchar>(
                    meanRow[col] + max(0., varRow[col] - noiseVar) / max(varRow[col], noiseVar) * (srcRow[col] - meanRow[col])
                );
            }
        }
    }

    return noiseVar;
}