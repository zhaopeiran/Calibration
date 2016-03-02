#include <opencv2/highgui/highgui.hpp>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include "detection/Pupil_contours_detection.hpp"
#include "detection/PUPIL_BLOB_DETECTION.hpp"
#include "detection/algo.hpp"
#include "findEyeCenter.hpp"

using namespace cv;
using namespace std;

uint green[3] = {0,255,0};
uint blue[3]  = {0,0,255};

struct Pupilpos{
  uint frame;
  int x;
  int y;
};

unsigned int counter;
uint click;
Mat im;
string name;
vector <Pupilpos> out_pos;
int num_images_per_pupil = -1;
unsigned int PUPIL_METHOD = 3         ;

void savePos( int x, int y ){
  char text[100];
  sprintf(text, "%d;%d;%d", counter, x, y);
  cout   << text << endl;
  Pupilpos pupilpos;
  pupilpos.x = x; pupilpos.y = y; pupilpos.frame = counter;
  out_pos.push_back(pupilpos);
  click = 1;
}

void return_previous(){
  if( counter > 0){
    out_pos.pop_back();
    counter -= 2; //at the beginning of the loop the counter will increase 1
    click = 1;
  }
}

void displayPos( int x, int y, uint color[3] ){
  char displaytext[100];
  Mat im2;
  im2 = im.clone();

  sprintf(displaytext, "Frame=%d  X=%d  Y=%d", counter, x, y);
  putText(im2, displaytext, Point(5,15), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(color[0], color[1], color[2]));
  imshow(name, im2);
}

static void onMouse( int event, int x, int y, int, void* ){

  if( event == EVENT_LBUTTONDOWN ){ savePos(x,y); }
  else if ( event == EVENT_MOUSEMOVE ){ displayPos(x, y, green); }
}

int main(int argc, char** argv){
  DIR *dir;
  struct dirent *ent;
  vector<string> files;
  bool exit = false;
  cv::Point pupil_center;
  int threshVal = 20;

  if( argc != 4 ){
    cout << "usage: " << argv[0] << " <file_dir> <out_file_name> <number_images_per_pupil>" << endl;
    return 1;
  }else{
    string tmpdir(argv[1]);
    if ((dir = opendir (argv[1])) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
        // printf ("%s\n", ent->d_name);
        string tmpname(ent->d_name);
        if( tmpname.compare(".") != 0 && tmpname.compare("..") != 0 ) files.push_back(tmpname);
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
      return 1;
    }

    // order alphabetically
    std::sort (files.begin(), files.end());

    //open output file
    ofstream output(argv[2]);
    if( ! output.is_open()  ){
      cout << "Error, cannot create output file" << endl;
      return 1;
    }

    for(counter=0; counter<files.size() && !exit; counter++){
      name = files[counter];
      im = imread(tmpdir + "/" + name);
      if (im.empty()){
        cout << "Cannot load image: " << name << endl;
      }else{
        click = 0;
        namedWindow( name, CV_WINDOW_AUTOSIZE );
        setMouseCallback( name, onMouse, 0 );
        // im = im + Scalar(35, 35, 35); // make the image brighter
        Mat im2;
        im2 = im.clone();
        vector<cv::RotatedRect> ellipses;        

        switch(PUPIL_METHOD){
        case 2:
          cv::cvtColor(im, im2, CV_RGB2GRAY);
          bitwise_not ( im, im2 );
          ellipses = PUPIL_BLOB_DETECTION::blob_finder(&im2);
          if( ellipses.size() >= 1 ) pupil_center = ellipses[0].center;
          break;
        case 3:
          extractEyeFeatures(im2, pupil_center);
          pupil_center = find_eye_center(im);
          break;
        case 1:
        default:
          Pupil_contours_detection::extractEyeFeatures(im2, pupil_center, threshVal);
          break;
        }

        circle(im, pupil_center, 4, CV_RGB(255,0,0), 2);
        displayPos(pupil_center.x, pupil_center.y, blue);
        while(!click && !exit){
          int c = waitKey(1);
          switch( (c & 255) ){
          case 27: //ESC
            cout << "Exiting ...\n";
            exit = true;
            break;
          case 32: //space
            savePos(pupil_center.x,pupil_center.y);
            break;
          case 8: //backspace
            return_previous();
            break;
          }
        }
        destroyWindow(name);
      }
    }

    //savePos all to the output file
    num_images_per_pupil = atoi(argv[3]);
    cout << "Saving ...\n";
    output << "my @pupilpositions_L = (";
    for(size_t idx=0; idx<out_pos.size(); idx++){
      if( num_images_per_pupil == idx ){ 
        cout << "change" << endl;
        output << ");" << endl << "my @pupilpositions_R = ("; 
      }
      char text[100];
      sprintf(text, "\"%3d;%3d\", ", out_pos[idx].x, out_pos[idx].y);
      if( idx%10 == 0 ){ output << endl; }
      output << text;
    }
    output << ");" << endl;
    output.close();
  }
}
