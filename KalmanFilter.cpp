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

#include "KalmanFilter.h"

double get_frame_score (Mat* f1, Mat* f2)
{
    unsigned int score = 0;
    int width, height;

    width = f1->rows;
    height = f1->cols;

    for ( int i = 0 ; i < width; i++)
    {
        for ( int j = 0 ; j < height; j++)
            score += abs(f1->data[i * width + j] - f2->data[i * width + j]);
    }
    return ((score) / (double)(width * height));
}

bool is_change(Mat* oldframe, Mat* frame, Scenechange* sc)
{
    bool change;
    double threshold;
    double score;
    double score_min;
    double score_max;

    score = get_frame_score (oldframe, frame);

    memmove (sc->diffs, sc->diffs + 1, sizeof (double) * (SC_N_DIFFS - 1));

    sc->diffs[SC_N_DIFFS - 1] = score;
    sc->n_diffs++;

    score_min = sc->diffs[0];
    score_max = sc->diffs[0];
    for (int i = 1; i < SC_N_DIFFS - 1; i++)
    {
        score_min = MIN (score_min, sc->diffs[i]);
        score_max = MAX (score_max, sc->diffs[i]);
    }
    threshold = 1.8 * score_max - 0.8 * score_min;

    if (threshold && score / threshold > 15.0)
    {
        change = true;
    }
    else if (score > (oldframe->rows * oldframe->cols))
    {
        change = true;
    }
    else
    {
        change = false;
    }

    if (change == true)
    {
        memset (sc->diffs, 0, sizeof (double) * SC_N_DIFFS);
        sc->n_diffs = 0;
    }

    return change;

}

KalmanFiltr::KalmanFiltr(Mat firstFrame, float q, float r, int maskSize, int d, double sigmaValue, int bits)
{
    height = firstFrame.rows;
    width = firstFrame.cols;

    xPredicted = Mat(height, width, CV_32F);
    xPredicted.setTo(1);

    pPredicted = Mat(height, width, CV_32F);
    pPredicted.setTo(1);

    firstFrame.convertTo(xCorrection, CV_32F);

    pCorrection = Mat(height, width, CV_32F);
    pCorrection.setTo(1);

    K = Mat(height, width, CV_32F);
    K.setTo(1);

    previous_blured = Mat(height, width, CV_32F);
    previous_blured.setTo(0);

    this->R = Mat(height, width, CV_32F);
    this->R.setTo(r);

    this->q = q;
    this->maskSize = maskSize;
    this->d = d;
    this->sigmas = sigmaValue;

    delta_Q = Mat(height, width, CV_32F);
    current_blured = Mat(height, width, CV_32F);
    bilateral_or_fastguided_filtered = Mat(height, width, CV_32F);
    if (bits == 1)
        kalman_filterd = Mat(height, width, CV_16U);
    else
        kalman_filterd = Mat(height, width, CV_8U);

    frameCount = 0;
    bilateralTime = 0.0;
    blurTime = 0.0;
    modifiedKalmanTime = 0.0;
}

Mat KalmanFiltr::KalmanFiltredFrame(const Mat &frame, int bits)
{
    frame.convertTo(floatFrame, CV_32F);
    blur(floatFrame, current_blured, Size(3, 3));
    bilateralFilter(floatFrame, bilateral_or_fastguided_filtered, d, sigmas, sigmas);

    delta_Q = current_blured - previous_blured;
    R = 1 + (R) / (1 + R);

    xPredicted = xCorrection.clone();
    xCorrection.copyTo(xPredicted);

    pPredicted = pCorrection + q * delta_Q.mul(delta_Q);

    K = pPredicted / (pPredicted + R);

    xCorrection = (1 - K).mul(xPredicted + K.mul(floatFrame - xPredicted)) + K.mul(bilateral_or_fastguided_filtered);

    pCorrection = pPredicted.mul(1 - K);

    previous_blured = current_blured.clone();
    current_blured.copyTo(previous_blured);

    if (bits == 1)
        xCorrection.convertTo(kalman_filterd, CV_16U);
    else
        xCorrection.convertTo(kalman_filterd, CV_8U);

    return kalman_filterd;
}