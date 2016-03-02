
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "peek.h"
#include "find_best_edge.h"
#include "th_angular_histo.h"

#include "optimize_pos.h"
#include "zero_around_region_th_border.h"
#include "rgb2gray_matlab.h"


#include "canny_ml.h"


static bool is_possible_pupil(cv::Mat *pic, cv::RotatedRect ellipse){

	int maximum=ellipse.size.height>ellipse.size.width?ellipse.size.height : ellipse.size.width;
	float fak_big=0.65;
	float fak_mid=0.5;
	float fak_inner=0.35;

	if(maximum<=0) {
		maximum=80;
	}
	



	int start_x=ellipse.center.x - (maximum*fak_big);
	int end_x=ellipse.center.x + (maximum*fak_big);
	int start_y=ellipse.center.y - (maximum*fak_big);
	int end_y=ellipse.center.y + (maximum*fak_big);


	if(start_x<0) start_x=0;
	if(start_y<0) start_y=0;
	if(end_x<pic->cols) end_x=pic->cols;
	if(end_y<pic->rows) end_y=pic->rows;




	int st_in_x=ellipse.center.x - (maximum*fak_mid);
	int en_in_x=ellipse.center.x + (maximum*fak_mid);
	int st_in_y=ellipse.center.y - (maximum*fak_mid);
	int en_in_y=ellipse.center.y + (maximum*fak_mid);

	if(st_in_x<0) st_in_x=0;
	if(st_in_y<0) st_in_y=0;
	if(en_in_x<pic->cols) en_in_x=pic->cols;
	if(en_in_y<pic->rows) en_in_y=pic->rows;


	int st_off_x=ellipse.center.x - (maximum*fak_inner);
	int en_off_x=ellipse.center.x + (maximum*fak_inner);
	int st_off_y=ellipse.center.y - (maximum*fak_inner);
	int en_off_y=ellipse.center.y + (maximum*fak_inner);

	if(st_off_x<0) st_off_x=0;
	if(st_off_y<0) st_off_y=0;
	if(en_off_x<pic->cols) en_off_x=pic->cols;
	if(en_off_y<pic->rows) en_off_y=pic->rows;



	






	int val_in=0;
	int cnt_in=0;
	int val_out=0;
	int cnt_out=0;

	for(int i=start_x; i<=end_x; i++)
		for(int j=start_y; j<=end_y; j++){


			if( !(i>=st_off_x && i<=en_off_x && j>=st_off_y && j<=en_off_y) ){

			if(i>=0 && i<pic->cols && j>=0 && j<pic->rows){
			if(i>=st_in_x && i<=en_in_x && j>=st_in_y && j<=en_in_y){
				cnt_in++;
				val_in+=pic->data[(pic->cols*j)+i];
			}else{
				cnt_out++;
				val_out+=pic->data[(pic->cols*j)+i];
			}
			}

			}


		}


	if(cnt_in>0 && cnt_out>0){
		val_in=val_in/cnt_in;
		val_out=val_out/cnt_out;

		if(val_out-val_in > 0)
			return true;
		else
			return false;
	}
	else
		return false;



}


