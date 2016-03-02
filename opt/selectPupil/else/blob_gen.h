#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>









static void gen_blob(int rad, cv::Mat *all_mat, cv::Mat *all_mat_neg){


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
					p[c0+j]=1;
					posis++;

				}else{ //neg

					int sz_w=sqrt( float(rad*rad) - float(i*i) );

					if(abs(j)<=sz_w){
						p[c0+j]=-1;
						negis++;
					}else{
						p[c0+j]=1;
						posis++;
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
				p_neg[j]=-1.0/posis;
			}else{
				p[j]=-1.0/negis;
				p_neg[j]=1.0/negis;
			}

		}
	}



/*
	for(int i=1;i<len-1;i++){
		p=all_mat->ptr<float>(i);
		p_neg=all_mat_neg->ptr<float>(i);
		for(int j=1;j<len-1;j++){
			if(p[j]>0 && (p[j+1]<0 || p[j-1]<0 || p[j+len]<0 || p[j-len]<0)){
				p_neg[j]=-1.0;
			}
			if(p[j]<0 && (p[j+1]>0 || p[j-1]>0 || p[j+len]>0 || p[j-len]>0)){
				p_neg[j]=1.0;
			}

		}
	}
*/	
		


	
	/*
	cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("plobiiii",*all_mat);
	cvWaitKey(1000);
	*/



}



static void gen_box(int rad, cv::Mat *all_mat, cv::Mat *all_mat_neg){


		int len=1+(6*rad);
		int c0=rad*3;
		float negis=0;
		float posis=0;

		*all_mat = cv::Mat::zeros(len, len, CV_32FC1);
		*all_mat_neg = cv::Mat::zeros(len, len, CV_32FC1);



		float *p, *p_neg;
		for(int i=-rad*3;i<=rad*3;i++){ //height
			p=all_mat->ptr<float>(c0+i);

			for(int j=-rad*3;j<=rad*3;j++){

				
				if(i<-rad*2 || i>rad*2 || j<-rad*2 || j>rad*2){ //pos
					p[c0+j]=1;
					posis++;

				}else if(!(i<-rad || i>rad || j<-rad || j>rad)){ //pos
					p[c0+j]=-1;
					negis++;
				}
				

			}
		}


	
	for(int i=0;i<len;i++){
		p=all_mat->ptr<float>(i);
		p_neg=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len;j++){
			if(p[j]>0){
				p[j]=1.0/posis;
				p_neg[j]=-1.0/posis;
			}
			if(p[j]<0){
				p[j]=-1.0/negis;
				p_neg[j]=1.0/posis;
			}

		}
	}
	

		


	
	/*
	cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("boxi",*all_mat);
	cvWaitKey(1000);
	*/



}






static void mum(cv::Mat *pic, cv::Mat *result, int fak){


	int fak_ges=fak+1;
	int sz_x=pic->cols/fak_ges;
	int sz_y=pic->rows/fak_ges;

	*result=cv::Mat::zeros(sz_y, sz_x, CV_8U);

	int hist[256];
	int mean=0;
	int cnt=0;
	int mean_2=0;

	int idx=0;
	int idy=0;

	for(int i=0;i<sz_y;i++){
		idy+=fak_ges;

		for(int j=0;j<sz_x;j++){
			idx+=fak_ges;

			for(int k=0;k<256;k++)
				hist[k]=0;


			mean=0;
			cnt=0;


			for(int ii=-fak;ii<=fak;ii++)
				for(int jj=-fak;jj<=fak;jj++){

					if(idy+ii>0 && idy+ii<pic->rows && idx+jj>0 && idx+jj<pic->cols){
						if((unsigned int)pic->data[(pic->cols*(idy+ii))+(idx+jj)]>255)
							pic->data[(pic->cols*(idy+ii))+(idx+jj)]=255;

						hist[pic->data[(pic->cols*(idy+ii))+(idx+jj)]]++;
						cnt++;
						mean+=pic->data[(pic->cols*(idy+ii))+(idx+jj)];
					}


				}


			mean=mean/cnt;

			mean_2=0;
			cnt=0;
			for(int ii=0;ii<=mean;ii++){
				mean_2+=ii*hist[ii];
				cnt+=hist[ii];
			}

			if(cnt==0)
				mean_2=mean;
			else
				mean_2=mean_2/cnt;

			result->data[(sz_x*(i))+(j)]=mean_2;
		}

		idx=0;
	}



		//imshow("img",*result);
		//cvWaitKey(500);

}









