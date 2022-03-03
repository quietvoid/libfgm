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

#include "common.h"
#include "CannyEdgeDetector.h"

void Detection::EdgeDetection(Mat data, Mat* canny, int bits)
{
    cv::Scalar tempVal = cv::mean(data);
    float mean = tempVal.val[0];
    canny_min_threshold = 0.66 * mean;
    canny_max_threshold = 1.33 * mean;
    Mat canny_output, canny_output1, closing_output;
    CannyEdgeDetector ced(canny_min_threshold, canny_max_threshold, bits);
    ced.Apply(data, canny_output);
    if (bits == 1)
        memcpy(canny->data, canny_output.data, sizeof(ushort) * canny->cols * canny->rows);
    else
        memcpy(canny->data, canny_output.data, sizeof(uchar) * canny->cols * canny->rows);
}

void Detection::NonEdge64x64Blocks(Mat canny, int* index, int* total)
{
    int height = canny.rows;
    int width = canny.cols;
    uint8_t *data = canny.data;
    int threshold = (64 * 64) * 0.95; // 95% of 64*64 block
    int ind = 0;
    memset (index, -1, sizeof(index));

    uint32_t numPartInWidth = (width + 64 - 1) / 64;
    uint32_t numPartInHeight = (height + 64 - 1) / 64;
    uint32_t blockXY = 0;
    for (int y = 0; y < numPartInHeight; y++)
    {
        for (int x = 0; x < numPartInWidth; x++)
        {
            uint32_t block_x = x * 64;
            uint32_t block_y = y * 64;
            int nonedge = 0;
            for (uint32_t block_yy = block_y; block_yy < block_y + 64 && block_yy < height; block_yy++)
            {
                for (uint32_t block_xx = block_x; block_xx < block_x + 64 && block_xx < width; block_xx++)
                {
                    if (data[block_yy * width + block_xx] == 0x00)
                        nonedge++;
                }
            }
            if (nonedge >= threshold)
                index[ind++] = blockXY;
            blockXY++;
        }
    }
    *total = ind;
}

int combined_avg(Mat& dst1, Mat& dst2, Mat& final_dst, char* filter, int dst_weight, int dst_kalman_weight, int bits )
{
    int width, height, k = 0;
    if (dst_weight > 9)
    {
        printf("Warning: Too big value for wiener-filter weight; Setting to Max - 9");
        dst_weight = 9;
    }
    if (dst_kalman_weight > 9)
    {
        printf("Warning: Too big value for kalman-filter weight; Setting to Max - 9");
        dst_kalman_weight = 9;
    }
    height = dst1.rows;
    width = dst1.cols;
    if (bits == 1)
    {
        final_dst = Mat(height, width, CV_16UC1);
        for (int row = 0; row < height; ++row)
        {
            ushort* const src1Row = dst1.ptr<ushort>(row);
            ushort* const src2Row = dst2.ptr<ushort>(row);
            ushort* const dstRow = final_dst.ptr<ushort>(row);
            for (int col = 0; col < width; ++col)
            {
                dstRow[col] = ((dst_weight * src1Row[col]) + (dst_kalman_weight * src2Row[col])) / (dst_weight + dst_kalman_weight);
            }
        }
    }
    else
    {
        final_dst = Mat(height, width, CV_8UC1);
        for (int row = 0; row < height; ++row)
        {
            uchar* const src1Row = dst1.ptr<uchar>(row);
            uchar* const src2Row = dst2.ptr<uchar>(row);
            uchar* const dstRow = final_dst.ptr<uchar>(row);
            for (int col = 0; col < width; ++col)
            {
                dstRow[col] = ((dst_weight * src1Row[col]) + (dst_kalman_weight * src2Row[col])) / (dst_weight + dst_kalman_weight);
            }
        }
    }
    return 0;
}