static void mean_under_mean(cv::Mat *pic, int win){

	cv::Mat ergebniss= cv::Mat::zeros(pic->rows, pic->cols, CV_8UC1);
	cv::Mat ergebniss_under= cv::Mat::zeros(pic->rows, pic->cols, CV_8UC1);
	
	cv::boxFilter(*pic,ergebniss,-1,cv::Size(win+win+1,win+win+1));

	int max=0;
	int min=1000;

	for(int i=win; i<pic->cols-win; i++)
		for(int j=win; j<pic->rows-win; j++){

			int cnt=0;
			int val=0;

			for(int k1=-win; k1<=win; k1++)
				for(int k2=-win; k2<=win; k2++){
					if(	(unsigned char)pic->at<char>(j+k1,i+k2)
						<=
						(unsigned char)ergebniss.at<char>(j+k1,i+k2) ){

						cnt++;
						val+=(unsigned char)pic->at<char>(j+k1,i+k2);
					}
				}

			if(cnt==0) ergebniss_under.at<char>(j,i)=pic->at<char>(j,i);
			else ergebniss_under.at<char>(j,i)= val/cnt;


			if((unsigned char)ergebniss_under.at<char>(j,i)>max)
				max=(unsigned char)ergebniss_under.at<char>(j,i);
			if((unsigned char)ergebniss_under.at<char>(j,i)<min)
				min=(unsigned char)ergebniss_under.at<char>(j,i);

		}

/*
		for(int i=win; i<pic->cols-win; i++)
			for(int j=win; j<pic->rows-win; j++){
				pic->at<char>(j,i)= (double((unsigned char)ergebniss_under.at<char>(j,i)-min)/double(max-min))*255;
			}
			*/
		*pic=ergebniss_under;


}




void find_circ(cv::RotatedRect *ellipse, cv::Mat *pic){

	float val=0.0;
	int val_bad=0;
	int val_good=0;

	float val_max=0.0;
	int i_max=0;

	float x0 =ellipse->center.x;
	float y0 =ellipse->center.y;

	for(int i=6; i<75;i++){

		for(int angle=0; angle<360; angle+=1){

		float radi= (2*3.14159265359*float(angle))/360.0;

		
		float x_in = (i*cos(radi)) + x0;
		float y_in = (i*sin(radi)) + y0;

		float x_out = ((i+1)*cos(radi)) + x0;
		float y_out = ((i+1)*sin(radi)) + y0;



		if(x_in>0 && x_in<pic->cols && y_in>0 && y_in<pic->rows && 
			x_out>0 && x_out<pic->cols && y_out>0 && y_out<pic->rows ){

				if(pic->at<char>(y_out,x_out)>pic->at<char>(y_in,x_in))
					val_good++;
				else
					val_bad++;

				val+=pic->at<char>(y_out,x_out)-pic->at<char>(y_in,x_in);


				val=val*(float(val_good)/float(val_good+val_bad));

				if(val>val_max){
					val_max=val;
					i_max=i;
				}

				
				//pic->at<char>(py_in,px_in)=255;
				//pic->at<char>(py_out,px_out)=150;
		}



	}
	}

	ellipse->size.width=i_max*2;
	ellipse->size.height=i_max*2;



}



