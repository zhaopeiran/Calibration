#ifndef __PUPIL_BLOB_DETECTION_HPP__
#define __PUPIL_BLOB_DETECTION_HPP__


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


namespace PUPIL_BLOB_DETECTION{


static void gen_blob_neu(int rad, cv::Mat *all_mat){


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
	imshow("plob",*all_mat);
	cv::normalize(*all_mat_neg, *all_mat_neg, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("average",*all_mat_neg);
	cvWaitKey(1000);
	*/
	


}





static std::vector<cv::RotatedRect> blob_finder(cv::Mat *pic){

	

	cv::Mat cppic;
	pic->copyTo(cppic);

	cv::normalize(cppic,cppic,0, 255, cv::NORM_MINMAX, CV_8UC1);

	cv::Mat img(cppic.rows/4,cppic.cols/4,CV_8UC1);
	cv::resize(cppic,img,img.size());


	int rad=6;


	cv::Point pos(0,0);
    // float abs_max=0;

    // float *p_res, *p_avg, *p_blob;
    float *p_res, *p_blob;
	cv::Mat result = cv::Mat::zeros(img.rows,img.cols,CV_32FC1);
	cv::Mat res_blob,res_avg,blob_mat;

	gen_blob_neu(rad,&blob_mat);
	

	img.convertTo(img, CV_32FC1);
	filter2D(img, res_blob, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

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
  // cout << "RESULT1: " << (result.ptr<float>(0))[0] << endl;

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

  // cout << "RESULT2: " << (result.ptr<float>(0))[0] << endl;

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

  // cout << "RESULT3: " << (result.ptr<float>(0))[0] << endl;

	result.convertTo(result, CV_8UC1);

  // cout << "RESULT4: " << (result.ptr<float>(0))[0] << endl;

	cv::threshold(result,result,100,255,cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours( result, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );


	std::vector<cv::RotatedRect> minEllipse;

    for( size_t i = 0; i < contours.size(); i++ ){
       if( contours[i].size() > 5 ){ 

		   cv::RotatedRect ak_el=cv::fitEllipse( cv::Mat(contours[i]) );

		   ak_el.center.x=ak_el.center.x*4;
		   ak_el.center.y=ak_el.center.y*4;
		   ak_el.size.width=ak_el.size.width*10;
		   ak_el.size.height=ak_el.size.height*10;
		   ak_el.center.x=ak_el.center.x-(ak_el.size.width/2);
		   ak_el.center.y=ak_el.center.y-(ak_el.size.height/2);

		   minEllipse.push_back(ak_el);
	   }
  }

  // for(size_t i = 0; i<minEllipse.size(); i++) cout << "MINIELLIPSE: " << minEllipse[i].center << endl;
	
	std::vector<cv::RotatedRect> pupiEllipse;

	pupiEllipse.clear();
	cv::Mat pupil_region;


	/*
	for( size_t k = 0; k < minEllipse.size(); k++ )
		cv::ellipse(*pic,minEllipse[k],cv::Scalar(255,255,255),1,8);
	cv::imshow("all posis",*pic);
	cv::waitKey(10000);
	*/


	for( size_t k = 0; k < minEllipse.size(); k++ ){
		if(minEllipse[k].size.height>25 && minEllipse[k].size.width>25 && 
			minEllipse[k].center.x>0 && minEllipse[k].center.y>0 && 
			minEllipse[k].size.width+minEllipse[k].center.x<cppic.cols && minEllipse[k].size.height+minEllipse[k].center.y<cppic.rows){
	
		pupil_region=cv::Mat::zeros(minEllipse[k].size.height,minEllipse[k].size.width,CV_8UC1);


		for(int i=0; i<pupil_region.rows;i++)
			for(int j=0; j<pupil_region.cols;j++)
				if(int(minEllipse[k].center.y+i)>0 && int(minEllipse[k].center.y+i)<cppic.rows && minEllipse[k].center.x+j>0 && minEllipse[k].center.x+j<cppic.cols){
					pupil_region.data[(pupil_region.cols*i)+j]=cppic.data[( cppic.cols*int(minEllipse[k].center.y+i)) + int(minEllipse[k].center.x+j)];
				}

		/*
		cv::imshow("pr",pupil_region);
		cv::waitKey(10000);
		*/
		
		cv::normalize(pupil_region,pupil_region,0, 255, cv::NORM_MINMAX, CV_8UC1);

	
		//histo seperation
		int histi[256];
		for(int i=0; i<256;i++)
			histi[i]=0;


		int safety_cnt=0;
		for(int i=0; i<pupil_region.rows;i++)
			for(int j=0; j<pupil_region.cols;j++)
        // if(pupil_region.data[(pupil_region.cols*i)+j] < 256){
						histi[pupil_region.data[(pupil_region.cols*i)+j]]++;

						safety_cnt++;
            //}

		if(safety_cnt<200)
			break;

		/*
		cv::Mat sh_hist=cv::Mat::zeros(800,256, CV_8UC1);
		for(int i=0; i<256;i++)
			for(int j=0; j<histi[i] && j<800;j++)
				sh_hist.data[(sh_hist.cols*(799-j))+i]=255;
	
		
		cv::imshow("histo",sh_hist);
		cv::waitKey(100);
		*/

	
		//th calc
		int window=25;


		int maxima[100];
		int maxima_idx=0;
		for(int i=0; i<100;i++)
			maxima[i]=0;

		for(int i=0; i<256;i++){
			bool bigest=true;
			for(int j=-window; j<=window && bigest;j++){
				if(i+j>=0 && i+j<256 && histi[i+j]>histi[i])
					bigest=false;
			}

			if(bigest)
				maxima[maxima_idx++]=i;

		}

		if(maxima_idx<2) return pupiEllipse;


		int max1=-1;

		for(int i=0; i<maxima_idx;i++){
			if(max1==-1 || histi[maxima[i]]>histi[max1]){
				max1=maxima[i];
			}
		}

		int max2=-1;

		for(int i=0; i<maxima_idx;i++){
			if((max2==-1 || histi[maxima[i]]>histi[max2]) && maxima[i]!=max1){
				max2=maxima[i];
			}
		}


		if(max1==-1 || max2==-1) return pupiEllipse;


		int m1=(max1>max2)?max1:max2;
		int m2=(max1<max2)?max1:max2;

		int min_val=1000000;
		int min_idx=0;

		for(int i=m2+1; i<m1;i++){
			int cnt=0;
			int val=0;
			for(int j=-window; j<=window;j++){
				if(i+j>0 && i+j<256){
					val+=histi[i+j];
					cnt++;
				}
			}

			if(cnt>0) 
				val=val/cnt;
			else
				val=0;

			if(val<min_val){
				min_val=val;
				min_idx=i;
			}

		}


		cv::threshold(pupil_region,pupil_region,min_idx,255,cv::THRESH_BINARY);

		/*
		std::cout<<max1<<":"<<max2<<"|"<<min_idx<<std::endl;
		cv::imshow("prth",pupil_region);
		cv::waitKey(1000);
		*/


		//contur
		contours.clear();
		hierarchy.clear();
		//cv::findContours( pupil_region, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

		cv::Mat border(pupil_region.rows,pupil_region.cols,CV_8UC1, cv::Scalar(0,0,0));

		for(int i=1;i<pupil_region.rows-1;i++)
			for(int j=1;j<pupil_region.cols-1;j++)
				if(pupil_region.data[(pupil_region.cols*i)+j]>0){
					bool has_black=false;

					for(int k1=-1;k1<=1;k1++)
						for(int k2=-1;k2<=1;k2++)
							if(pupil_region.data[(pupil_region.cols*(i+k1))+(j+k2)]==0)
								has_black=true;

					if(has_black)
						border.data[(border.cols*i)+j]=255;
				}


	
		/*
		cv::imshow("contur",border);
		cv::waitKey(10000);
		*/
		//collect lines
		std::vector<cv::Point> ak_line;
		for(int i=0;i<border.rows;i++)
			for(int j=0;j<border.cols;j++)
				if(border.data[(border.cols*i)+j]==255){

					ak_line.clear();
					border.data[(border.cols*i)+j]=200;
					ak_line.push_back(cv::Point(j,i));
              size_t ak_idx=0;

					while(ak_idx<ak_line.size()){
						cv::Point ak_pos=ak_line.at(ak_idx);

						for(int k1=-1;k1<=1;k1++)
							for(int k2=-1;k2<=1;k2++)
								if(ak_pos.y+k1>0 && ak_pos.y+k1<border.rows && ak_pos.x+k2>0 && ak_pos.x+k2<border.cols)
								if(border.data[(border.cols*(ak_pos.y+k1))+(ak_pos.x+k2)]==255){
									border.data[(border.cols*(ak_pos.y+k1))+(ak_pos.x+k2)]=200;
									ak_line.push_back(cv::Point(ak_pos.x+k2,ak_pos.y+k1));
								}

						ak_idx++;

					}

					if(ak_line.size()>10)
						contours.push_back(ak_line);

				}






	
		int index=-1;
        for( size_t i = 0; i < contours.size(); i++ )
			if(index==-1 || contours[index].size()<contours[i].size())
				index=i;


		//ellipse fit RANSAC like only better :-)
		if(index>=0)
		for( int i = index; i <=index; i++ ){
			if( contours[i].size() > 5 ){ 

				cv::RotatedRect best_pupi;
				best_pupi.size.width=100;
				best_pupi.size.height=1000;

				int step=contours[i].size()/4;
				std::vector<cv::Point> ak_set;

				for(int iter=0;iter<4;iter++){
					cv::RotatedRect ak_pupi;
					ak_set.clear();
					for(int set_st=step*iter;set_st<(step*iter)+(2*step);set_st++)
						ak_set.push_back(contours[i].at(set_st%contours[i].size()));
					


					if(ak_set.size()>5){
						ak_pupi=cv::fitEllipse( cv::Mat(ak_set) ); 
						/*
						cv::ellipse(pupil_region,ak_pupi,cv::Scalar(150,150,150),1,8);
						cv::imshow("ellipses",pupil_region);
						*/
					}

					if(ak_pupi.center.x>0 && ak_pupi.center.y>0 && ak_pupi.size.height>0  && ak_pupi.size.width>0 &&
						ak_pupi.size.area()>500 && ak_pupi.size.height-ak_pupi.size.width<10){

							if(abs(best_pupi.size.width-best_pupi.size.height)>abs(ak_pupi.size.width-ak_pupi.size.height))
								best_pupi=ak_pupi;
					}
				}

				if(best_pupi.center.x>0 && best_pupi.center.y>0 && best_pupi.size.height>0  && best_pupi.size.width>0 &&
					best_pupi.size.area()>500 && best_pupi.size.height-best_pupi.size.width<10){
						
						best_pupi.center.x+=minEllipse[k].center.x;
						best_pupi.center.y+=minEllipse[k].center.y;
						pupiEllipse.push_back(best_pupi);
				}

			}
		}


			
		}
	}

	minEllipse.clear();


	// std::vector<cv::RotatedRect> pupiEllipse_empty;
  // return pupiEllipse_empty;



	/*
	cv::imshow("img",*pic);
	cv::imshow("pupils",result);
	cv::waitKey(200);
	*/

  // cout << "SIZE: " << pupiEllipse.size() << endl;
	// if( pupiEllipse.size() > 0 ){ cout << "DATA: " << pupiEllipse[0].center.x << "," << pupiEllipse[0].center.y << endl; }
	return pupiEllipse;

}



// static double photorefraction(cv::Mat *pic, cv::RotatedRect el1){

// 	cv::Mat pattern_pupil_el1(pic->rows, pic->cols, CV_8UC1, cv::Scalar(0,0,0));
// 	cv::ellipse(pattern_pupil_el1,el1,cv::Scalar( 255, 255, 255), -1, 8);



	
// 	//remove cornea and eye lashes
// 	double eyelash_cnt=0;
// 	double cornea_cnt=0;
// 	double pupil_cnt=0;
// 	for(int i=0;i<pic->rows;i++)
// 		for(int j=0;j<pic->cols;j++){
// 			if(pattern_pupil_el1.data[(pic->cols*i)+j]>0){
// 				if(pic->data[(pic->cols*i)+j]<50){
// 					eyelash_cnt++;
// 					pattern_pupil_el1.data[(pic->cols*i)+j]=0;
// 				}

// 				if(pic->data[(pic->cols*i)+j]>200){
// 					cornea_cnt++;
// 					pattern_pupil_el1.data[(pic->cols*i)+j]=0;
// 				}
				
// 				pupil_cnt++;
// 			}
// 		}


// 	if(pupil_cnt==0)
// 		return 0;


// 	// bool has_cornea=false;
// 	// if(cornea_cnt/pupil_cnt  <0.02 && cornea_cnt/pupil_cnt >0)
// 	// 	has_cornea=true;

// 	// bool has_eyelash=false;
// 	// if(eyelash_cnt/pupil_cnt >0)
// 	// 	has_eyelash=true;

// //	std::cout<<"cornea:"<<cornea_cnt/pupil_cnt<<"   eyelash:"<<eyelash_cnt/pupil_cnt<<std::endl;




// 	//calc gradient
// 	std::vector<double> gradient;

// 	for(int i=0;i<pic->rows;i++){
// 		double cnt=0;
// 		double val=0;

// 		for(int j=0;j<pic->cols;j++){
// 			if(pattern_pupil_el1.data[(pic->cols*i)+j]>0){
// 				cnt++;
// 				val+=pic->data[(pic->cols*i)+j];
// 			}
// 		}
// 		if(cnt>0){
// 			val=val/cnt;
// 			gradient.push_back(val);
// 		}


// 	}


// 	if(gradient.size()<=5)
// 		return 0;



// 	//lineare regression
// 	double mean_x=0;
// 	double mean_y=0;
//     for(size_t i=0;i<gradient.size();i++){
// 		mean_x+=i;
// 		mean_y+=gradient.at(i);
// 	}
// 	mean_x=mean_x/gradient.size();
// 	mean_y=mean_y/gradient.size();

// 	double var_x=0;
// 	double var_xy=0;

//     for(size_t i=0;i<gradient.size();i++){
// 		var_x+=(i-mean_x)*(i-mean_x);
// 		var_xy+=(i-mean_x)*(gradient.at(i)-mean_y);
// 	}




// 	if(var_x!=0){
// 		double stg=var_xy/var_x;
// 		return stg;
// 	}

// 	return 0;
// }


}

#endif