static cv::Point blob_finder_alt(cv::Mat *pic){

	cv::Point pos(0,0);
	float abs_max=0;;

	float *p_erg;
	cv::Mat blob_mat, blob_mat_neg;


	int fakk=3;//3
	//mum resize
	//cv::Mat img = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_8U);
	//cv::Mat erg = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_32FC1);
	//resize(*pic, img, img.size());

	cv::Mat img;
	mum(pic, &img, fakk);
	cv::Mat erg = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);






	cv::Mat result, result_neg;
	//4-8
for(int iii=4;iii<8;iii++){

	

	gen_blob(iii,&blob_mat,&blob_mat_neg);



	
	
	img.convertTo(img, CV_32FC1);
	filter2D(img, result, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

	
	float * p_res, *p_neg_res;
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
			if(p_res[j]<0)
				p_res[j]=0;
		}
	}

	filter2D(result, result_neg, -1 , blob_mat_neg, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

	for(int i=0; i<result.rows;i++){
		p_neg_res=result_neg.ptr<float>(i);
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
				p_erg[j]+=p_neg_res[j];

		}
	}



}

	float * p_img;
	for(int i=0; i<erg.rows;i++){
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<erg.cols;j++){
			if(abs_max<p_erg[j]){
				abs_max=p_erg[j];

				//pos.x=j*fakk;
				//pos.y=i*fakk;

				pos.x=(fakk+1)+(j*(fakk+1));
				pos.y=(fakk+1)+(i*(fakk+1));



			}
		}
	}









	

	/*

	img.convertTo(img, CV_8U);
	cv::normalize(img, img, 0, 255, cv::NORM_MINMAX, CV_8U);
	
	for(int i=-3; i<4;i++){
		for(int j=-3; j<4;j++){
			pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]=255;
		}
	}
std::cout<<pos.x<<":"<<pos.y<<std::endl;


	//result.convertTo(result, CV_8U);
	cv::normalize(result, result, 0, 255, cv::NORM_MINMAX, CV_8U);
	//result_neg.convertTo(result_neg, CV_8U);
	cv::normalize(result_neg, result_neg, 0, 255, cv::NORM_MINMAX, CV_8U);



	imshow("pos",result);
	imshow("neg",result_neg);
	imshow("img small",img);
	imshow("img",*pic);
	cvWaitKey(500);
	
	
	*/

	
	return pos;

}
































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
					p[c0+j]=1;
					posis++;

				}else{ //neg

					int sz_w=sqrt( float(rad*rad) - float(i*i) );

					if(abs(j)<=sz_w){
						p[c0+j]=-1;
						negis++;
					}else{
						p[c0+j]=1;
						posis++;
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
				p_neg[j]=0.0;
			}else{
				p[j]=-1.0/negis;
				p_neg[j]=1.0/negis;
			}

		}
	}



	/*
	//ring
	for(int i=1;i<len-1;i++){
		p=all_mat->ptr<float>(i);
		p_ring=all_ring->ptr<float>(i);

		for(int j=1;j<len-1;j++){
			if(p[j]>0 && (p[j+1]<0 || p[j-1]<0 || p[j+len]<0 || p[j-len]<0)){
				p_ring[j]=-1.0;
			}
			if(p[j]<0 && (p[j+1]>0 || p[j-1]>0 || p[j+len]>0 || p[j-len]>0)){
				p_ring[j]=1.0;
			}

		}
	}
	*/
		


	
	/*
	cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("plob",*all_mat);
	cv::normalize(*all_mat_neg, *all_mat_neg, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("average",*all_mat_neg);
	cv::normalize(*all_ring, *all_ring, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("neg blob",*all_ring);
	cvWaitKey(1000);
	*/
	


}










