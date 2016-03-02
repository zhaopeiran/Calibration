#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


static cv::Mat rgb2gray(cv::Mat *pic){


	//find min max
	int min=1000;
	int	max=0;

	for(int k=0; k<3; k++)
	for(int i=0; i<pic->cols; i++)
		for(int j=0; j<pic->rows; j++){
			if(pic->at<cv::Vec3b>(j,i)[k]<min) min=pic->at<cv::Vec3b>(j,i)[k];
			if(pic->at<cv::Vec3b>(j,i)[k]>max) max=pic->at<cv::Vec3b>(j,i)[k];
		}

	//normalize
	for(int k=0; k<3; k++)
	for(int i=0; i<pic->cols; i++)
		for(int j=0; j<pic->rows; j++){
			float help=(float)pic->at<cv::Vec3b>(j,i)[k];

			pic->at<cv::Vec3b>(j,i)[k]= floor( ( (help-float(min))/float(max-min) )*255 +0.5);
		}



	cv::Mat m = cv::Mat::zeros(pic->rows, pic->cols, CV_8U);
	
	float gray_transform[3] = {0.1140,0.5870,0.2989};

	for(int i=0; i<m.cols; i++)
		for(int j=0; j<m.rows; j++){
			m.at<char>(j,i)= floor( (float(pic->at<cv::Vec3b>(j,i)[0])*gray_transform[0]) + 
									(float(pic->at<cv::Vec3b>(j,i)[1])*gray_transform[1]) + 
									(float(pic->at<cv::Vec3b>(j,i)[2])*gray_transform[2]) + 
									0.5);
		}


	return m;

}