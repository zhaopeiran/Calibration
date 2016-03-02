#ifndef __ALGO_HPP__
#define __ALGO_HPP__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "detection/peek.hpp"
#include "detection/find_best_edge.hpp"
#include "detection/th_angular_histo.hpp"

#include "detection/optimize_pos.hpp"
#include "detection/zero_around_region_th_border.hpp"


#include "detection/canny_ml.hpp"
#include "detection/cornealReflection.hpp"


static cv::RotatedRect detectPupilEllipse(cv::Mat *pic, cv::Mat *pic_th, cv::Mat *th_edges){

	//mean under mean
	//mean_under_mean(pic, 5);
	cv::normalize(*pic, *pic, 0, 255, cv::NORM_MINMAX, CV_8U);



	double border=0.1;
	int peek_detector_factor=10;
	int bright_region_th=199;
	double mean_dist=3;
	int inner_color_range=5; 
	double th_histo=0.5;
	int max_region_hole=5;
	int min_region_size=7;
	double area_opt=0.1;
  double area_edges=0.2;
	int edge_to_th=5;

	cv::RotatedRect ellipse;
	cv::Point pos(0,0);

	int start_x=floor(double(pic->cols)*border);
	int start_y=floor(double(pic->rows)*border);

	int end_x =pic->cols-start_x;
	int end_y =pic->rows-start_y;



	double stddev=0;
	bool edges_only_tried=false;
	bool peek_found=false;
	int threshold_up=0;


	//cv::Mat pic=rgb2gray(m); //m is input pointer to mat
	//cv::normalize(*m, *m, 0, 255, cv::NORM_MINMAX, CV_8U);
	//cv::Mat pic;
	//cvtColor(*m, pic, CV_RGB2GRAY);



	peek_found=peek(pic, &stddev, start_x, end_x, start_y, end_y, peek_detector_factor, bright_region_th);
	threshold_up=ceil(stddev/2);
	threshold_up--;



	cv::Mat picpic = cv::Mat::zeros(end_y-start_y, end_x-start_x, CV_8U);


	

	for(int i=0; i<picpic.cols; i++)
		for(int j=0; j<picpic.rows; j++){
			picpic.data[(picpic.cols*j)+i]=pic->data[(pic->cols*(start_y+j))+(start_x+i)];
		}
	
		/*
	cv::Mat detected_edges2;
	cv::GaussianBlur(picpic,detected_edges2, cv::Size(15,15),sqrt(2.0));
	Canny( detected_edges2, detected_edges2, stddev*0.4, stddev, 3 );
	*/
	cv::Mat detected_edges2 = canny_impl(&picpic);
	


	cv::Mat detected_edges = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
	for(int i=0; i<detected_edges2.cols; i++)
		for(int j=0; j<detected_edges2.rows; j++){
			detected_edges.data[(detected_edges.cols*(start_y+j))+(start_x+i)]=detected_edges2.data[(detected_edges2.cols*j)+i];
		}



	remove_points_with_low_angle(&detected_edges, start_x, end_x, start_y, end_y);

	


//peek_found=1;
	if(peek_found){
		edges_only_tried=true;
		ellipse=find_best_edge(pic, &detected_edges, start_x, end_x, start_y, end_y,mean_dist, inner_color_range);

		if(ellipse.center.x<=0 || ellipse.center.x>=pic->cols || ellipse.center.y<=0 || ellipse.center.y>=pic->rows){
			ellipse.center.x=0;
			ellipse.center.y=0;
			ellipse.angle=0.0;
			ellipse.size.height=0.0;
			ellipse.size.width=0.0;
			peek_found=false;
		}
	}



	if(!peek_found){
		pos= th_angular_histo(pic, pic_th, start_x, end_x, start_y, end_y, threshold_up, th_histo,max_region_hole, min_region_size);

		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;

	}
	


	if(pos.x==0 && pos.y==0 && !edges_only_tried){
		ellipse=find_best_edge(pic, &detected_edges, start_x, end_x, start_y, end_y,mean_dist, inner_color_range);

		peek_found=true;
	}




	if(pos.x>0 && pos.y>0 && pos.x<pic->cols && pos.y<pic->rows && !peek_found){
		optimize_pos(pic, area_opt, &pos);

		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;
		zero_around_region_th_border(pic, &detected_edges, th_edges, threshold_up, edge_to_th, mean_dist, area_edges, &ellipse);
	}

  return ellipse;

}

inline static cv::Point detectPupil(cv::Mat *pic, cv::Mat *pic_th, cv::Mat *th_edges){
  cv::RotatedRect ellipse;
  ellipse = detectPupilEllipse(pic, pic_th, th_edges);
  return cv::Point( int(ellipse.center.x), int(ellipse.center.y) );
}

// static void extractEyeFeatures(cv::Mat frame, cv::Point &pupil_center, std::vector<cv::Point> *glints = NULL){
// cv::Mat pic_th,th_edges;
//   cv::cvtColor(frame, frame, CV_BGR2GRAY);
//   pic_th=cv::Mat(frame.rows, frame.cols, CV_8UC1);
//   th_edges=cv::Mat(frame.rows, frame.cols, CV_8UC1);
//   //extract pupil centers
//   pupil_center = detectPupil(&frame, &pic_th, &th_edges);
//   cv::RotatedRect pupil_ellipse = detectPupilEllipse(&frame, &pic_th, &th_edges);
//   pupil_center.x = pupil_ellipse.center.x;
//   pupil_center.y = pupil_ellipse.center.y;
//   if( glints != NULL ){
//     get_corneal_reflections( &pupil_ellipse, &frame, glints );
//   }
// }

static void extractEyeFeatures(cv::Mat frame, cv::Point &pupil_center, std::vector<cv::Point> *glints = NULL){
  // Invert the source image and convert to grayscale
  cv::Mat gray, gray2;
  cv::cvtColor(~frame, gray, CV_BGR2GRAY);
  //cv::cvtColor(~frame, gray2, CV_BGR2GRAY);

  // Convert to binary image by thresholding it
  cv::threshold(gray, gray, 220, 255, cv::THRESH_BINARY);

  // Find all contours
  std::vector<std::vector<cv::Point> > contours;
  cv::findContours(gray.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

  // Fill holes in each contour
  cv::drawContours(gray, contours, -1, CV_RGB(255,255,255), -1);

  // EyeRectangle *eye_rect  = NULL ;
  for (size_t i = 0; i < contours.size(); i++){
    double area = cv::contourArea(contours[i]);
    cv::Rect rect = cv::boundingRect(contours[i]);
    int radius = rect.width/2;

    // If contour is big enough and has round shape
    // Then it is the pupil
    if (area >= 30 && 
        std::abs(1 - ((double)rect.width / (double)rect.height)) <= 0.2 &&
        std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)	
      {
        pupil_center.x = rect.x + radius;
        pupil_center.y = rect.y + radius;
        if( glints != NULL ){
          cv::RotatedRect ellipse;
          ellipse.center.x=pupil_center.x;
          ellipse.center.y=pupil_center.y;
          ellipse.angle=0.0;
          ellipse.size.height=rect.height;
          ellipse.size.width=rect.width;
          // cv::Rect brect = ellipse.boundingRect();
          // cv::rectangle(frame, brect,  cv::Scalar(0,0,255),2);
          get_corneal_reflections( &ellipse, &gray, glints, 35, 2 );
          // corneal_reflection corneal;
          // corneal = corneal::locate_corneal_reflection(gray.clone(), pupil_center);
          // if( corneal.radius > 0 ) glints->push_back(corneal.center);
        }

      }
  }
}

#endif