static void gen_blob_in_blob(int rad, cv::Mat *all_mat, cv::Mat *all_mat_neg){


		int len0=1+(4*rad);
		int len1=1+(8*rad);
		int c0=rad*2;
		int c1=rad*4;
		float negis0=0;
		float posis0=0;
		float negis1=0;
		float posis1=0;

		*all_mat = cv::Mat::zeros(len0, len0, CV_32FC1);
		*all_mat_neg = cv::Mat::zeros(len1, len1, CV_32FC1);


		float *p;
		for(int i=-rad*2;i<=rad*2;i++){ //height
			p=all_mat->ptr<float>(c0+i);

			for(int j=-rad*2;j<=rad*2;j++){

				if(i<-rad || i>rad){ //pos
					p[c0+j]=1;
					posis0++;

				}else{ //neg

					int sz_w=sqrt( float(rad*rad) - float(i*i) );

					if(abs(j)<=sz_w){
						p[c0+j]=-1;
						negis0++;
					}else{
						p[c0+j]=1;
						posis0++;
					}

				}

			}
		}


		for(int i=-rad*4;i<=rad*4;i++){ //height
			p=all_mat_neg->ptr<float>(c1+i);

			for(int j=-rad*4;j<=rad*4;j++){


				
				

					int sz_w=sqrt( float((rad*4)*(rad*4)) - float(i*i) );
					int sz_w1=sqrt( float((rad*3)*(rad*3)) - float(i*i) );
					int sz_w2=sqrt( float((rad*2)*(rad*2)) - float(i*i) );
					int sz_w_small=sqrt( float((rad)*(rad)) - float(i*i) );

					if(abs(j)<=sz_w_small){
						p[c1+j]=-1;
						negis1++;
					}else if(abs(j)<=sz_w2){
						p[c1+j]=0;
					}else if(abs(j)<=sz_w){
						p[c1+j]=1;
						posis1++;
					}else{
						p[c1+j]=0;
					}

				
				

			}
		}





	
	for(int i=0;i<len0;i++){
		p=all_mat->ptr<float>(i);

		for(int j=0;j<len0;j++){

			if(p[j]>0){
				p[j]=1.0/posis0;
			}else{
				p[j]=-1.0/negis0;
			}

		}
	}


	
	for(int i=0;i<len1;i++){
		p=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len1;j++){

			if(p[j]>0)
				p[j]=1.0/posis1;
			if(p[j]<0)
				p[j]=-1.0/negis1;
			

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









static bool is_good_ellipse_evaluation(cv::RotatedRect *ellipse, cv::Mat *pic){



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

	if(val>10) return true;
	else return false;
}









static cv::RotatedRect blob_finder(cv::Mat *pic){

	cv::Point pos(0,0);
	float abs_max=0;

	float *p_erg;
	cv::Mat blob_mat, blob_mat_neg;


	int fak_mum=5;
	int fakk=pic->cols>pic->rows?(pic->cols/100)+1:(pic->rows/100)+1;
	//mum resize
	//cv::Mat img = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_8U);
	//cv::Mat erg = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_32FC1);
	//resize(*pic, img, img.size());

	cv::Mat img;
	mum(pic, &img, fak_mum);
	cv::Mat erg = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);




	cv::Mat result, result_neg;


	gen_blob_neu(fakk,&blob_mat,&blob_mat_neg);
	//gen_blob(fakk,&blob_mat,&blob_mat_neg);
	//gen_blob_in_blob(fakk,&blob_mat,&blob_mat_neg);
	
	img.convertTo(img, CV_32FC1);
	filter2D(img, result, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );

	
	float * p_res, *p_neg_res;
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
			if(p_res[j]<0)
				p_res[j]=0;
		}
	}

	filter2D(img, result_neg, -1 , blob_mat_neg, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );
	

	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);
		p_neg_res=result_neg.ptr<float>(i);
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
				p_neg_res[j]=(255.0f-p_neg_res[j]);
				p_erg[j]=(p_neg_res[j])*(p_res[j]);
				//p_erg[j]=(p_res[j]);
		}
	}







	float * p_img;
	for(int i=0; i<erg.rows;i++){
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<erg.cols;j++){
			if(abs_max<p_erg[j]){
				abs_max=p_erg[j];


				pos.x=(fak_mum+1)+(j*(fak_mum+1));
				pos.y=(fak_mum+1)+(i*(fak_mum+1));

			}
		}
	}


