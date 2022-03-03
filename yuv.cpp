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

#include "yuv.h"

using namespace cv;

enum YUV_ReturnValue
YUV_init(FILE* fin, FILE* fout, size_t w, size_t h, enum InputFormat ifmt, struct YUV_Capture* out, int bit_10)
{
    if (!fin || w % 2 == 1 || h % 2 == 1 || !fout)
        return YUV_PARAMETER_ERROR;

    dim = 1 << ifmt;
    bits = bit_10;
    out->fin = fin;
    out->fout = fout;
    out->width = w;
    out->height = h;
    out->iformat = ifmt;
    out->frames = 0;

    if (bits == 1)
    {
        out->cbcr = (char *)malloc(w * dim * h * sizeof(uint16_t) / 2);
        out->y = (char *)malloc(w * h * sizeof(uint16_t) );
        out->ydst = (char *)malloc(w * h * sizeof(uint16_t) );
        out->y_prev = (char *)malloc(w * h * sizeof(uint16_t) );
    }
    else
    {
        out->cbcr = (char *)malloc(w * dim * h * sizeof(uint8_t) / 2);
        out->y = (char *)malloc(w * h * sizeof(uint8_t) );
        out->ydst = (char *)malloc(w * h * sizeof(uint8_t) );
        out->y_prev = (char *)malloc(w * h * sizeof(uint8_t) );
    }

    if(out->y == NULL || out->cbcr == NULL || out->ydst == NULL || out->y_prev == NULL)
    {
        YUV_cleanup(out);
        printf("Out of memory\n");
        return YUV_OUT_OF_MEMORY;
    }

    return YUV_OK;
}

enum YUV_ReturnValue
YUV_read(struct YUV_Capture* cap, cv::Mat& src)
{
    size_t bytes_read;
    size_t npixels;

    npixels = cap->width * cap->height ;
    if(bits == 1)
        bytes_read = fread(cap->y, sizeof(uint16_t), npixels, cap->fin);
    else
        bytes_read = fread(cap->y, sizeof(uint8_t), npixels, cap->fin);

    if (bytes_read == 0)
        return YUV_EOF;
    else if (bytes_read != npixels)
        return YUV_IO_ERROR;

    if (bits == 1)
    {
        src = Mat(cap->height, cap->width, CV_16UC1, cap->y);
        bytes_read = fread(cap->cbcr, sizeof(uint16_t), npixels * dim / 2, cap->fin);
    }
    else
    {
        src = Mat(cap->height, cap->width, CV_8UC1, cap->y);
        bytes_read = fread(cap->cbcr, sizeof(uint8_t), npixels * dim / 2, cap->fin);
    }

    if (bytes_read != npixels * dim / 2)
        return YUV_IO_ERROR;

    return YUV_OK;
}

enum YUV_ReturnValue
YUV_read(struct YUV_Capture* cap, cv::Mat& src, cv::Mat& prev)
{
    size_t bytes_read;
    size_t npixels;
    npixels = cap->width * cap->height ;

    if (cap->frames)
    {
        if (bits == 1)
        {
            memcpy(cap->y_prev, cap->y, cap->width * cap->height * sizeof(uint16_t));
            prev = Mat(cap->height, cap->width, CV_16UC1, cap->y_prev);
        }
        else
        {
            memcpy(cap->y_prev, cap->y, cap->width * cap->height * sizeof(uint8_t));
            prev = Mat(cap->height, cap->width, CV_8UC1, cap->y_prev);
        }
    }

    if (bits == 1)
        bytes_read = fread(cap->y, sizeof(uint16_t), npixels, cap->fin);
    else
        bytes_read = fread(cap->y, sizeof(uint8_t), npixels, cap->fin);

    if (bytes_read == 0)
        return YUV_EOF;
    else if (bytes_read != npixels)
        return YUV_IO_ERROR;

    if (bits == 1)
    {
        src = Mat(cap->height, cap->width, CV_16UC1, cap->y);
        bytes_read = fread(cap->cbcr, sizeof(uint16_t), npixels * dim / 2, cap->fin);
    }
    else
    {
        src = Mat(cap->height, cap->width, CV_8UC1, cap->y);
        bytes_read = fread(cap->cbcr, sizeof(uint8_t), npixels * dim / 2, cap->fin);
    }

    if (bytes_read != npixels * dim /  2)
        return YUV_IO_ERROR;
    cap->frames++;
    return YUV_OK;
}

enum YUV_ReturnValue
YUV_write(struct YUV_Capture* cap, const cv::Mat& dst)
{
    size_t bytes_write;
    size_t npixels;

    npixels = cap->width * cap->height ;
    if (bits == 1)
        bytes_write = fwrite(dst.data, sizeof(uint16_t), npixels, cap->fout);
    else
        bytes_write = fwrite(dst.data, sizeof(uint8_t), npixels, cap->fout);

    if (bytes_write != npixels)
        return YUV_IO_ERROR;
    if (bits == 1)
        bytes_write = fwrite(cap->cbcr, sizeof(uint16_t), npixels * dim / 2, cap->fout);
    else
        bytes_write = fwrite(cap->cbcr, sizeof(uint8_t), npixels * dim / 2, cap->fout);

    if (bytes_write != npixels * dim / 2)
        return YUV_IO_ERROR;

    return YUV_OK;
}

void
YUV_cleanup(struct YUV_Capture* cap)
{
    if (!cap)
        return;

    if (cap->y)
        free(cap->y);
    if (cap->ydst)
        free(cap->ydst);
    if (cap->cbcr)
        free(cap->cbcr);
    if (cap->y_prev)
        free(cap->y_prev);
}