bool is_ellipse(cv::RotatedRect *ellipse, cv::Mat *pic){

	
	float val=0.0;
	int val_bad=0;
	int val_good=0;
	
	float a=ellipse->size.width/2.0;
	float b=ellipse->size.height/2.0;


	if(a<=5 || b<=5){
		a=0;
		b=0;

		ellipse->size.width=0;
		ellipse->size.height=0;
	}





	float x0 =ellipse->center.x;
	float y0 =ellipse->center.y;

	float r=ellipse->angle;



	//coarse pos find circle
	if(a==0 && b==0) find_circ(ellipse, pic);


	a=ellipse->size.width/2.0;
	b=ellipse->size.height/2.0;




if(a>0 && b>0){


	r= (2*3.14159265359*float(r))/360.0;

	float rot_mat[4];

	rot_mat[0]=cos(r);
	rot_mat[1]=-sin(r);
	rot_mat[2]=sin(r);
	rot_mat[3]=cos(r);

	
	


	for(int pix=3; pix<=4; pix++){
		float v_a_in=a-pix;
		float v_b_in=b-pix;

		float v_a_out=a+pix;
		float v_b_out=b+pix;
	for(int angle=0; angle<360; angle+=1){

		float radi= (2*3.14159265359*float(angle))/360.0;

		
		float x_in = (v_a_in*cos(radi));
		float y_in = (v_b_in*sin(radi));

		float x_out = (v_a_out*cos(radi));
		float y_out = (v_b_out*sin(radi));


		float px_in= x_in*rot_mat[0] + y_in*rot_mat[1] + x0;
		float py_in= x_in*rot_mat[2] + y_in*rot_mat[3] + y0;

		float px_out= x_out*rot_mat[0] + y_out*rot_mat[1] + x0;
		float py_out= x_out*rot_mat[2] + y_out*rot_mat[3] + y0;


		/* real ellipse
		float x = (a*cos(radi));
		float y = (b*sin(radi));

		float a1= x*rot_mat[0] + y*rot_mat[1];
		float b1= x*rot_mat[2] + y*rot_mat[3];

		a1=x0+a1;
		b1=y0+b1;
		*/

		if(px_in>0 && px_in<pic->cols && py_in>0 && py_in<pic->rows && 
			px_out>0 && px_out<pic->cols && py_out>0 && py_out<pic->rows ){

				int h_big=(unsigned int)pic->at<char>(py_out,px_out);
				int h_small=(unsigned int)pic->at<char>(py_in,px_in);

				if(h_big>h_small)
					val_good++;
				else
					val_bad++;

				val+=h_big-h_small;

				
				//pic->at<char>(py_in,px_in)=255;
				//pic->at<char>(py_out,px_out)=150;
		}



	}
	}

}




	cv::ellipse(*pic,*ellipse,150,2);


	imshow("ellipse",*pic);

	cvWaitKey(500);

	std::cout<<"val:"<<val<<"   bad:"<<val_bad<<"   good:"<<val_good<<std::endl;

	if(val_good>val_bad && val>0)
		return true;
	else
		return false;
}






#define MAX_RADI 50


