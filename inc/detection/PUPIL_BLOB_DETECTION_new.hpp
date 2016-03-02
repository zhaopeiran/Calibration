#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


namespace PUPIL_BLOB_DETECTION{


static void gen_blob_neu(int rad, cv::Mat *all_mat, cv::Mat *all_mat_neg){


		int len=1+(4*rad);
		int c0=rad*2;
		float negis=0;
		float posis=0;

		*all_mat = cv::Mat::zeros(len, len, CV_32FC1);
		*all_mat_neg = cv::Mat::zeros(len, len, CV_32FC1);


		float *p, *p_neg;
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
		p_neg=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len;j++){

			if(p[j]>0){
				p[j]=1.0/posis;
				p_neg[j]=1.0/posis;
			}else{
				p[j]=-1.0/negis;
				p_neg[j]=0.0;
			}

		}
	}



		


	
	/*
	cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("plob",*all_mat);
	cv::normalize(*all_mat_neg, *all_mat_neg, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("average",*all_mat_neg);
	cvWaitKey(1000);
	*/
	


}


static std::vector<cv::RotatedRect> blob_finder(cv::Mat *pic){

	cv::normalize(*pic,*pic,0, 255, cv::NORM_MINMAX, CV_8UC1);

	cv::Mat img(pic->rows/4,pic->cols/4,CV_8UC1);
	cv::resize(*pic,img,img.size());


	int rad=6;


	cv::Point pos(0,0);

	float *p_res, *p_blob;
	cv::Mat result(img.rows,img.cols,CV_32FC1);
	cv::Mat res_blob,res_avg,blob_mat,avg_mat;

	gen_blob_neu(rad,&blob_mat,&avg_mat);
	

	img.convertTo(img, CV_32FC1);
	filter2D(img, res_blob, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );
	//filter2D(img, res_avg, -1 , avg_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );



	for(int i=20; i<result.rows;i++){ //ignore 20 first becouse of advertisement
		p_res=result.ptr<float>(i);
		//p_avg=res_avg.ptr<float>(i);
		p_blob=res_blob.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
			if(p_blob[j]>15)
				p_res[j]=p_blob[j]*p_blob[j];
			else
				p_res[j]=0;
		}
	}
	

	
	//cv::normalize(result,result,0, 255, cv::NORM_MINMAX, CV_8UC1);


	float lmin=100000000;
	float lmax=0;
	float rmin=100000000;
	float rmax=0;
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);
		for(int j=0; j<result.cols/2;j++){
			if(p_res[j]<lmin)
				lmin=p_res[j];
			if(p_res[j]>lmax)
				lmax=p_res[j];
		}
		for(int j=result.cols/2; j<result.cols;j++){
			if(p_res[j]<rmin)
				rmin=p_res[j];
			if(p_res[j]>rmax)
				rmax=p_res[j];
		}
	}
	//norm
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);
		for(int j=0; j<result.cols/2;j++){
			p_res[j]= ((p_res[j]-lmin)/(lmax-lmin))*255;
		}
		for(int j=result.cols/2; j<result.cols;j++){
			p_res[j]= ((p_res[j]-rmin)/(rmax-rmin))*255;
		}
	}
	result.convertTo(result, CV_8UC1);




	cv::threshold(result,result,50,255,cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours( result, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );


	std::vector<cv::RotatedRect> minEllipse( contours.size() );

	for( size_t i = 0; i < contours.size(); i++ ){
       if( contours[i].size() > 5 ){ 
		   minEllipse[i] = cv::fitEllipse( cv::Mat(contours[i]) ); 
		   minEllipse[i].center.x=minEllipse[i].center.x*4;
		   minEllipse[i].center.y=minEllipse[i].center.y*4;
		   minEllipse[i].size.width=minEllipse[i].size.width*5;
		   minEllipse[i].size.height=minEllipse[i].size.height*5;
	   }
     }




	/*
	for(int i=1; i<result.rows-1;i++){
		for(int j=1; j<result.cols-1;j++){

			if(result.data[(result.cols*(i))+(j)]==255){
				bool found_black = false;
				for(int k1=-1;k1<=1 && !found_black;k1++)
					for(int k2=-1;k2<=1 && !found_black;k2++){
						if(result.data[(result.cols*(i+k1))+(j+k2)]==0)
							found_black=true;
					}

				if(!found_black)
					result.data[(result.cols*(i))+(j)]=100;
			}

		}
	}



	std::vector<std::vector<cv::Point>> all_lines;
	std::vector<cv::Point> line;


	for(int i=1; i<result.rows-1;i++){
		for(int j=1; j<result.cols-1;j++){

			if(result.data[(result.cols*(i))+(j)]==255){
				line.clear();
				result.data[(result.cols*(i))+(j)]=200;
				line.push_back(cv::Point(j,i));
				int pos=0;

				while(pos<line.size()){

					cv::Point ak_point=line[pos];

					for(int k1=-1;k1<=1;k1++)
						for(int k2=-1;k2<=1;k2++)
							if(ak_point.y+k1>0 && ak_point.y+k1<result.rows && ak_point.x+k2>0 && ak_point.x+k2 < result.cols)
							if(result.data[(result.cols*(ak_point.y+k1))+(ak_point.x+k2)]==255){
								result.data[(result.cols*(ak_point.y+k1))+(ak_point.x+k2)]=200;
								line.push_back(cv::Point((ak_point.x+k2),(ak_point.y+k1)));

							}

							
					pos++;
				}



				all_lines.push_back(line);


			}

		}
	}


	
	cv::RotatedRect ellipse[2];

	for(int i=0;i<all_lines.size() && i<2;i++){

		ellipse[i]=cv::fitEllipse(cv::Mat(all_lines[i]));
		ellipse[i].center.x=ellipse[i].center.x*4;
		ellipse[i].center.y=ellipse[i].center.y*4;
		ellipse[i].size.width=ellipse[i].size.width*4;
		ellipse[i].size.height=ellipse[i].size.height*4;

std::cout<<i<<":"<<ellipse[i].size.width<<":"<<ellipse[i].size.height<<std::endl;

	}



	*/
	





	/*
	cv::imshow("img",*pic);
	cv::imshow("pupils",result);
	cv::waitKey(200);
	*/


	
	return minEllipse;

}










}
