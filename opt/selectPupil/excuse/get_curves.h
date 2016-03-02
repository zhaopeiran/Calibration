#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define IMG_SIZE 1000 //400





static bool is_good_ellipse_eval(cv::RotatedRect *ellipse, cv::Mat *pic, int *erg){



	if(ellipse->center.x==0 && ellipse->center.y==0)
		return false;

	
	float x0 =ellipse->center.x;
	float y0 =ellipse->center.y;


	int st_x=x0-(ellipse->size.width/4.0);
	int st_y=y0-(ellipse->size.height/4.0);
	int en_x=x0+(ellipse->size.width/4.0);
	int en_y=y0+(ellipse->size.height/4.0);


	float val=0.0;
	float val_cnt=0;
	float ext_val=0.0;

	for(int i=st_x; i<en_x;i++)
		for(int j=st_y; j<en_y;j++){

			if(i>0 && i<pic->cols && j>0 && j<pic->rows ){
				val+=pic->data[(pic->cols*j)+i];
				val_cnt++;
			}
		}

	if(val_cnt>0)
		val=val/val_cnt;
	else
		return false;


	val_cnt=0;

	st_x=x0-(ellipse->size.width*0.75);
	st_y=y0-(ellipse->size.height*0.75);
	en_x=x0+(ellipse->size.width*0.75);
	en_y=y0+(ellipse->size.height*0.75);

	int in_st_x=x0-(ellipse->size.width/2);
	int in_st_y=y0-(ellipse->size.height/2);
	int in_en_x=x0+(ellipse->size.width/2);
	int in_en_y=y0+(ellipse->size.height/2);



	for(int i=st_x; i<en_x;i++)
		for(int j=st_y; j<en_y;j++){
			float x1=x0+i;
			float y1=y0+j;

			if(!(i>=in_st_x && i<=in_en_x && j>=in_st_y && j<=in_en_y))
			if(i>0 && i<pic->cols && j>0 && j<pic->rows ){
				ext_val+=pic->data[(pic->cols*j)+i];
				val_cnt++;

				//pic->at<char>(j,i)=255;
			}
		}




	if(val_cnt>0)
		ext_val=ext_val/val_cnt;
	else
		return false;

	/*
	cv::ellipse(*pic,*ellipse,150,2);
	imshow("ellipse",*pic);
	cvWaitKey(500);
	std::cout<<"val:"<<ext_val-val<<std::endl;
	*/



	val=ext_val-val;


	*erg=val;

	if(val>10) return true;
	else return false;
}







