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

#include "FrequencyFilteringModel.h"
#include <cassert>

void computeFGParams(struct intensityValues* int_values, struct SEIFilmGrainCharacteristics* fgm_char, int scale_factor)
{
    /* Adding relevant SEI film grain params */
    float factor = 0.7;
    memset(fgm_char, 0, sizeof(struct SEIFilmGrainCharacteristics));
    fgm_char->m_filmGrainModelId = 0;
    fgm_char->m_log2ScaleFactor = scale_factor;
    fgm_char->m_compModel[0].bPresentFlag = true;
    fgm_char->m_compModel[1].bPresentFlag = false;
    fgm_char->m_compModel[2].bPresentFlag = false;
    if (scale_factor == 6)
        factor = 1.1;
    /* Set the luma film grain param values */
    fgm_char->m_compModel[0].numModelValues = 1;
    fgm_char->m_compModel[0].m_filmGrainNumIntensityIntervalMinus1 = 0;
    CompModelIntensityValues* intensityVal = &fgm_char->m_compModel[0].intensityValues[0];

    intensityVal->intensityIntervalLowerBound = CLIP3(0, 255, int_values->min_intensity * 0.1);
    intensityVal->intensityIntervalUpperBound = CLIP3(0, 255, int_values->max_intensity * 1.1);
    intensityVal->compModelValue[0] = CLIP3(0, 255, int_values->model_intensity * factor);

}