if(pos.y>0 && pos.y<pic->rows && pos.x>0 && pos.x<pic->cols){
	
	//calc th
	int opti_x=0;
	int opti_y=0;

	float mm=0;
	float cnt=0;
	for(int i=-(2); i<(2);i++){
		for(int j=-(2); j<(2);j++){
			if( pos.y+i>0 && pos.y+i<pic->rows && pos.x+j>0 && pos.x+j<pic->cols){
				mm+=pic->data[(pic->cols*(pos.y+i))+(pos.x+j)];
				cnt++;
			}

		}
	}

	if(cnt>0)
		mm=ceil(mm/cnt);


	int th_bot=0;
	if(pos.y>0 && pos.y<pic->rows && pos.x>0 && pos.x<pic->cols)
		th_bot= pic->data[(pic->cols*(pos.y))+(pos.x)] + abs(mm-pic->data[(pic->cols*(pos.y))+(pos.x)]);
	//int th_top= 255-abs(mm-pic->data[(pic->cols*(pos.y))+(pos.x)]);
	cnt=0;

	for(int i=-(fak_mum*fak_mum); i<(fak_mum*fak_mum);i++){
		for(int j=-(fak_mum*fak_mum); j<(fak_mum*fak_mum);j++){

			if( pos.y+i>0 && pos.y+i<pic->rows && pos.x+j>0 && pos.x+j<pic->cols){

				if(pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]<=th_bot){
					//pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]=100;
					opti_x+=pos.x+j;
					opti_y+=pos.y+i;
					cnt++;
				}
			}

		}
	}

//imshow("img",*pic);
//cvWaitKey(10000);

	if(cnt>0){
		opti_x=opti_x/cnt;
		opti_y=opti_y/cnt;
	}else{
		opti_x=pos.x;
		opti_y=pos.y;
	}

	pos.x=opti_x;
	pos.y=opti_y;



}



	/*
	//opti in big image
	int opti_x=pos.x;
	int opti_y=pos.y;
	int opti_val=1000;

	for(int i=-(fakk); i<(fakk);i++){
		for(int j=-(fakk); j<(fakk);j++){

			int val=0;
			int cnt=0;

			for(int k1=-(fakk*fakk*fakk); k1<(fakk*fakk);k1++){
				for(int k2=-(fakk*fakk*fakk); k2<(fakk*fakk);k2++){

					if( pos.y+i+k1>0 && pos.y+i+k1<pic->rows && pos.x+j+k2>0 && pos.x+j+k2<pic->cols){
						val+=pic->data[(pic->cols*(pos.y+i+k1))+(pos.x+j+k2)];
						cnt++;
					}


				}
			}

			if(cnt>0)
				val=val/cnt;
			else
				val=10000;


			if(val<opti_val){
				opti_val=val;
				opti_x=pos.x+j;
				opti_y=pos.y+i;
			}



		}
	}
	
	
	pos.x=opti_x;
	pos.y=opti_y;
	*/



	
	
	/*
	img.convertTo(img, CV_8U);
	cv::normalize(img, img, 0, 255, cv::NORM_MINMAX, CV_8U);
	
	for(int i=-3; i<4;i++){
		for(int j=-3; j<4;j++){
			pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]=255;

			pic->data[(pic->cols*(opti_y+i))+(opti_x+j)]=150;
		}
	}
std::cout<<pos.x<<":"<<pos.y<<std::endl;


	//result.convertTo(result, CV_8U);
	cv::normalize(result, result, 0, 255, cv::NORM_MINMAX, CV_8U);
	//result_neg.convertTo(result_neg, CV_8U);
	cv::normalize(result_neg, result_neg, 0, 255, cv::NORM_MINMAX, CV_8U);

	cv::normalize(erg, erg, 0, 255, cv::NORM_MINMAX, CV_8U);

	
	imshow("erg",erg);
	imshow("pos",result);
	imshow("neg",result_neg);
	imshow("img small",img);
	imshow("img",*pic);
	cvWaitKey(500);
	*/
	
	

	cv::RotatedRect ellipse;

	if( pos.y>0 && pos.y<pic->rows && pos.x>0 && pos.x<pic->cols){
		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=(fak_mum*fak_mum*2) +1;
		ellipse.size.width=(fak_mum*fak_mum*2) +1;

		if(!is_good_ellipse_evaluation(&ellipse, pic)){
			ellipse.center.x=0;
			ellipse.center.y=0;
			ellipse.angle=0;
			ellipse.size.height=0;
			ellipse.size.width=0;
		}

	}else{
		ellipse.center.x=0;
		ellipse.center.y=0;
		ellipse.angle=0;
		ellipse.size.height=0;
		ellipse.size.width=0;

	}



	
	return ellipse;

}
















