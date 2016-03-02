#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>


static bool peek(cv::Mat *pic, double *stddev, int start_x, int end_x, int start_y, int end_y, int peek_detector_factor, int bright_region_th){
	int gray_hist[256];
	int max_gray=0;
	int max_gray_pos=0;
	int mean_gray=0;
	int mean_gray_cnt=0;

	for(int i=0; i<256; i++)
		gray_hist[i]=0;


	double mean_feld[1000];
	double std_feld[1000];
	for(int i=start_x; i<end_x; i++){
		mean_feld[i]=0;
		std_feld[i]=0;
	}


	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){
			int idx=(int)pic->data[(pic->cols*j)+i];
			gray_hist[idx]++;
			mean_feld[i]+=idx;
		}

	for(int i=start_x; i<end_x; i++)
		mean_feld[i]= (mean_feld[i]/double(end_y-start_y));


	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){
			int idx=(int)pic->data[(pic->cols*j)+i];

				std_feld[i]+=(mean_feld[i]-idx)*(mean_feld[i]-idx);

		}

	for(int i=start_x; i<end_x; i++)
		std_feld[i]= sqrt(std_feld[i]/double(end_y-start_y));

	*stddev=0;
	for(int i=start_x; i<end_x; i++){
		*stddev+=std_feld[i];
	}

	*stddev=*stddev/((end_x-start_x));


		



	for(int i=0; i<256; i++)
		if(gray_hist[i]>0){
			
			mean_gray+=gray_hist[i];
			mean_gray_cnt++;

			if(max_gray<gray_hist[i]){
				max_gray=gray_hist[i];
				max_gray_pos=i;
			}
		}
		
	if(mean_gray_cnt<1) mean_gray_cnt=1;

	mean_gray=ceil((double)mean_gray/(double)mean_gray_cnt);


	if(max_gray>(mean_gray*peek_detector_factor) && max_gray_pos>bright_region_th)
		return true;
	else
		return false;
}