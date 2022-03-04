/*************************************************************************
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
 ************************************************************************/

#include "AutoRegressionModel.h"
#include <cassert>

void calculateModelValues(Mat src1, int* index, struct intensityValues* int_values, struct SEIFilmGrainCharacteristics* fgm_char)
{
    memset(fgm_char, 0, sizeof(struct SEIFilmGrainCharacteristics));

    fgm_char->m_filmGrainModelId = 1;
    fgm_char->m_log2ScaleFactor = 4;
    fgm_char->m_compModel[0].bPresentFlag = true;
    fgm_char->m_compModel[1].bPresentFlag = false;
    fgm_char->m_compModel[2].bPresentFlag = false;

    /* Set the luma film grain param values */
    fgm_char->m_compModel[0].numModelValues = 3;
    fgm_char->m_compModel[0].m_filmGrainNumIntensityIntervalMinus1 = 0;
    CompModelIntensityValues* intensityVal = &fgm_char->m_compModel[0].intensityValues[0];

    intensityVal->intensityIntervalLowerBound = CLIP3(0, 255, int_values->min_intensity * 0.5);
    intensityVal->intensityIntervalUpperBound = CLIP3(0, 255, int_values->max_intensity * 1.1);
    intensityVal->compModelValue[0] = int_values->standardDeviation * 0.7;

    int width, height, k = 0, ind = 0;
    height = src1.rows;
    width = src1.cols;

    uint32_t numPartInWidth = (width + 64 - 1) / 64;
    uint32_t numPartInHeight = (height + 64 - 1) / 64;
    uint32_t blockXY = numPartInWidth+1;
    uint32_t mean_diff_above = 0, mean_diff_left = 0,mean_diff_above_left = 0, mean_diff_below_left = 0,count = 0;
    for (int y = 1; y < numPartInHeight; y++)
    {
        for (int x = 1; x < numPartInWidth; x++,blockXY++)
        {
            if ((blockXY%numPartInWidth) == 0)
                continue;
            if (!int_values->avg[blockXY] || !int_values->avg[blockXY-1] || !int_values->avg[blockXY-numPartInWidth])
                continue;
            mean_diff_above = int_values->avg[blockXY] - int_values->avg[blockXY-numPartInWidth];
            mean_diff_left = int_values->avg[blockXY] - int_values->avg[blockXY-1];
            int_values->mean_diff += ((mean_diff_above+mean_diff_left) / 2);
            count++;
        }
    }
    intensityVal->compModelValue[1] = CLIP3(0, 255, (int_values->mean_diff / count)) * 0.7;
    intensityVal->compModelValue[2] = 0;
    int_values->mean_diff = 0;
    count = 0;
    for (int y = 1; y < numPartInHeight-1; y++)
    {
        for (int x = 1; x < numPartInWidth; x++,blockXY++)
        {
            if ((blockXY%numPartInWidth) == 0)
                continue;
            if (!int_values->avg[blockXY] || !int_values->avg[blockXY-1+numPartInWidth] || !int_values->avg[blockXY-numPartInWidth-1])
                continue;
            mean_diff_above_left = int_values->avg[blockXY] - int_values->avg[blockXY-numPartInWidth-1];
            mean_diff_below_left = int_values->avg[blockXY] - int_values->avg[blockXY+numPartInWidth-1];
            int_values->mean_diff += CLIP3(0, 255, ((mean_diff_above_left+mean_diff_below_left) / 2));
            count++;
        }
    }
    intensityVal->compModelValue[3] = CLIP3(0, 255, (int_values->mean_diff / count)) * 0.7;
    intensityVal->compModelValue[4] = 0;
    intensityVal->compModelValue[5] = 0;
}
