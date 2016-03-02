
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>



static void rays(cv::Mat *th_edges, int end_x, int end_y, cv::Point *pos, int *ret){

	for(int i=0; i<8; i++) ret[i]=-1;


	for(int i=0; i<end_x; i++)
		for(int j=0; j<end_y; j++){
			
			if(pos->x-i>0 && pos->x+i<th_edges->cols && pos->y-j>0 && pos->y+j<th_edges->rows){


			if((int)th_edges->data[(th_edges->cols*(pos->y))+(pos->x+i)] !=0 && ret[0]==-1){
				ret[0]=th_edges->data[(th_edges->cols*(pos->y))+(pos->x+i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y))+(pos->x-i)] !=0 && ret[1]==-1){
				ret[1]=th_edges->data[(th_edges->cols*(pos->y))+(pos->x-i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x)] !=0 && ret[2]==-1){
				ret[2]=th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x<<" y:"<<pos->y+j<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x)] !=0 && ret[3]==-1){
				ret[3]=th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x<<" y:"<<pos->y-j<<std::endl;
			}


			if((int)th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x+i)] !=0 && ret[4]==-1 && i==j){
				ret[4]=th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x+i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y+j<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x-i)] !=0 && ret[5]==-1 && i==j){
				ret[5]=th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x-i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y-j<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x+i)] !=0 && ret[6]==-1 && i==j){
				ret[6]=th_edges->data[(th_edges->cols*(pos->y-j))+(pos->x+i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x+i<<" y:"<<pos->y-j<<std::endl;
			}
			if((int)th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x-i)] !=0 && ret[7]==-1 && i==j){
				ret[7]=th_edges->data[(th_edges->cols*(pos->y+j))+(pos->x-i)]-1;
				//std::cout<<"val:"<<ret[0]<<" x:"<<pos->x-i<<" y:"<<pos->y+j<<std::endl;
			}


			}

		}


}




static void zero_around_region_th_border(cv::Mat *pic, cv::Mat *edges, cv::Mat *th_edges, int th, int edge_to_th, double mean_dist, double area, cv::RotatedRect *pos){

	int ret[8];
	std::vector<cv::Point> selected_points;
	cv::RotatedRect ellipse;

	int start_x=pos->center.x - (area*pic->cols);
	int end_x=pos->center.x + (area*pic->cols);
	int start_y=pos->center.y - (area*pic->rows);
	int end_y=pos->center.y + (area*pic->rows);


	if(start_x<0) start_x=edge_to_th;
	if(start_y<0) start_y=edge_to_th;
	if(end_x>pic->cols) end_x=pic->cols-(edge_to_th+1);
	if(end_y>pic->rows) end_y=pic->rows-(edge_to_th+1);
	
	th=th+th+1;


	//std::cout<<"sx:"<<start_x<<" sy:"<<start_y<<" ex:"<<end_x<<" ey:"<<end_y<<" dist:"<<edge_to_th<<std::endl;

	for(int i=start_x; i<end_x; i++)
		for(int j=start_y; j<end_y; j++){

			if(pic->data[(pic->cols*j)+(i)]<th){

				for(int k1=-edge_to_th; k1<edge_to_th; k1++)
					for(int k2=-edge_to_th; k2<edge_to_th; k2++){

						if(i+k1>=0 && i+k1<pic->cols && j+k2>0 && j+k2<edges->rows)
						if((int)edges->data[(edges->cols*(j+k2))+(i+k1)])
							th_edges->data[(edges->cols*(j+k2))+(i+k1)]=255;

				}

			}

		}



	//remove_points_with_low_angle(th_edges, start_x, end_x, start_y, end_y);

	std::vector<std::vector<cv::Point>> all_curves=get_curves(pic, th_edges, start_x, end_x, start_y, end_y, mean_dist, 0);


//std::cout<<"all curves:"<<all_curves.size()<<std::endl;
	if(all_curves.size()>0){


	//zero th_edges
		/*
	for(int i=start_x-edge_to_th; i<end_x+edge_to_th; i++)
		for(int j=start_y-edge_to_th; j<end_y+edge_to_th; j++){
			th_edges->data[(th_edges->cols*(j))+(i)]=0;
		}
		*/
		
	for(int i=0; i<th_edges->cols; i++)
		for(int j=0; j<th_edges->rows; j++){
			th_edges->data[(th_edges->cols*(j))+(i)]=0;
		}


	//draw remaining edges
	for(unsigned int i=0; i<all_curves.size(); i++){
		//std::cout<<"written:"<<i+1<<std::endl;
		for(unsigned int j=0; j<all_curves[i].size(); j++){

			if(all_curves[i][j].x>=0 && all_curves[i][j].x<th_edges->cols && all_curves[i][j].y>=0 && all_curves[i][j].y<th_edges->rows)
			th_edges->data[(th_edges->cols*(all_curves[i][j].y))+(all_curves[i][j].x)]=i+1;//+1 becouse of first is 0
		}
	}



 
	
	cv::Point st_pos;
	st_pos.x=pos->center.x;
	st_pos.y=pos->center.y;
	//send rays add edges to vector
	rays(th_edges, (end_x-start_x)/2, (end_y-start_y)/2, &st_pos, ret);

	//for(int i=0; i<8; i++) std::cout<<"ret:"<<ret[i]<<std::endl;
	//cv::imshow("akt", *th_edges);
	//cv::waitKey(0);

	//gather points
	for(int i=0; i<8; i++)
		if(ret[i]>-1 && static_cast<unsigned int>(ret[i])<all_curves.size()){
			//std::cout<<"size:"<<all_curves.size()<<std::endl;
			//std::cout<<"idx:"<<ret[i]<<std::endl;
			for(unsigned int j=0; j<all_curves[ret[i]].size(); j++){
				selected_points.push_back(all_curves[ret[i]][j]);
			}
		}
	//ellipse fit if size>5



	if(selected_points.size()>5){

		*pos=cv::fitEllipse( cv::Mat(selected_points) );

		/*
		cv::ellipse(*pic, cv::RotatedRect(ellipse.operator CvBox2D()),CV_RGB(255,255,255));
		cv::imshow("akt", *pic);
		*/
	}

	}

	/*
	std::cout<<pos->x<<";"<<pos->y<<std::endl;
	cv::ellipse(*pic, cv::RotatedRect(*pos, cv::Size2f(5,5),0), CV_RGB(255,255,255));
	imshow("opt",*pic);
	*/
}
