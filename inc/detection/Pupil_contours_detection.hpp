#ifndef __PUPIL_CONTOURS_DETECTION__
#define __PUPIL_CONTOURS_DETECTION__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "detection/cornealReflection.hpp"

using namespace cv;

namespace Pupil_contours_detection {

  static void extractEyeFeatures(cv::Mat frame, cv::Point &pupil_center, const int threshVal, std::vector<cv::Point> *glints = NULL){
    Mat frameCopy = frame.clone();

    pupil_center = Point(0,0);

    Mat ycbcr;
    cvtColor(frame, ycbcr, CV_BGR2YCrCb);


    Mat chan[3];
    split(ycbcr,chan);
    Mat with_corneal = chan[0];
		
    equalizeHist(with_corneal, with_corneal);
    threshold(with_corneal, with_corneal, threshVal, 255, THRESH_BINARY_INV);

    // corneal_remover cr;
    // Mat no_corneal = cr.remove_corneal_reflection(with_corneal.clone(), cornea);
    // // imshow("Output", with_corneal);
		
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Find contours
    findContours(with_corneal, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));
		
    int maxArea = 0;
    int maxIndex = 0;
    for(size_t idx_contour = 0; idx_contour<contours.size(); idx_contour++) {
      int ar = contourArea(contours[idx_contour]);
      // std::cout<<ar<<std::endl;
      if(ar>maxArea) {
        maxIndex = idx_contour;
        maxArea = ar;
      }
    }
    drawContours(frame, contours, maxIndex, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());

    // Fit ellipses
    RotatedRect fittedEllipse = fitEllipse(Mat(contours[maxIndex]));
    ellipse(frameCopy, fittedEllipse, Scalar(0, 255, 0), 2, 8);
    // cv::circle(frame, fittedEllipse.center, 4, CV_RGB(0,0,255), 2);
    // std::cout<<"Ellipse height: "<<fittedEllipse.size.height<<" width: "<<fittedEllipse.size.width<<std::endl;
    pupil_center = fittedEllipse.center;
		
    // // Show in a window
    // namedWindow("Contours", CV_WINDOW_AUTOSIZE);
    // imshow("Contours", frame);
    // waitKey(1);

    if( glints != NULL ){
      RotatedRect ellipse = fittedEllipse;
      // cv::Rect brect = ellipse.boundingRect();
      // cv::rectangle(frame, brect,  cv::Scalar(0,0,255),2);
      get_corneal_reflections( &ellipse, &frame, glints, 35, 2 );
      // corneal_reflection corneal;
      // corneal = corneal::locate_corneal_reflection(gray.clone(), pupil_center);
      // if( corneal.radius > 0 ) glints->push_back(corneal.center);
    }

  }
}

#endif