static std::vector<std::vector<cv::Point>> get_curves(cv::Mat *pic, cv::Mat *edge, int start_x, int end_x, int start_y, int end_y, double mean_dist, int inner_color_range){

	std::vector<std::vector<cv::Point>> all_curves;
	std::vector<cv::Point> curve;

	if(start_x<2) start_x=2;
	if(start_y<2) start_y=2;
	if(end_x>pic->cols-2) end_x=pic->cols-2;
	if(end_y>pic->rows-2) end_y=pic->rows-2;

	
	int all_curves_idx=0;
	int curve_idx=0;
	cv::Point mean_p;
	bool add_curve;
	int mean_inner_gray;
	int mean_inner_gray_last=1000000;
	
	//curve.reserve(1000);
	//all_curves.reserve(1000);

	all_curves.clear();

	bool check[IMG_SIZE][IMG_SIZE];

	for(int i=0; i<IMG_SIZE; i++)
		for(int j=0; j<IMG_SIZE; j++)
			check[i][j]=0;


	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){

			
			

			if(edge->data[(edge->cols*(j))+(i)]==255 && !check[i][j]){
				check[i][j]=1;
				
				curve.clear();
				curve_idx=0;

				curve.push_back(cv::Point(i,j));
				mean_p.x=i;
				mean_p.y=j;
				curve_idx++;

				
				int akt_idx=0;

				while(akt_idx<curve_idx){

					cv::Point akt_pos=curve[akt_idx];
					for(int k1=-1;k1<2;k1++)
						for(int k2=-1;k2<2;k2++){

							if(akt_pos.x+k1>=start_x && akt_pos.x+k1<end_x && akt_pos.y+k2>=start_y && akt_pos.y+k2<end_y)
							if(!check[akt_pos.x+k1][akt_pos.y+k2] )
								if( edge->data[(edge->cols*(akt_pos.y+k2))+(akt_pos.x+k1)]==255){
								check[akt_pos.x+k1][akt_pos.y+k2]=1;
								
								mean_p.x+=akt_pos.x+k1;
								mean_p.y+=akt_pos.y+k2;
								curve.push_back(cv::Point(akt_pos.x+k1,akt_pos.y+k2));
								curve_idx++;
							}
							
					}
					akt_idx++;

				}





				if(curve_idx>0 && curve.size()>0){

				int ergebniss=0;

				add_curve=true;
				mean_p.x=floor(( double(mean_p.x)/double(curve_idx) )+0.5);
				mean_p.y=floor(( double(mean_p.y)/double(curve_idx) )+0.5);
				for(int i=0;i<curve.size();i++) 
					if(  abs(mean_p.x-curve[i].x)<= mean_dist && abs(mean_p.y-curve[i].y) <= mean_dist)
						add_curve=false;


				//is ellipse fit possible
				if(add_curve){
					cv::RotatedRect ellipse=cv::fitEllipse( cv::Mat(curve) );

					if(ellipse.center.x<0 || ellipse.center.y<0 || 
						ellipse.center.x>pic->cols || ellipse.center.y>pic->rows){

						add_curve=false;
					}

					if(ellipse.size.height > 2.0*ellipse.size.width ||
						ellipse.size.width > 2.0*ellipse.size.height){

						add_curve=false;
					}

					
					if(add_curve){
						if(!is_good_ellipse_eval(&ellipse,pic,&ergebniss))
							add_curve=false;
					}
					
				}





				if(add_curve) {
					
//std::cout<<"in"<<mean_p.x<<":"<<mean_p.y<<std::endl;
					if(inner_color_range>0){
						mean_inner_gray=0;
						//calc inner mean
						for(int i=0;i<curve.size();i++){

							if(pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x)]!=0 || pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x)]!=0 )
								if( sqrt( pow(double(curve[i].y-mean_p.y),2) + pow(double(curve[i].x-mean_p.x)+2,2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y),2) + pow(double(curve[i].x-mean_p.x)-2,2)) )

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y))+(curve[i].x + 2)];
								else
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y))+(curve[i].x - 2)];


							else if(pic->data[(pic->cols*(curve[i].y))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y))+(curve[i].x-1)]!=0 )
								if( sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x),2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x),2)) )

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x)];
								else
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x)];


							else if(pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x-1)]!=0 )
								if( sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x+2),2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x-2),2)) )

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x+2)];
								else
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x-2)];


							else if(pic->data[(pic->cols*(curve[i].y-1))+(curve[i].x+1)]!=0 || pic->data[(pic->cols*(curve[i].y+1))+(curve[i].x-1)]!=0 )
								if( sqrt( pow(double(curve[i].y-mean_p.y+2),2) + pow(double(curve[i].x-mean_p.x+2),2)) < 
									sqrt( pow(double(curve[i].y-mean_p.y-2),2) + pow(double(curve[i].x-mean_p.x-2),2)) )

									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y+2))+(curve[i].x+2)];
								else
									mean_inner_gray+=(unsigned char)pic->data[(pic->cols*(curve[i].y-2))+(curve[i].x-2)];
                        
                        


							//mean_inner_gray+=pic->data[( pic->cols*( curve[i].y+((mean_p.y-curve[i].y)/2) ) ) + ( curve[i].x+((mean_p.x-curve[i].x)/2) )];
						

						}
						mean_inner_gray=floor(( double(mean_inner_gray)/double(curve.size()) )+0.5);

						if(mean_inner_gray_last>(mean_inner_gray+inner_color_range)){
							mean_inner_gray_last=mean_inner_gray;
							all_curves.clear();
							all_curves.push_back(curve);
						}else if(mean_inner_gray_last<=(mean_inner_gray+inner_color_range) && mean_inner_gray_last>=(mean_inner_gray-inner_color_range)){

							if(curve.size()>all_curves[0].size()){
								mean_inner_gray_last=mean_inner_gray;
								all_curves.clear();
								all_curves.push_back(curve);
							}
						}
					}
					else
						all_curves.push_back(curve);


				}


				}



					

			}


		}

		
		/*
		std::cout<<all_curves.size()<<std::endl;
		
		for(int i=0;i<1;i++)
			for(int j=0;j<all_curves[i].size();j++)
			 std::cout<<all_curves[i][j].x<<";"<<all_curves[i][j].y<<std::endl;
		
		
		cv::Mat m = cv::Mat::zeros(edge->rows, edge->cols, CV_8U);

		for(int i=0;i<all_curves.size();i++)
			for(int j=0;j<all_curves[i].size();j++)
				m.data[(edge->cols*all_curves[i][j].y)+(all_curves[i][j].x)]=255;

		imshow("ddd",m);
		*/


		return all_curves;
}