void getResidual(Mat src1, Mat src2, Mat* difference_image, int* index, struct intensityValues* int_values, int bits )
{
    int width, height, k = 0, ind = 0;
    height = src1.rows;
    width = src1.cols;
    if(bits == 1)
        memset(difference_image->data, 0, width * height * (sizeof(short)));
    else
        memset(difference_image->data, 0, width * height );
    uint32_t numPartInWidth = (width + 64 - 1) / 64;
    uint32_t numPartInHeight = (height + 64 - 1) / 64;
    uint32_t blockXY = 0, avg = 0, N = 64 * 64;
    int_values->min_intensity = 255;
    int_values->max_intensity = 0;
    memset(int_values->intensity_count, 0, sizeof(int_values->intensity_count));
    int_values->avg = (uint32_t *) malloc(sizeof(unsigned int) * numPartInWidth * numPartInHeight );
    memset(int_values->avg, 0, sizeof(unsigned int) * numPartInWidth * numPartInHeight);
    uint32_t max_count = 0;
    int_values->standardDeviation = 0;
    for (int y = 0; y < numPartInHeight; y++)
    {
        for (int x = 0; x < numPartInWidth; x++, blockXY++)
        {
            if (index[ind] != blockXY)
                continue;
            uint32_t block_x = x * 64;
            uint32_t block_y = y * 64;
            avg = 0;
            for (uint32_t block_yy = block_y; block_yy < block_y + 64 && block_yy < height; block_yy++)
            {
                for (uint32_t block_xx = block_x; block_xx < block_x + 64 && block_xx < width; block_xx++)
                {
                    uint32_t id = block_yy * width + block_xx;
                    difference_image->data[id] = src1.data[id] - src2.data[id];
                    avg += (difference_image->data[id] + 127); // Adding offset
                }
            }

            int_values->avg[blockXY] = CLIP3(0, 255, (avg >> 12));
            if (int_values->avg[blockXY] < int_values->min_intensity)
                int_values->min_intensity = int_values->avg[blockXY];
            else if (int_values->avg[blockXY] > int_values->max_intensity)
                int_values->max_intensity = int_values->avg[blockXY];
            int_values->intensity_count[int_values->avg[blockXY]]++;
            if (max_count < int_values->intensity_count[int_values->avg[blockXY]])
            {
                max_count = int_values->intensity_count[int_values->avg[blockXY]];
                int_values->model_intensity = int_values->avg[blockXY];
            }
            uint32_t standDeviation = 0;
            for (uint32_t block_yy = block_y; block_yy < block_y + 64 && block_yy < height; block_yy++)
            {
                for (uint32_t block_xx = block_x; block_xx < block_x + 64 && block_xx < width; block_xx++)
                {
                    uint32_t id = block_yy * width + block_xx;
                    standDeviation += pow(( int_values->avg[blockXY] - difference_image->data[id] ),2);
                }
            }
            int_values->standardDeviation += pow((standDeviation/N) , 0.5);
            ind++;
        }
    }
    int_values->standardDeviation = int_values->standardDeviation / ind ;
}

void writeFilmGrainFrequencyModel(struct SEIFilmGrainCharacteristics filmgrain, FILE* fout)
{
    /* Write to the model file */
    fwrite((char* )&filmgrain.m_filmGrainCharacteristicsCancelFlag, sizeof(bool), 1, fout);
    fwrite((char* )&filmgrain.m_filmGrainCharacteristicsPersistenceFlag, sizeof(bool), 1, fout);
    fwrite((char* )&filmgrain.m_filmGrainModelId, sizeof(unsigned char), 1, fout);
    fwrite((char* )&filmgrain.m_separateColourDescriptionPresentFlag, sizeof(bool), 1, fout); // Always set to 0
    fwrite((char* )&filmgrain.m_blendingModeId, sizeof(unsigned char), 1, fout);
    fwrite((char* )&filmgrain.m_log2ScaleFactor, sizeof(unsigned char), 1, fout);
    fwrite((char* )&filmgrain.m_compModel[0].bPresentFlag, sizeof(bool), 1, fout);
    fwrite((char* )&filmgrain.m_compModel[1].bPresentFlag, sizeof(bool), 1, fout);
    fwrite((char* )&filmgrain.m_compModel[2].bPresentFlag, sizeof(bool), 1, fout);
    for (int i = 0; i < 3; i++)
    {
        if (filmgrain.m_compModel[i].bPresentFlag)
        {
            fwrite((char* )&filmgrain.m_compModel[i].m_filmGrainNumIntensityIntervalMinus1, sizeof(unsigned char), 1, fout);
            fwrite((char* )&filmgrain.m_compModel[i].numModelValues, sizeof(unsigned char), 1, fout);
            for (int j = 0; j <= filmgrain.m_compModel[i].m_filmGrainNumIntensityIntervalMinus1; j++)
            {
                fwrite((char* )&filmgrain.m_compModel[i].intensityValues[j].intensityIntervalLowerBound, sizeof(unsigned char), 1, fout);// min intensity
                fwrite((char* )&filmgrain.m_compModel[i].intensityValues[j].intensityIntervalUpperBound, sizeof(unsigned char), 1, fout);// max intensity
                for (int k = 0; k < filmgrain.m_compModel[i].numModelValues; k++)
                {
                    fwrite((char* )&filmgrain.m_compModel[i].intensityValues[j].compModelValue[k], sizeof(int), 1, fout);// compModelValue
                }
            }
        }
    }
}