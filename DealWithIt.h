#ifndef DEALWITHIT_H
#define DEALWITHIT_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
//#include <cstdio>

using namespace std;
using namespace cv;

#if defined(__linux__)
#define cascade_prefix "/usr/share/opencv/haarcascades/"
#elif defined(__APPLE__)
#define cascade_prefix "/usr/local/Cellar/opencv/3.0.0/share/OpenCV/haarcascades/"
#else
#define cascade_prefix "C:\\opencv\\share\\haarcascades\\"
#endif

#define DEBUG false

#define log if(DEBUG) cout

class DealWithIt
{
private:
    Mat dealWithIt;
    Mat mainImg;
    Mat rotatedImg;
    int xoffset;
    int yoffset;

    CascadeClassifier face_cascade;
    CascadeClassifier eyes_cascade;

public:
    DealWithIt(String mainImgPath, int xoffset = 0, int yoffset = 0);

    void animate(Point& from, Point& to);
    void process();

    static void blendImage(Mat* src, Mat* clip, const Point &where);
    static void rotateImage(Mat &inImage, Mat &outImage, double angle);
};

#endif // DEALWITHIT_H
