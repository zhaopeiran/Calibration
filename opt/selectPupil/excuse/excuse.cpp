// excuse.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <iostream>            
#include <fstream>
#include <time.h>


#include "algo.h"


void imshowscale(const std::string& name, cv::Mat& m, double scale)
{
    cv::Mat res;
    cv::resize(m, res, cv::Size(), scale, scale, cv::INTER_NEAREST);
    cv::imshow(name, res);
}

#define DEFSZ 18	//18
#define DEFST 0	//0

int _tmain(int argc, _TCHAR* argv[])
{

	




	int size_data_set=0;

	std::string pfad = "C:\\Users\\Wolfgang2\\Desktop\\data sets\\";

	std::string bezer[18];
	bezer[0]="data set I";
	bezer[1]="data set II";
	bezer[2]="data set III";
	bezer[3]="data set IV";
	bezer[4]="data set V";
	bezer[5]="data set VI";
	bezer[6]="data set VII";
	bezer[7]="data set VIII";
	bezer[8]="data set IX";
	bezer[9]="data set X";
	bezer[10]="data set XI";
	bezer[11]="data set XII";
	bezer[12]="data set XIII";
	bezer[13]="data set XIV";
	bezer[14]="data set XV";
	bezer[15]="data set XVI";
	bezer[16]="data set XVII";
	bezer[17]="swirski";

	int size_data_sets[18];
	size_data_sets[0]=6554;
	size_data_sets[1]=505;
	size_data_sets[2]=9799;
	size_data_sets[3]=2655;
	size_data_sets[4]=2135;
	size_data_sets[5]=4400;
	size_data_sets[6]=4890;
	size_data_sets[7]=630;
	size_data_sets[8]=2831;
	size_data_sets[9]=840;
	size_data_sets[10]=655;
	size_data_sets[11]=524;
	size_data_sets[12]=491;
	size_data_sets[13]=469;
	size_data_sets[14]=363;
	size_data_sets[15]=392;
	size_data_sets[16]=268;
	size_data_sets[17]=600;





	for(int kk=DEFST;kk<DEFSZ;kk++){

		size_data_set=size_data_sets[kk];

	clock_t zeit1 , zeit2, sum_all;
	sum_all=0;

	std::string str_number;
	char buffer[20];

	int corr5=0;

	int val1,val2,txt_x,txt_y;
	std::string line1, line2, line_x, line_y;

	std::fstream Quelldatei;                          // neuen Lese-Stream erzeugen
Quelldatei.open(pfad+bezer[kk]+".txt");   

//	std::fstream Zieldatei;                          // neuen Lese-Stream erzeugen
//Zieldatei.open(pfad+"erg_"+bezer[kk]+".txt",std::fstream::in | std::fstream::out | std::fstream::trunc); 


	//avoid first line
	Quelldatei>>line1>>line2>>line_x>>line_y;


	for(int i=0;i<size_data_set;i++){

	Quelldatei>>line1>>line2>>line_x>>line_y;

	val1=atoi(line1.c_str());
	val2=atoi(line2.c_str());
	txt_x=atoi(line_x.c_str())/2;
	txt_y=atoi(line_y.c_str())/2;


	//std::cout<<line1<<std::endl;
	//std::cout<<line2<<std::endl;
	//std::cout<<line_x<<std::endl;
	//std::cout<<line_y<<std::endl;
	
	
		

	str_number=sprintf (buffer, "%010d", val2);
	str_number=buffer;
//std::cout<<str_number<<std::endl;std::cout.flush();


std::string path_images1 = pfad+bezer[kk]+"\\"+str_number+".png";

	//cv::Mat m = cv::imread(path_images1, CV_LOAD_IMAGE_COLOR);
	cv::Mat m = cv::imread(path_images1, CV_LOAD_IMAGE_GRAYSCALE);


	if(!m.data) std::cout<<"error img read"<<std::endl;


	txt_y=m.rows-txt_y;

	cv::Mat pic_th = cv::Mat::zeros(m.rows, m.cols, CV_8U);
	cv::Mat th_edges = cv::Mat::zeros(m.rows, m.cols, CV_8U);


 
	zeit1 = clock();

	cv::RotatedRect pos=run(&m,&pic_th,&th_edges);
	//cv::RotatedRect pos=run_solo(&m,&pic_th,&th_edges);
	
	zeit2 = clock(); 

	sum_all+=zeit2-zeit1;




		double akt_dist=0.0;
		akt_dist=sqrt(pow(double(pos.center.x-txt_x),2)+pow(double(pos.center.y-txt_y),2));


		akt_dist=floor(akt_dist);
		if(akt_dist<=5) corr5++;
		//else std::cout<<str_number<<"     er:"<<akt_dist<<std::endl;
		//else Zieldatei<<str_number<<"     er:"<<akt_dist<<std::endl;




		//m.release();
		//pic_th.release();
		//th_edges.release();
	}
	Quelldatei.close();
//	Zieldatei.close();

	//int tt;
	//std::cin>>tt;
	std::cout<<"data set:"<<bezer[kk]<<std::endl;
	std::cout<<"time:"<<sum_all/size_data_set<<std::endl;
	std::cout<<"ergebniss:"<<double(double(corr5)/double(size_data_set))<<std::endl;
	std::cout<<"-----------------------------------------------------------"<<std::endl;
	}





	std::cout<<"ENDE"<<std::endl;
	int tt;
	std::cin>>tt;

    return 0;
}