static void gen_blob_isef(int rad, cv::Mat *all_mat){


		int len=1+(4*rad);
		int c0=rad*2;
		int posis=0;
		int negis=0;

		*all_mat = cv::Mat::zeros(1, len, CV_32FC1);


		float *p, *p_neg;
		p=all_mat->ptr<float>(0);
		for(int i=-rad*2;i<=rad*2;i++){ //height
			float b=0.5;
			float val=(-(log(b)/2) * pow(b,abs(i+rad)) ) - (-(log(b)/2) * pow(b,abs(i-rad)) );
			p[c0+i]=val;
		}


		
		for(int i=1; i<len-1;i++){

			if(p[i]>p[i+1])
				p[i]=-abs(p[i+1]-p[i]);
			else if(p[i]<p[i+1])
				p[i]=abs(p[i+1]-p[i]);
			else
				p[i]=0;


		}
		p[0]=0;
		p[len-1]=0;
		



		/*
	for(int i=0;i<len;i++){
		p=all_mat->ptr<float>(i);
		p_neg=all_mat_neg->ptr<float>(i);

		for(int j=0;j<len;j++){

			if(p[j]>0){
				p[j]=p[j]/posis;
				p_neg[j]=0.0;
			}else{
				p[j]=p[j]/negis;
				p_neg[j]=1.0/negis;
			}

		}
	}
	*/

		


	
	/*
	cv::normalize(*all_mat, *all_mat, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("plob",*all_mat);
	cv::normalize(*all_mat_neg, *all_mat_neg, 0, 255, cv::NORM_MINMAX, CV_8U);
	imshow("average",*all_mat_neg);
	cvWaitKey(10000);
	*/
	


}













