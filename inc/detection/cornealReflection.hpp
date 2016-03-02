#ifndef _CORNEAL_REFLECTION_HPP
#define _CORNEAL_REFLECTION_HPP

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

static void gen_blob(int rad, cv::Mat *all_mat){

  int len=1+(4*rad);
  int c0=rad*2;
  float negis=0;
  float posis=0;

  *all_mat = cv::Mat::zeros(len, len, CV_32FC1);

  float *p;
  for(int i=-rad*2;i<=rad*2;i++){ //height
    p=all_mat->ptr<float>(c0+i);

    for(int j=-rad*2;j<=rad*2;j++){

      if(i<-rad || i>rad){ //pos
        p[c0+j]=-1;
        negis++;

      }else{ //neg

        int sz_w=sqrt( float(rad*rad) - float(i*i) );

        if(abs(j)<=sz_w){
          p[c0+j]=1;
          posis++;
        }else{
          p[c0+j]=-1;
          negis++;
        }
      }
    }
  }
	
	for(int i=0;i<len;i++){
		p=all_mat->ptr<float>(i);

		for(int j=0;j<len;j++){
			if(p[j]>0){
				p[j]=1.0/posis;
			}else{
				p[j]=-1.0/negis;
			}

		}
	}
	
	/*
    cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
    imshow("plobiiii",*all_mat);
    cvWaitKey(1000);
	*/
}






static void get_corneal_reflections(cv::RotatedRect *ellipse, cv::Mat *pic, std::vector<cv::Point> *glints, uint quality, uint maxReflections){

	cv::normalize(*pic, *pic, 0, 255, cv::NORM_MINMAX, CV_8U);

	float a=ellipse->size.width*3;
	float b=ellipse->size.height*3;

	float x0 =ellipse->center.x;
	float y0 =ellipse->center.y;

	int sx=0;
	int sy=0;
	int ex=0;
	int ey=0;

  uint totalReflections = 0;
  cv::Point *reflections = new cv::Point[maxReflections];
  // float reflections_value[maxReflections];

	if(a>0 && a<pic->cols && b>0 && x0>0 && b<pic->rows && y0>0 && x0<pic->cols && y0<pic->rows){

		//calc part img size
		sx=x0-a;
		sy=y0-b;
		ex=x0+a;
		ey=y0+b;

		if(sx<0) sx=0;
		if(sy<0) sy=0;
		if(ex>pic->cols) ex=pic->cols-1;
		if(ey>pic->rows) ey=pic->rows-1;

		cv::Mat part=cv::Mat::zeros(ey-sy,ex-sx,CV_8U);

		//extract part of image
		for(int i=0; i<ey-sy;i++)
			for(int j=0; j<ex-sx;j++){
				if(i+sy>0 && i+sy<pic->rows && j+sx>0 && j+sx<pic->cols && 
           i>0 && i<part.rows && j>0 && j<part.cols){

          //if(pic->data[(part.cols*(i+sy))+(j+sx)]>=100)
          part.data[(part.cols*(i))+(j)]=pic->data[(pic->cols*(i+sy))+(j+sx)];
				}
			}

    // cv::normalize(part, part, 0, 255, cv::NORM_MINMAX, CV_8U);
    // cv::imshow("part",part);
    // cv::waitKey(10);

		//convolve blob
		cv::Mat blob, result;
		gen_blob(3, &blob);

		part.convertTo(part, CV_32FC1);
		filter2D(part, result, -1 , blob, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

		//find max positions
    uint ind=0;
		float maxi=quality; //quality of the algorithm from 0 to 255 (scale of gray) 
		float *p;
		for(int i=(b*0.5); i<result.rows-(b*0.5);i++){
			p=result.ptr<float>(i);
			for(int j=(a*0.5); j<result.cols-(a*0.5);j++){
				if(i>0 && i<result.rows && j>0 && j<result.cols){				
					if(p[j]>maxi){
						maxi=p[j];
            // reflections_value[ind] = p[j];
            reflections[ind].x = j;
            reflections[ind].y = i;
            ind = (ind + 1) % maxReflections;
            totalReflections++;
					}
				}
			}
		}

    // result.convertTo(result, CV_8U);
    // cv::normalize(result, result, 0, 255, cv::NORM_MINMAX, CV_8U);
    // cv::imshow("result",result);
    // cv::waitKey(10);
	}


  //copy the results to the glints vector
  for(uint idx=0; idx<totalReflections && idx<maxReflections; idx++){
    if(reflections[idx].x>0 && reflections[idx].x<pic->cols && reflections[idx].y>0 && reflections[idx].y<pic->rows){
      glints->push_back( cv::Point(reflections[idx].x + sx, reflections[idx].y + sy ) );
      // cout << reflections_value[idx] << endl;

      // pic->data[(pic->cols*(reflections[idx].y))+(reflections[idx].x)]=0;
      // pic->data[(pic->cols*(reflections[idx].y))+(reflections[idx].x+1)]=0;
      // pic->data[(pic->cols*(reflections[idx].y))+(reflections[idx].x+2)]=0;
      // pic->data[(pic->cols*(reflections[idx].y))+(reflections[idx].x-1)]=0;
      // pic->data[(pic->cols*(reflections[idx].y))+(reflections[idx].x-2)]=0;
      // pic->data[(pic->cols*(reflections[idx].y+1))+(reflections[idx].x)]=0;
      // pic->data[(pic->cols*(reflections[idx].y+2))+(reflections[idx].x)]=0;
      // pic->data[(pic->cols*(reflections[idx].y-1))+(reflections[idx].x)]=0;
      // pic->data[(pic->cols*(reflections[idx].y-2))+(reflections[idx].x)]=0;

    }
  }

  delete reflections;
}


#endif //_CORNEAL_REFLECTION_HPP