void grow_region(cv::RotatedRect *ellipse, cv::Mat *pic){


	float mean=0.0;


	int x0 =ellipse->center.x;
	int y0 =ellipse->center.y;

	int mini=1000;
	int maxi=0;


		for(int i=-2;i<3;i++)
			for(int j=-2;j<3;j++){

				if(y0+j>0 && y0+j<pic->rows && x0+i>0 && x0+i<pic->cols){
				if(mini>pic->data[(pic->cols*(y0+j))+(x0+i)])
					mini=pic->data[(pic->cols*(y0+j))+(x0+i)];

				if(maxi<pic->data[(pic->cols*(y0+j))+(x0+i)])
					maxi=pic->data[(pic->cols*(y0+j))+(x0+i)];

				mean+=pic->data[(pic->cols*(y0+j))+(x0+i)];
				}

			}

		mean=mean/25.0;

		float diff=abs(mean-pic->data[(pic->cols*(y0))+(x0)]);

		int th_up=ceil(mean+diff)+1;
		int th_down=floor(mean-diff)-1;

		int radi=0;

		for(int i=1;i<MAX_RADI;i++){
			radi=i;

			int left=0;
			int right=0;
			int top=0;
			int bottom=0;

			for(int j=-i;j<=1+(i*2);j++){
				

				//left
				if(y0+j>0 && y0+j<pic->rows && x0+i>0 && x0+i<pic->cols)
				if(pic->data[(pic->cols*(y0+j))+(x0+i)]>th_down && pic->data[(pic->cols*(y0+j))+(x0+i)]<th_up){
					left++;
//pic->data[(pic->cols*(y0+j))+(x0+i)]=255;
				}

				//right
				if(y0+j>0 && y0+j<pic->rows && x0-i>0 && x0-i<pic->cols)
				if(pic->data[(pic->cols*(y0+j))+(x0-i)]>th_down && pic->data[(pic->cols*(y0+j))+(x0-i)]<th_up){
					right++;
//pic->data[(pic->cols*(y0+j))+(x0-i)]=255;
				}

				//top
				if(y0-i>0 && y0-i<pic->rows && x0+j>0 && x0+j<pic->cols)
				if(pic->data[(pic->cols*(y0-i))+(x0+j)]>th_down && pic->data[(pic->cols*(y0-i))+(x0+j)]<th_up){
					top++;
//pic->data[(pic->cols*(y0-i))+(x0+j)]=255;
				}

				//bottom
				if(y0+i>0 && y0+i<pic->rows && x0+j>0 && x0+j<pic->cols)
				if(pic->data[(pic->cols*(y0+i))+(x0+j)]>th_down && pic->data[(pic->cols*(y0+i))+(x0+j)]<th_up){
					bottom++;
//pic->data[(pic->cols*(y0+i))+(x0+j)]=255;
				}

			}



			//if less than 25% stop
			float p_left=float(left)/float(1+(2*i));
			float p_right=float(right)/float(1+(2*i));
			float p_top=float(top)/float(1+(2*i));
			float p_bottom=float(bottom)/float(1+(2*i));


			if(p_top<0.2 && p_bottom<0.2)
				break;

			if(p_left<0.2 && p_right<0.2)
				break;

		}

		ellipse->size.height=radi*2;
		ellipse->size.width=radi*2;




		//////////////////////////////////////////////////

/*
		//collect points in threashold
		cv::Mat ch_mat=cv::Mat::zeros(pic->rows, pic->cols, CV_8UC1);
		cv::Point2i coor;
		std::vector<cv::Point2i> all_points;

		ch_mat.data[(pic->cols*(y0))+(x0)]=1;
		coor.x=x0;
		coor.y=y0;
		all_points.push_back(coor);
		
		int all_p_idx=0;

		while(all_p_idx<all_points.size()){
			cv::Point2i ak_p=all_points.at(all_p_idx);
pic->data[(pic->cols*(ak_p.y))+(ak_p.x)]=255;

			for(int i=-1;i<2;i++)
				for(int j=-1;j<2;j++){



					if(ak_p.y+j>0 && ak_p.y+j<pic->rows && ak_p.x+i>0 && ak_p.x+i<pic->cols)
					if((int)ch_mat.data[(ch_mat.cols*(ak_p.y+j))+(ak_p.x+i)]==0 &&
						(int)pic->data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]>th_down &&
						(int)pic->data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]<th_up){

							coor.x=ak_p.x+i;
							coor.y=ak_p.y+j;
							ch_mat.data[(pic->cols*(ak_p.y+j))+(ak_p.x+i)]=1;
							all_points.push_back(coor);

					}


				}


			all_p_idx++;

//std::cout<<all_points.size()<<":"<<all_p_idx<<std::endl;
			
		}
*/
		////////////////////////////////////////////////////////









	//	imshow("ellipse",*pic);
	//	cvWaitKey(500);




	

}







bool is_good_ellipse(cv::RotatedRect *ellipse, cv::Mat *pic){



	if(ellipse->center.x==0 && ellipse->center.y==0)
		return false;

	if(ellipse->size.width==0 || ellipse->size.height==0)
		grow_region(ellipse, pic);

	
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




	if(val>10) return true;
	else return false;
}





static cv::RotatedRect run(cv::Mat *pic, cv::Mat *pic_th, cv::Mat *th_edges){


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



//if(ellipse.size.height>0 && ellipse.size.width>0.0){


if(is_good_ellipse(&ellipse, pic))
	return ellipse;
else{
	ellipse.center.x=0;
	ellipse.center.y=0;
	ellipse.angle=0.0;
	ellipse.size.height=0.0;
	ellipse.size.width=0.0;
	
	return ellipse;
}
	
//}

	return ellipse;
	/*
	if(is_possible_pupil(pic, ellipse))
		return ellipse;
	else{
		ellipse.center.x=0;
		ellipse.center.y=0;
		ellipse.angle=0.0;
		ellipse.size.height=0.0;
		ellipse.size.width=0.0;
		return ellipse;
	}
	*/
	
}








