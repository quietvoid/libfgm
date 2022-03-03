/*************************************************************************
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

#ifndef YUV_H
#define YUV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

static int dim,bits ;

/* Is returned by the public API functions */
enum YUV_ReturnValue
{
    YUV_OK = 0,             /**< Function terminated correctly >*/
    YUV_PARAMETER_ERROR,    /**< The parameters passed to the function were
                                 incorrect */
    YUV_OUT_OF_MEMORY,      /**< The function ran out of memory >*/
    YUV_IO_ERROR,           /**< The function encountered an error reading
                                 data from disk, or reached EOF earlier than 
                                 was expected (premature end of frame) >*/
    YUV_EOF                 /**< The function reached EOF (not premature) >*/
};

enum InputFormat
{
    YUV420P = 0,
    YUV422P,
    YUV444P,
    YUV420P10,
    YUV422P10,
    YUV444P10,
    MAX_FORMAT
};

/**
 * Used to capture YUV frames from a file on disk.
 *
 * @see YUV_init
 * @see YUV_read
 */

struct YUV_Capture
{
    FILE* fin;              /**< The input file pointer >*/
    FILE* fout;             /**< The output file pointer >*/
    size_t width;           /**< The width of the frame, in pixels >*/
    size_t height;          /**< The height of the frame, in pixels >*/
    enum InputFormat iformat; /** < Input Format - YUV420P, YUV422P, YUV444P > */

    char* y;            /**< Used internally. >*/
    char* ydst;         /**< Filtered Y plane >*/
    char* cbcr;           /**< Used internally. >*/
    char* y_prev;        /**<Required for Kalman filter> **/
    unsigned int frames;
};

/**
 * Initialize a YUV_Capture instance.  Allocates memory used for internal
 * processing.  After initialization, frames can be read using YUV_read.
 */
enum YUV_ReturnValue YUV_init(FILE* fin, FILE* fout, size_t w, size_t h, enum InputFormat ifmt, struct YUV_Capture* out, int bit_10);

/**
 * Read a single frame from a previously-instantiated YUV_Capture instance.
 */
enum YUV_ReturnValue YUV_read(struct YUV_Capture* cap, cv::Mat &src);

enum YUV_ReturnValue YUV_read(struct YUV_Capture* cap, cv::Mat& src, cv::Mat& prev);

/**
 * Write a single frame from a previously-instantiated YUV_Capture instance.
 */
enum YUV_ReturnValue YUV_write(struct YUV_Capture* cap, const cv::Mat &dst);

/**
 * Deallocate the memory allocated during initialization.
 */
void YUV_cleanup(struct YUV_Capture* cap);

#endif