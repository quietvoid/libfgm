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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

#include "WienerFilter.h"
#include "KalmanFilter.h"
#include "yuv.h"
#include "common.h"
#include "FrequencyFilteringModel.h"
#include "AutoRegressionModel.h"

void printInfo()
{
    printf ("--help print help info\n"
            "--input input.yuv\n"
            "--output output.yuv\n"
            "--frames frames count\n"
            "--skip skip count\n"
            "--wiener-filter <0/1/2> 0 = 3x3; 1 = 5x5; 2 = 7x7\n"
            "--wiener-weight <1..9>\n"
            "--kalman-weight <0..9>\n"
            "--width W\n"
            "--height H\n"
            "--pix-fmt yuv pixel format (yuv420p)\n"
            "--model modelFilename.bin\n"
            "--frequency-filter <0/1> 0 = AutoRegressionModel 1 = FrequencyFilteringModel");
}

int main(int argc, char **argv)
{
    int width = 0;
    int height = 0;
    char inputName[256] = {0};
    char outName[256] = {0};
    char modelName[256] = {0};
    enum InputFormat ifmt = YUV420P;
    FILE *fin = NULL;
    FILE *fout = NULL;
    FILE *fmodel = NULL;
    struct YUV_Capture cap;
    enum YUV_ReturnValue ret;
    Mat src, dst, combined_dst, canny;
    Mat difference_image;
    Mat prev_grey;
    Mat dst_kalman;
    int cop = 0;
    bool scenecut = true; // first frame is taken as scenecut
    int frame_count = 0;
    Scenechange sc;
    float q = 0.005f;
    float r = 1;
    int maskSize = 5;
    int bilateralKernelSize = 5;
    float bilateralSigmas = 50;
    int dst_weight = 1;
    int dst_kalman_weight = 1;
    int dst_frequency_filtering = 1;
    char* filter;
    int total_frames = 0;
    int skip_frames = 0;
    int blockSize = 0;
    Detection edge_detect;
    struct SEIFilmGrainCharacteristics fg_char;
    struct intensityValues int_value;
    int bits = 0;

    if (argc < 11)
    {
        printInfo();
        return 1;
    }

    for (int i = 1; i < argc; i += 2)
    {
        if (!strcmp(argv[i], "--input")) {
            strcpy(inputName, argv[i+1]);
        } else if (!strcmp(argv[i], "--output")) {
            strcpy(outName, argv[i+1]);
        }  else if (!strcmp(argv[i], "--model")) {
            strcpy(modelName, argv[i+1]);
        } else if (!strcmp(argv[i], "--frames")) {
            total_frames = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--skip")) {
            skip_frames = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--width")) {
            width = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--height")) {
            height = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--wiener-filter")) {
            blockSize = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--wiener-weight")) {
            dst_weight = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--kalman-weight")) {
            dst_kalman_weight = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--frequency-filter")) {
            dst_frequency_filtering = atoi(argv[i+1]);
        } else if (!strcmp(argv[i], "--pix-fmt")){
            if (!strcmp(argv[i+1],"yuv422p")){
                ifmt = YUV422P;
            } else if (!strcmp(argv[i+1],"yuv444p")) {
                ifmt = YUV444P;
            } else if (!strcmp(argv[i+1],"yuv420p")){
                ifmt = YUV420P;
            } else if (!strcmp(argv[i+1],"yuv444p10")) {
                bits = 1;
                ifmt = YUV444P;
            } else if (!strcmp(argv[i+1],"yuv420p10")){
                bits = 1;
                ifmt = YUV420P;
            } else if (!strcmp(argv[i+1],"yuv422p10")) {
                bits = 1;
                ifmt = YUV422P;
            }
        } else if (!strcmp(argv[i], "--help")) {
            printInfo();
            return 0;
        } else {
            printf("Invalid Option %s. Use --help to know more\n", argv[i]);
            return -1;
        }
    }

    if (width <= 0 || height <= 0)
    {
        fprintf(stderr, "error: bad frame dimensions: %d x %d\n", width, height);
        return 1;
    }
    if (dst_weight < 1 || dst_weight > 9)
    {
        fprintf(stdout, "Warning: bad wiener weight: %d; Setting to def 1\n", dst_weight);
        dst_weight = 1;
    }
    if (dst_kalman_weight < 0 || dst_kalman_weight > 9)
    {
        fprintf(stdout, "Warning: bad kalman weight: %d; Setting to def 1\n", dst_kalman_weight);
        dst_kalman_weight = 1;
    }

    fin = fopen(inputName, "r");
    if (!fin)
    {
        fprintf(stderr, "error: unable to open file for read: %s\n", inputName);
        return 1;
    }
    fout = fopen(outName, "wb+");
    if (!fout)
    {
        fprintf(stderr, "error: unable to open file for write: %s\n", outName);
        return 1;
    }
    fmodel = fopen(modelName, "wb+");
    if (!fmodel)
    {
        fprintf(stderr, "error: unable to open model file for write: %s\n", modelName);
        return 1;
    }

    ret = YUV_init(fin, fout, width, height, ifmt, &cap, bits);
    assert(ret == YUV_OK);
    sc.n_diffs = 0;
    memset (sc.diffs, 0, sizeof (double) * SC_N_DIFFS);
    if (bits == 1)
    {
        filter = (char* ) malloc (width * height * sizeof(uint16_t));
        canny = Mat(height, width, CV_16U);
        difference_image = Mat(height, width, CV_16S); // Need to be signed to match with film-grains
    }
    else
    {
        filter = (char* ) malloc (width * height);
        canny = Mat(height, width, CV_8U);
        difference_image = Mat(height, width, CV_8S); // Need to be signed to match with film-grains
    }
    int num64x64blocks = (width * height) / (64 * 64);
    int* index_nonedge_blocks = (int*) malloc (sizeof(int) * num64x64blocks);
    int total_nonedge_blocks = 0;

    while (cop <= skip_frames)
    {
        ret = YUV_read(&cap, src, prev_grey);
        if (ret == YUV_EOF)
            printf("End of file reached\n");
        else if (ret == YUV_IO_ERROR)
            fprintf(stderr, "I/O error\n");
        cop++;
    }
    KalmanFiltr kf = KalmanFiltr(src, q, r, maskSize, bilateralKernelSize, bilateralSigmas,bits);

    for (; ;frame_count++)
    {
        double estimatedNoiseVariance;
        if (blockSize == 0)
            estimatedNoiseVariance = WienerFilter(src, dst, bits,Size(3, 3));
        else if (blockSize == 1)
            estimatedNoiseVariance = WienerFilter(src, dst, bits);
        else
            estimatedNoiseVariance = WienerFilter(src, dst, bits, Size(7, 7));

        Mat temp_src = src.clone();
        src.copyTo(temp_src);
        dst_kalman = kf.KalmanFiltredFrame(temp_src, bits);
        if (!frame_count || is_change(&prev_grey, &src, &sc))
        {
            printf("Scenecut detected\n"); fflush(stdout);
            dst_kalman = src.clone();
            src.copyTo(dst_kalman);
            scenecut = true;
        }

        if (scenecut || !frame_count)
            ret = YUV_write(&cap, dst);
        else
        {
            printf("Kalman + Wiener");
            int avg_ret = combined_avg(dst, dst_kalman, combined_dst, filter, dst_weight, dst_kalman_weight, bits);
            ret = YUV_write(&cap, combined_dst);
        }

        if (ret == YUV_IO_ERROR)
        {
            fprintf(stderr, "I/O error\n");
            break;
        }

        /* Start frequency filtering model at first frame or at scencuts */
        if (!frame_count || scenecut)
        {
            int scale_factor;
            edge_detect.EdgeDetection(dst, &canny, bits);
            /* Return list of 64x64 block index with the 95% non-edges region */
            edge_detect.NonEdge64x64Blocks(canny, index_nonedge_blocks, &total_nonedge_blocks);
            getResidual(dst, src, &difference_image, index_nonedge_blocks, &int_value, bits);
            if(dst_frequency_filtering)
            {
                if ((height <= 1080 && height >= 720) || (width <= 1920 && width >= 1280))
                    scale_factor = 5;
                else if (height < 720 || width < 1280)
                    scale_factor = 6;
                else
                    scale_factor = 4;
                computeFGParams(&int_value, &fg_char, scale_factor);
            }
            else
                calculateModelValues(dst, index_nonedge_blocks, &int_value, &fg_char);
        }
        writeFilmGrainFrequencyModel(fg_char, fmodel);
        if (total_frames && ((frame_count + 1) >= total_frames))
            break;

        ret = YUV_read(&cap, src, prev_grey);
        printf("\rFrame read = %d", cop++); fflush(stdout);
        if (ret == YUV_EOF)
        {
            printf("End of file reached\n");
            break;
        }
        else if (ret == YUV_IO_ERROR)
        {
            fprintf(stderr, "I/O error\n");
            break;
        }
        /* Reset scenecut to false */
        scenecut = false;
    }
    printf("\n\nFILTER COMPLETE\n\n");

    free(index_nonedge_blocks);
    free(filter);
    fclose(fin);
    fclose(fout);
    fclose(fmodel);
    return 0;
}