static cv::RotatedRect blob_finder_isef(cv::Mat *pic){

	cv::Point pos(0,0);
	float abs_max=0;;

	cv::Mat blob_mat;


	int fak_mum=5;
	int fakk=pic->cols>pic->rows?(pic->cols/100)+1:(pic->rows/100)+1;
	//mum resize
	//cv::Mat img = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_8U);
	//cv::Mat erg = cv::Mat::zeros(pic->rows/fakk, pic->cols/fakk, CV_32FC1);
	//resize(*pic, img, img.size());

	cv::Mat img;
	mum(pic, &img, fak_mum);
	cv::Mat erg = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);



	cv::Mat result, result_neg;


	gen_blob_isef(fakk,&blob_mat);
	//gen_blob(fakk,&blob_mat,&blob_mat_neg);
	//gen_blob_in_blob(fakk,&blob_mat,&blob_mat_neg);
	
	img.convertTo(img, CV_32FC1);
	filter2D(img, result, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );
	cv::transpose(img,img);
	filter2D(img, result_neg, -1 , blob_mat, cv::Point( -1, -1 ), 0, cv::BORDER_REPLICATE );
	cv::transpose(img,img);
	cv::transpose(result_neg,result_neg);

	float *p_res,*p_neg_res,*p_erg;
	for(int i=0; i<result.rows;i++){
		p_res=result.ptr<float>(i);
		p_neg_res=result_neg.ptr<float>(i);
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<result.cols;j++){
				p_erg[j]=(p_neg_res[j])*(p_res[j]);
				//p_erg[j]=(p_res[j]);
		}
	}







	float * p_img;
	for(int i=0; i<erg.rows;i++){
		p_erg=erg.ptr<float>(i);

		for(int j=0; j<erg.cols;j++){
			if(abs_max<p_erg[j]){
				abs_max=p_erg[j];


				pos.x=(fak_mum+1)+(j*(fak_mum+1));
				pos.y=(fak_mum+1)+(i*(fak_mum+1));

			}
		}
	}



	
	//calc th
	int opti_x=0;
	int opti_y=0;

	float mm=0;
	float cnt=0;
	for(int i=-(2); i<(2);i++){
		for(int j=-(2); j<(2);j++){
			if( pos.y+i>0 && pos.y+i<pic->rows && pos.x+j>0 && pos.x+j<pic->cols){
				mm+=pic->data[(pic->cols*(pos.y+i))+(pos.x+j)];
				cnt++;
			}

		}
	}

	if(cnt>0)
		mm=ceil(mm/cnt);


	int th_bot=0;
	if(pos.y>0 && pos.y<pic->rows && pos.x>0 && pos.x<pic->cols)
		th_bot= pic->data[(pic->cols*(pos.y))+(pos.x)] + abs(mm-pic->data[(pic->cols*(pos.y))+(pos.x)]);
	//int th_top= 255-abs(mm-pic->data[(pic->cols*(pos.y))+(pos.x)]);
	cnt=0;

	for(int i=-(fak_mum*fak_mum); i<(fak_mum*fak_mum);i++){
		for(int j=-(fak_mum*fak_mum); j<(fak_mum*fak_mum);j++){

			if( pos.y+i>0 && pos.y+i<pic->rows && pos.x+j>0 && pos.x+j<pic->cols){

				if(pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]<=th_bot){
					pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]=100;
					opti_x+=pos.x+j;
					opti_y+=pos.y+i;
					cnt++;
				}
			}

		}
	}

	if(cnt>0){
		opti_x=opti_x/cnt;
		opti_y=opti_y/cnt;
	}else{
		opti_x=pos.x;
		opti_y=pos.y;
	}

	pos.x=opti_x;
	pos.y=opti_y;


	/*
	//opti in big image
	int opti_x=pos.x;
	int opti_y=pos.y;
	int opti_val=1000;

	for(int i=-(fakk); i<(fakk);i++){
		for(int j=-(fakk); j<(fakk);j++){

			int val=0;
			int cnt=0;

			for(int k1=-(fakk*fakk*fakk); k1<(fakk*fakk);k1++){
				for(int k2=-(fakk*fakk*fakk); k2<(fakk*fakk);k2++){

					if( pos.y+i+k1>0 && pos.y+i+k1<pic->rows && pos.x+j+k2>0 && pos.x+j+k2<pic->cols){
						val+=pic->data[(pic->cols*(pos.y+i+k1))+(pos.x+j+k2)];
						cnt++;
					}


				}
			}

			if(cnt>0)
				val=val/cnt;
			else
				val=10000;


			if(val<opti_val){
				opti_val=val;
				opti_x=pos.x+j;
				opti_y=pos.y+i;
			}



		}
	}
	
	
	pos.x=opti_x;
	pos.y=opti_y;
	*/



	
	
	/*
	img.convertTo(img, CV_8U);
	cv::normalize(img, img, 0, 255, cv::NORM_MINMAX, CV_8U);
	
	for(int i=-3; i<4;i++){
		for(int j=-3; j<4;j++){
			pic->data[(pic->cols*(pos.y+i))+(pos.x+j)]=255;

			pic->data[(pic->cols*(opti_y+i))+(opti_x+j)]=150;
		}
	}
std::cout<<pos.x<<":"<<pos.y<<std::endl;


	//result.convertTo(result, CV_8U);
	cv::normalize(result, result, 0, 255, cv::NORM_MINMAX, CV_8U);
	//result_neg.convertTo(result_neg, CV_8U);
	cv::normalize(result_neg, result_neg, 0, 255, cv::NORM_MINMAX, CV_8U);

	cv::normalize(erg, erg, 0, 255, cv::NORM_MINMAX, CV_8U);

	
	imshow("erg",erg);
	imshow("pos",result);
	imshow("neg",result_neg);
	imshow("img small",img);
	imshow("img",*pic);
	cvWaitKey(500);
	*/
	
	

	cv::RotatedRect ellipse;

	if( pos.y>0 && pos.y<pic->rows && pos.x>0 && pos.x<pic->cols){
		ellipse.center.x=pos.x;
		ellipse.center.y=pos.y;
		ellipse.angle=0.0;
		ellipse.size.height=fak_mum*fakk;
		ellipse.size.width=fak_mum*fakk;

	}else{
		ellipse.center.x=0;
		ellipse.center.y=0;
		ellipse.angle=0;
		ellipse.size.height=0;
		ellipse.size.width=0;

	}



	
	return ellipse;

}






