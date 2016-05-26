#include "DealWithIt.h"

DealWithIt::DealWithIt(String mainImgPath, int xoffset, int yoffset)
{
    dealWithIt = imread("dealwithit.png", IMREAD_UNCHANGED);
    mainImg = imread(mainImgPath, IMREAD_UNCHANGED);

    this->xoffset = xoffset;
    this->yoffset = yoffset;

    if(dealWithIt.empty())
    {
        throw runtime_error("Failed loading awesome 8-bit glass image (dealwithit.png)");
    }

    if(mainImg.empty())
    {
        throw runtime_error("Failed loading main image (" + mainImgPath + ")");
    }

    String face_cascade_name;
    face_cascade_name.append(cascade_prefix);
    face_cascade_name.append("haarcascade_frontalface_alt.xml");

    String eyes_cascade_name;
    eyes_cascade_name.append(cascade_prefix);
    eyes_cascade_name.append("haarcascade_eye_tree_eyeglasses.xml");

    bool face_ok = face_cascade.load(face_cascade_name);
    bool eyes_ok = eyes_cascade.load(eyes_cascade_name);

    if(face_ok && eyes_ok)
    {
        log << "Loaded cascade successfully\n";
    }
    else
    {
        throw runtime_error("Failed loading cascade files");
    }
}

void DealWithIt::blendImage(Mat *src, Mat *clip, const Point &where)
{
    for(int y = max(where.y, 0); y < src->rows; ++y)
    {
        int fy = y - where.y;

        if (fy >= clip->rows)
            break;

        for (int x = max(where.x, 0); x < src->cols; ++x)
        {
            int fx = x - where.x;

            if (fx >= clip->cols)
                break;

            double opacity = ((double)clip->data[fy * clip->step + fx * clip->channels() + 3]) / 255;

            for (int c = 0; opacity > 0 && c < src->channels(); ++c)
            {
                unsigned char overlayPx = clip->data[fy * clip->step + fx * clip->channels() + c];
                unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
                src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
            }
        }
    }
}

void DealWithIt::animate(Point &from, Point &to)
{
    log << "Sending glass from " << from << " to " << to << "\n";

    double m = atan2(to.y-from.y, to.x-from.x);
    m *= 180.0 / M_PI;

    for(int y=from.y, i=0; y<=to.y; y++, i++)
    {
        int x = to.x + (y/m);

        Mat draw = mainImg.clone();

        blendImage(&draw, &rotatedImg, Point(x, y));

        imshow("DealWithIt", draw);

        /*
         * GIF GENERATOR
         */
        //char filename[20] = {};
        //sprintf(filename, "frame_%03d.jpg", i);
        //imwrite(filename, draw);

        waitKey(1);
    }

    log << "Press any key to quit\n";

    waitKey(0xFFFFFF);
}

void DealWithIt::rotateImage(Mat &inImage, Mat &outImage, double angle)
{
    Point center = Point(inImage.cols/2, inImage.rows/2);
    Mat rotationMat = getRotationMatrix2D(center, angle, 1);
    Rect bbox = RotatedRect(center, inImage.size(), angle).boundingRect();
    rotationMat.at<double>(0,2) += bbox.width/2.0 - center.x;
    rotationMat.at<double>(1,2) += bbox.height/2.0 - center.y;
    warpAffine(inImage, outImage, rotationMat, bbox.size() );
}

void DealWithIt::process()
{
    Mat frame_gray;
    vector<Rect> faces;

    cvtColor(mainImg, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));

    if(faces.size() == 0)
    {
        cerr << "No face detected. Can't deal with it\n";
        return;
    }

    log << "Detected " << faces.size() << " faces\n";

    for(unsigned int i=0; i<faces.size(); i++)
    {
        Rect& face = faces[i];
        Mat faceROI = frame_gray(face);

        vector<Rect> eyes;
        eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));

        if(eyes.size() != 2)
        {
            cerr << "I need exactly 2 eyes to deal with it\n";
            continue;
        }
        else
        {
            log << "Detected " << eyes.size() << " eyes\n";
        }

        Rect eyel = eyes[0];
        Rect eyer = eyes[1];

        if(eyel.x > eyer.x)
            swap(eyel, eyer);

        log << "Left eye at " << eyel << "\n";
        log << "Right eye at " << eyer << "\n";

        eyel.x += face.x;
        eyel.y += face.y;

        eyer.x += face.x;
        eyer.y += face.y;

        Rect glass = (eyel | eyer);
        log << "Glass to be placed at " << glass << "\n";

        Point centerl(eyel.x + eyel.width/2, eyel.y + eyel.height/2);
        Point centerr(eyer.x + eyer.width/2, eyer.y + eyer.height/2);

        double angle = -atan2(centerr.y - centerl.y, centerr.x - centerl.x);
        angle *= 180.0 / M_PI;

        log << "Inclination is " << angle << " degree\n";

        double ratio = (double)dealWithIt.cols / dealWithIt.rows;
        int width = face.width; // * 1.20; //333333;
        int height = width / ratio;
        resize(dealWithIt, dealWithIt, Size(width, height));

        rotateImage(dealWithIt, rotatedImg, angle);

        Point from((mainImg.cols/2)-(dealWithIt.cols/2),
                   -dealWithIt.rows);
        Point to((centerl.x+centerr.x)/2 - dealWithIt.cols/2,
                 (centerl.y+centerr.y)/2 - dealWithIt.rows/2);

        to.x += xoffset;
        to.y += yoffset;

        if(DEBUG)
        {
            Mat debugDraw = mainImg.clone();

            // Mark glass area (left + right eye union)
            rectangle(debugDraw, glass, Scalar(0,255,255), 2);
            // Mark face
            rectangle(debugDraw, face, Scalar(255,0,255), 2);
            // Mark left eye
            rectangle(debugDraw, eyel, Scalar(0,255,0), 2);
            // Mark right eye
            rectangle(debugDraw, eyer, Scalar(0,255,0), 2);
            // Draw eye to eye center
            line(debugDraw, centerl, centerr, Scalar(0,0,255), 2);
            // Show the result
            imshow("Debug", debugDraw);
        }

        animate(from, to);

    }
}
