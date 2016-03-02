#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include "LogFileWriter.hpp"
#include "utils.hpp"
#include "detection/algo.hpp"
#include "detection/PUPIL_BLOB_DETECTION.hpp"
#include "detection/Pupil_contours_detection.hpp"
#include "calibration/Calibration.hpp"
#include "calibration/Interpolation.hpp"
#include "calibration/Geomodel.hpp"

using namespace std;

#define ALGO_INTER "inter"
#define ALGO_GEO "geo"

#define PUPIL_CANNY "canny"
#define PUPIL_BLOB "blob"
#define PUPIL_CONT "contour"

const string LOG_FILE="Gazetracker.log";
const string OUT_FILE="Gazetracker.csv";
const string STD_FILE="Stadistics.csv";

LogFileWriter logfile;


static void show_usage( string name ) {
  cerr << "Usage: " << name << " <options> " << endl
       << "Options:" << endl
       << "\t-h,--help\t\tShow this help message" << endl
       << "\t-d,--debug\t\tDebug mode" << endl
       << "\t-a,--algorithm ALGO\tSpecify the algorithm type. Available algorithms are:" << endl
       << "\t\t- "  << ALGO_INTER << endl
       << "\t\t- "  << ALGO_GEO   << endl
       << "\t-p,--pupil-algorithm PALGO\tSpecify the algorithm type for pupil detection. Available algorithms are:" << endl
       << "\t\t- "  << PUPIL_CONT  << " (by default)" << endl
       << "\t\t- "  << PUPIL_CANNY << endl
       << "\t\t- "  << PUPIL_BLOB  << endl
       << "\t-f,--files INPUT_FILES\tSpecify the video files to load separated by \";\"" << endl 
       << "\t-i,--interdistance DISTANCE\tInter-pupilar distance of the subject" << endl 
       << "\t\t- The first file is for right eye and second is for left eye " << endl
       << "\t\t- Ex: \"right_eye.avi;left_eye.avi\" " << endl
       << "\t-o,--output OUTPUT\tSpecify the output folder path" << endl
       << "\t-ct,--calibration_target\tCALIB_FILE\tCalibration File with target positions " << endl
       << "\t\t- File format is: time, pos_x, pos_y, pos_z" << endl
       << "\t-cv,--calibration_video VIDEOS\tVideos for calibration algorithm (synchronized with calibration targets) to load separated by \";\"" << endl
       << "\t\t- The first file is for right eye and second is for left eye " << endl
       << "\t\t- Ex: \"right_calibration_eye.avi;left_calibration_eye.avi\" " << endl
       << "\t-et,--error_target\tERROR_FILE\tFile with position of the targets in input videos for error calculation " << endl
       << "\t-fp,--filter_pupil\t\tActivate filter pupil position  " << endl
       << "\t\t- File format is: time, pos_x, pos_y, pos_z" << endl
       << "You can also specify (optional):" << endl
       << "\t-x,--prefix PREFIX\tSpecify the output files prefix" << endl
       << endl
       << "\t-cp,--calibration_pupil TEXT_FILES\tThe text files to load separated by \";\"" << endl
       << "\t\t- The first file is for right eye and second is for left eye " << endl
       << "\t\t- Ex: \"right_calib_pupil.txt;left_calib_pupil.txt\" " << endl
       << "\t\t- Each file format is: Frame, Pupil_X, Pupil_Y" << endl
       << "\t-ep,--estimation_pupil TEXT_FILES\tThe text files to load separated by \";\"" << endl
       << "\t\t- The first file is for right eye and second is for left eye " << endl
       << "\t\t- Ex: \"right_estim_pupil.txt;left_estim_pupil.txt\" " << endl
       << "\t\t- Each file format is: Frame, Pupil_X, Pupil_Y" << endl;

}


static int processStringOption(int argc, int i, char* argv[], string &option, string optName){
  if (i + 1 < argc) {
    option = argv[++i];
    logfile.writeDebug(optName + ": " + option );
  } else {
    cerr << optName + "option requires one argument" << endl;
    logfile.writeError(optName + "option requires one argument");
    return 1;
  }
  return 0;
}


static void extractEyeFeatures(string algo, cv::Mat &frame, cv::Point &pupil_center, std::vector<cv::Point> &glints){
  if( algo == PUPIL_CANNY) extractEyeFeatures(frame, pupil_center, &glints);
  else if( algo == PUPIL_CONT){
    int threshVal = 20;
    Pupil_contours_detection::extractEyeFeatures(frame, pupil_center, threshVal, &glints);
  }else{
    cv::Mat frame2;
    cv::cvtColor(frame, frame2, CV_RGB2GRAY);
    vector<cv::RotatedRect> ellipses = PUPIL_BLOB_DETECTION::blob_finder(&frame2);
    if( ellipses.size() >= 1 ) pupil_center = ellipses[0].center;
  }
}

static void extractEyeFeatures(string algo, cv::Mat &frame, cv::Point &pupil_center){
  if( algo == PUPIL_CANNY) extractEyeFeatures(frame, pupil_center);
  else if( algo == PUPIL_CONT){
    int threshVal = 20;
    Pupil_contours_detection::extractEyeFeatures(frame, pupil_center, threshVal);
  }else{
    cv::Mat frame2;
    cv::cvtColor(frame, frame2, CV_RGB2GRAY);
    vector<cv::RotatedRect> ellipses = PUPIL_BLOB_DETECTION::blob_finder(&frame2);
    if( ellipses.size() >= 1 ) pupil_center = ellipses[0].center;
  }
}

static bool filterPupil(vector<eye_feature_t> *eye_features){
  // create at window of 20 positions, get the median value (if not 0) and remove what is more that
  const static size_t AVE_POINTS = 20;
  const static size_t THR_DIST = 30;

  if( eye_features->size() > AVE_POINTS ){
    vector<cv::Point2d> points(AVE_POINTS);
    cv::Point2d average, prev_average;
    for( size_t idx=0; idx<20; idx++ ){ points[idx] = ((*eye_features)[idx]).pupil_center; }
    prev_average = geometry::calcAverage(points);
    if ( prev_average == cv::Point2d(0,0) ){ logfile.writeWarn("Not using filterPupil. initial average cannot be 0"); return false; }

    unsigned int position_in_average = 0;
    for( size_t idx=0; idx< eye_features->size(); idx++ ){
      //calculate new average
      average = geometry::calcAverage(points);
      //write the average point in the pupil position
      if ( geometry::cartesian2dDiff(average, ((*eye_features)[idx]).pupil_center) > THR_DIST ) ((*eye_features)[idx]).pupil_center = average;

      //add new value to the average vector replacing and old one 
      points[position_in_average] = ((*eye_features)[idx]).pupil_center;
      position_in_average = (position_in_average+1)%AVE_POINTS;
      prev_average = average;

    }
    return true;
  }

  logfile.writeWarn("Not using filterPupil. Not enough points: " + utils::uint2str(eye_features->size()) ); return false;
  return false;
}


int main(int argc, char* argv[]){
  //set bool
  bool debug = false;
  bool filter_pupil = false;
  //set Strings
  string algoName    = "";
  string pupilName   = "";
  string calibTarget = "";
  string errorTarget = "";
  string outputPath  = "";
  string outPrefix   = "";
  string interpupil  = "";
  //set vectors
  vector<string> fileList;
  vector<string> calibList;
  vector<string> calibPupilList;
  vector<string> estimPupilList;
  vector<cv::Point3d> errorList;

  logfile.createlogfile( LOG_FILE );

  //write log file info
  logfile.write("CompanyName: University of Tuebingen ");
  logfile.write("FileDescription: Gaze Tracker");
  logfile.write("ProductVersion: " + GAZETRACKER_VERSION);
  logfile.write("InternalName: Gazetracker commandline");
  logfile.write("LegalCopyright: Copyright (C) 2015");
  logfile.write("OriginalFilename: " + string(argv[0]));
  logfile.write("ProductName: Gaze Tracker test program");
  logfile.write("Starting application...");

  setlocale(LC_NUMERIC, "C");

  // get the arguments
  if (argc < 2) {
    show_usage(argv[0]);
    logfile.write("Application finish with errors");
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
      show_usage(argv[0]);
      return 0;
    }else if ((arg == "-d") || (arg == "--debug")) {
      debug = true;
    }else if ((arg == "-fp") || (arg == "--filter_pupil")) {
      filter_pupil = true;
    }else if ((arg == "-a") || (arg == "--algorithm")) {
      processStringOption(argc, i, argv, algoName, "--algorithm");
    }else if ((arg == "-p") || (arg == "--pupil-algorithm")) {
      processStringOption(argc, i, argv, pupilName, "--pupil-algorithm");
    }else if ((arg == "-f") || (arg == "--files")) {
      string files = "";
      if ( !processStringOption(argc, i, argv, files, "--files") ){
        utils::split(files, ';', fileList);
      }
    }else if ((arg == "-ct") || (arg == "--calibration_target")) {
      processStringOption(argc, i, argv, calibTarget, "--calibration_target");
    }else if ((arg == "-et") || (arg == "--error_target")) {
      processStringOption(argc, i, argv, errorTarget, "--error_target");
    }else if ((arg == "-o") || (arg == "--output")) {
      processStringOption(argc, i, argv, outputPath, "--output");
    }else if ((arg == "-x") || (arg == "--prefix")) {
      processStringOption(argc, i, argv, outPrefix, "--prefix");
    }else if ((arg == "-cv") || (arg == "--calibration_video")) {
      string calibVideo = "";
      if ( !processStringOption(argc, i, argv, calibVideo, "--calibration_video") ){
        utils::split(calibVideo, ';', calibList);
      }
    }else if ((arg == "-cp") || (arg == "--calibration_pupil")) {
      string calibPupil = "";
      if ( !processStringOption(argc, i, argv, calibPupil, "--calibration_pupil") ){
        utils::split(calibPupil, ';', calibPupilList);
      }
    }else if ((arg == "-ep") || (arg == "--estimation_pupil")) {
      string estimPupil = "";
      if ( !processStringOption(argc, i, argv, estimPupil, "--estimation_pupil") ){
        utils::split(estimPupil, ';', estimPupilList);
      }
    }else if ((arg == "-i") || (arg == "--interdistance")) {
      processStringOption(argc, i, argv, interpupil, "--interdistance");
    }
  }

  //check minimum parameters
  if( (algoName == "") || (fileList.size() != 2) || (outputPath == "") || (calibTarget == "") || ((calibList.size() != 2) && (calibPupilList.size() != 2)) || (interpupil == "") ){
    cerr << "You must specify --algorithm, --files, --output, --calibration_target, --calibration_video and --calibration_pupil options" << endl;
    logfile.writeError("You must specify --algorithm, --files, --output, --calibration_target, --calibration_video and --calibration_pupil options");
    logfile.write("Application finish with errors");
    return 1;
  }
  else if((calibList.size() != 0 ) && (calibPupilList.size() != 0)){
    cerr << "You must specify  --calibration_video and --calibration_pupil options, you have to choose only one calibration input" << endl;
    logfile.writeError("You must specify  --calibration_video and --calibration_pupil options, you have to choose only one calibration input");
    logfile.write("Application finish with errors");
    return 1;
  }

  //check algorithm parameter is correct
  if( algoName != ALGO_INTER && algoName != ALGO_GEO ){
    cerr << "Not valid algorithm" + algoName << endl;
    logfile.writeError("Not valid algorithm: " + algoName);
    logfile.write("Application finish with errors");
    return 1;
  }
  logfile.writeDebug( algoName + " algorithm selected");

  //check pupil detection algorithm
  if( pupilName == "" || ( pupilName != PUPIL_CANNY && pupilName != PUPIL_BLOB && pupilName != PUPIL_CONT ) ) pupilName = PUPIL_CONT;
  logfile.writeDebug( pupilName + " pupil algorithm selected");

  Calibration *calib;
  Interpolation *interp;
  Geomodel *geomod;
  if( algoName == ALGO_INTER ){
    interp = new Interpolation(&logfile);
    calib = (Calibration*) interp;
  }else if( algoName == ALGO_GEO ){
    geomod = new Geomodel(&logfile);
    calib = (Calibration*) geomod;
  }


  //extract pupil positions from calibration text files
  if(calibPupilList.size() == 2){
      ifstream calibPupilFile;
      for(unsigned int idx_eye = 0; idx_eye<2; idx_eye++){
          calibPupilFile.open(calibPupilList[idx_eye], ios_base::in);
          if(calibPupilFile.is_open()){
            logfile.writeDebug("extract pupil positions from calibration texts");
            //extract the pupil positions
            string  line;
            while(getline(calibPupilFile,line)){
              vector<string> line_separate;
              utils::split(line, ';', line_separate);
              if(line_separate.size() >= 3 && line_separate[0] != "Frame"){
                //is not the header
                unsigned int frame = std::stoll(line_separate[0]);
                cv::Point2d pupil_position;
                pupil_position.x = std::stof(line_separate[1]);
                pupil_position.y = std::stof(line_separate[2]);
                calib->addCalibrationData(idx_eye, frame, pupil_position);
              }
            }
          }
          else{
            cerr << "Not valid input calibration pupil file: " << calibPupilList[idx_eye] << endl;
            logfile.writeError("Not valid input calibration pupil file: " + calibPupilList[idx_eye]);
            logfile.write("Application finish with errors");
            if( algoName == ALGO_INTER ) delete interp;
            else if( algoName == ALGO_GEO ) delete geomod;
            return 1;
          }
          calibPupilFile.close();
      }

  }

  //extract calibration positions
  ifstream calibFile;
  calibFile.open(calibTarget, ios_base::in);
  if( calibFile.is_open() ){
    logfile.writeDebug("extraction calibration positions");
    //extract the target positions and time
    string  line;
    while ( getline(calibFile,line) ) {
      vector<string> line_separate;
      utils::split(line, ';', line_separate);
      if( line_separate.size() >= 4 && line_separate[0] != "Frame"){
        //is not the header
        unsigned int frame = std::stoll(line_separate[0]);
        float posx = std::stof(line_separate[1]);
        float posy = std::stof(line_separate[2]);
        float posz = std::stof(line_separate[3]);
        calib->addCalibrationTarget( frame, posx, posy, posz );
      }
    }
  }else{
    cerr << "Not valid input calibration target file: " << calibTarget << endl;
    logfile.writeError("Not valid input calibration target file: " + calibTarget);
    logfile.write("Application finish with errors");
    if( algoName == ALGO_INTER ) delete interp;
    else if( algoName == ALGO_GEO ) delete geomod;
    return 1;
  }
  calibFile.close();

  //extract error estimation positions
  if( errorTarget != "" ){ 
    logfile.writeDebug("extraction target error estimation positions");
    ifstream errorFile;
    errorFile.open(errorTarget, ios_base::in);
    if( errorFile.is_open() ){
      //extract the target positions and time
      string  line;
      while ( getline(errorFile,line) ) {
        vector<string> line_separate;
        utils::split(line, ';', line_separate);
        if( line_separate.size() >= 2 && line_separate[0] != "Frame"){
          //is not the header
          float posx = std::stof(line_separate[1]);
          float posy = std::stof(line_separate[2]);
          float posz = std::stof(line_separate[3]);
          errorList.push_back(cv::Point3d(posx, posy, posz));
        }
      }
    }else{
      cerr << "Not valid input error estimation target file: " << errorTarget << endl;
      logfile.writeError("Not valid input error estimation target file: " + errorTarget);
      logfile.write("Application finish with errors");
      if( algoName == ALGO_INTER ) delete interp;
      else if( algoName == ALGO_GEO ) delete geomod;
      return 1;
    }
    errorFile.close();
  }

  //extract pupil positions from calibration videos
  if(calibList.size() == 2){
      cv::VideoCapture vid;
      cv::Mat frame,pic_th,th_edges;
      for(unsigned int idx_eye = 0; idx_eye<2; idx_eye++){
        vid.open(calibList[idx_eye]);
        // cout << "Video Cal: " << calibList[idx_eye] << endl;
        if(vid.isOpened()){
          int frame_size=vid.get(CV_CAP_PROP_FRAME_COUNT);
          for(int idx_frame=0; idx_frame<frame_size; idx_frame++){
            vid >> frame;
            cv::Point pupil_center;
            std::vector<cv::Point> glints;
            // cout << "FRAME: " << idx_frame << endl;
            if( algoName == ALGO_GEO ){
              extractEyeFeatures(pupilName, frame, pupil_center, glints);
              calib->addCalibrationData(idx_eye, idx_frame, pupil_center, &glints);
            }else{
              extractEyeFeatures(pupilName, frame, pupil_center);
              calib->addCalibrationData(idx_eye, idx_frame, pupil_center);
            }
            // cout << "TEST: " << pupil_center << endl;

            if( debug ){
              //paint for debuging
              // cout << GET_EYENAME(idx_eye) << " FRAME: " << idx_frame << endl;
              // cout << "  PUPIL CENTER: " << pupil_center << endl;
              cv::circle(frame, pupil_center, 4, CV_RGB(255,0,0), 2);
              // cout << "  GLINTS: " << glints.size() << endl;
              for( size_t idx_glint=0; idx_glint < glints.size(); idx_glint++){
                // cout << "  - " << glints[idx_glint] << endl;
                cv::circle(frame, glints[idx_glint], 4, CV_RGB(0,0,255), 2);
              }
              cv::imshow("image", frame);
              cv::waitKey(0); //change for cv::waitKey(10) if you want not to press spacebar
            }
          }

          vid.release();

          //filter pupil values
          if( filter_pupil ){
            vector<eye_feature_t> *vecpupil;
            vecpupil = calib->get_eyefeatures(idx_eye);
            if( vecpupil == NULL ){ logfile.writeWarn("Not exist pupil vector for: " + string(GET_EYENAME(idx_eye))); }
            if( ! filterPupil(vecpupil) ){
              logfile.writeWarn("Not applying filter pupil for: " + string(GET_EYENAME(idx_eye)));
            }else{
              if( debug ){
                vid.open(calibList[idx_eye]);
                int frame_size=vid.get(CV_CAP_PROP_FRAME_COUNT);
                for(int idx_frame=0;idx_frame<frame_size;idx_frame++){
                  vid >> frame;
                  cv::circle(frame, (*vecpupil)[idx_frame].pupil_center, 4, CV_RGB(0,0,255), 2);
                  cv::imshow("image", frame);
                  cv::waitKey(10); //change for cv::waitKey(10) if you want not to press spacebar
                }
                vid.release();
              }
            }
          }

        }else{
          cerr << "Not valid input calibration video: " << calibList[idx_eye] << endl;
          logfile.writeError("Not valid input calibration video: " + calibList[idx_eye]);
          logfile.write("Application finish with errors");
          return 1;
        }
      }
  }

  //add camera information
  double tvec_l[9]  = {-67, 378, 35};
  double rmat_l[9]  = {-0.089193, 0.992427, -0.084458,
                       0.995625,  0.091206,  0.020282,
                       0.027832, -0.082280, -0.996221 };
  double cammat_l[9]  = {7.6492626645285077e+002, 0.                     , 3.1950000000000000e+002,
                         0.                     , 7.6492626645285077e+002, 2.3950000000000000e+002,
                         0.                     , 0.                     , 1.                     };
  double distmat_l[5] = {-9.7557387688586572e-002, 1.5110053335383864e+000, 0., 0., -7.7234823468854303e+000};
  Camera *cam_l = calib->getCamera(EYE_LEFT);
  cam_l->setTranslationVector(tvec_l);
  cam_l->setRotationMatrix(rmat_l);
  cam_l->setIntrinsicMatrix(cammat_l);
  cam_l->setDistortionMatrix(distmat_l);
  cam_l->setResolution(640,480);
  cam_l->setPosition(cv::Point3d(-67, 378, 35));

  double tvec_r[9]  = {67, 378, 35};
  double rmat_r[9]  = {-0.089193, 0.992427, -0.084458,
                       0.995625,  0.091206,  0.020282,
                       0.027832, -0.082280, -0.996221 };
  double cammat_r[9]  = {8.2605953379878736e+002, 0.                     , 3.1950000000000000e+002,
                         0.                     , 8.2605953379878736e+002, 2.3950000000000000e+002,
                         0.                     , 0.                     , 1.                     };
  double distmat_r[5] = {-1.9164811736455795e-001, 4.3927568558111147e+000, 0., 0., -4.3098736663073929e+001};
  Camera *cam_r = calib->getCamera(EYE_RIGHT);
  cam_r->setTranslationVector(tvec_r);
  cam_r->setRotationMatrix(rmat_r);
  cam_r->setIntrinsicMatrix(cammat_r);
  cam_r->setDistortionMatrix(distmat_r);
  cam_r->setResolution(640,480);
  cam_r->setPosition(cv::Point3d(67, 378, 35));



  if( algoName == ALGO_GEO ){
    //at some point, this should be in a configuration file
    //add led information (position respect to the camera)
    geomod->addLed(EYE_RIGHT, cv::Point3d(0,-13,0));
    geomod->addLed(EYE_RIGHT, cv::Point3d(-13,5,0));
    geomod->addLed(EYE_LEFT, cv::Point3d(13,-5,0));
    geomod->addLed(EYE_LEFT, cv::Point3d(0,13,0));
  }else{
    interp->setInterPupil(atoi(interpupil.c_str()));
  }

  // Call to calibration algorithm
  if( ! calib->calibrate() ){
    string err_msg = "Not posible Calibration \n";
    cerr << err_msg;
    logfile.writeError(err_msg);
    logfile.write("Application finish with errors");
    return 1;
  }


  if( algoName == ALGO_GEO ){
    //print information
    std::ostringstream out;
    out << *geomod;
    logfile.writeDebug( out.str() );
  }
  else if( algoName == ALGO_INTER ){
    std::ostringstream out;
    out << *interp;
    logfile.writeDebug( out.str() );
  }


  // after calibration, start estimation
  // open input text files
  vector<cv::Point2d> pupilEstimation[2];
  if(estimPupilList.size() == 2){
      ifstream estimPupilFile;
      for(unsigned int idx_eye = 0; idx_eye<2; idx_eye++){
          estimPupilFile.open(estimPupilList[idx_eye], ios_base::in);
          if(estimPupilFile.is_open()){
            logfile.writeDebug("extract pupil positions from calibration texts");
            //extract the pupil positions
            string  line;
            while(getline(estimPupilFile,line)){
              vector<string> line_separate;
              utils::split(line, ';', line_separate);
              if(line_separate.size() >= 3 && line_separate[0] != "Frame"){
                //is not the header
                // unsigned int frame = std::stoll(line_separate[0]);
                cv::Point2d pupil_position;
                pupil_position.x = std::stof(line_separate[1]);
                pupil_position.y = std::stof(line_separate[2]);
                pupilEstimation[idx_eye].push_back(pupil_position);
              }
            }
          }
          else{
            cerr << "Not valid input estimation pupil file: " << calibPupilList[idx_eye] << endl;
            logfile.writeError("Not valid input estimation pupil file: " + calibPupilList[idx_eye]);
            logfile.write("Application finish with errors");
            if( algoName == ALGO_INTER ) delete interp;
            else if( algoName == ALGO_GEO ) delete geomod;
            return 1;
          }
          estimPupilFile.close();
      }
      if(pupilEstimation[EYE_RIGHT].size() != pupilEstimation[EYE_LEFT].size() ){
        logfile.writeWarn("Frames in text files are different for each eye: R:" + utils::uint2str(pupilEstimation[EYE_RIGHT].size() )
                          + " L:" + utils::uint2str(pupilEstimation[EYE_LEFT].size()) + " . Using R");
      }      
  }

  //create output file
  string output = outputPath + "/" + outPrefix + "_" + OUT_FILE;
  ofstream outfile(output, ios::out );
  if( !outfile.is_open() ){
    string err_msg = "Not posible to create output: \n";
    err_msg += " - Output file:      " + output    + "\n";
    cerr << err_msg;
    logfile.writeError(err_msg);
    logfile.write("Application finish with errors");
    return 1;
  }

  //write header in the output file
  outfile << "Frame; POS_X; POS_Y; POS_Z; Duration(microseconds)" << endl;

  vector<cv::Point3d> result;
  double times = 0;
  for(size_t j=0;j < pupilEstimation[EYE_RIGHT].size();j++){
    cv::Point3d estimated;
    //cout << j << ": ";
    //Estimate 3D position with calibration data
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    estimated = calib->calcCoordinates( pupilEstimation[EYE_RIGHT].at(j), pupilEstimation[EYE_LEFT].at(j) );
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    //cout << estimated << endl;
    //write the estimated position and time
    outfile << j << ";" << estimated.x << ";" << estimated.y << ";" << estimated.z << ";" << duration << endl;
    //add to vector for error estimation
    result.push_back(estimated);
    times += duration;
  }
  logfile.write( "average TIME in calculation: " + utils::double2str(times/pupilEstimation[EYE_RIGHT].size()) + " microsecs" );

  //if file for comparison is given, then calculate the error average and standar deviation
  if( errorList.size() == result.size() ){
    double mean_deg=0, mean_mm=0, mean2d_deg=0, mean2d_mm=0, std_deg=0, std_mm=0, std2d_deg=0, std2d_mm=0;
    string stdoutput = outputPath + "/" + outPrefix + "_" + STD_FILE;
    calib->calculateError(errorList, result, mean_deg, mean_mm, mean2d_deg, mean2d_mm, std_deg, std_mm, std2d_deg, std2d_mm, stdoutput);
    logfile.write( "TOTAL points estimated:        " + utils::uint2str(result.size()) );
    logfile.write( "average ERROR (degree):        " + utils::double2str(mean_deg) + "deg" );
    logfile.write( "average ERROR (milimiters):    " + utils::double2str(mean_mm) + "mm" );
    logfile.write( "average ERROR 2d (degree):     " + utils::double2str(mean2d_deg) + "deg" );
    logfile.write( "average ERROR 2d (milimiters): " + utils::double2str(mean2d_mm) + "mm" );
    logfile.write( "std dev ERROR (degree):        " + utils::double2str(std_deg) + "deg" );
    logfile.write( "std dev ERROR (milimiters):    " + utils::double2str(std_mm) + "mm" );
    logfile.write( "std dev ERROR 2d (degree):     " + utils::double2str(std2d_deg) + "deg" );
    logfile.write( "std dev ERROR 2d (milimiters): " + utils::double2str(std2d_mm) + "mm" );
  }else{
    logfile.writeDebug( utils::uint2str(errorList.size()) + " comparison points provided" );
    logfile.writeDebug( "total points estimated: " + utils::uint2str(result.size()) );
  }
  if( outfile.is_open() ) outfile.close();

/*
  cv::VideoCapture vid_R, vid_L;
  cv::Mat frame_L, pic_th_L, th_edges_L;
  cv::Mat frame_R, pic_th_R, th_edges_R;
  cv::Point pupil_R, pupil_L;

  //create output file
  string output = outputPath + "/" + outPrefix + "_" + OUT_FILE;
  ofstream outfile(output, ios::out );
  if( !outfile.is_open() ){
    string err_msg = "Not posible to create output: \n";
    err_msg += " - Output file:      " + output    + "\n";
    cerr << err_msg;
    logfile.writeError(err_msg);
    logfile.write("Application finish with errors");
    return 1;
  }

  //Open input video files
  vid_R.open(fileList[0]);
  vid_L.open(fileList[1]);
  if( vid_R.isOpened() && vid_L.isOpened() ){
    int frame_size_R=vid_R.get(CV_CAP_PROP_FRAME_COUNT);
    int frame_size_L=vid_L.get(CV_CAP_PROP_FRAME_COUNT);

    if(frame_size_R != frame_size_L ){
      logfile.writeWarn("Frames in video files are different for each eye: R:" + utils::uint2str(frame_size_R) + " L:" + utils::uint2str(frame_size_L) + " . Using R");
    }

    //write header in the output file
    outfile << "Frame; POS_X; POS_Y; POS_Z; Duration(microseconds)" << endl;

    vector<cv::Point3d> result;
    double times = 0;
    for(int j=0;j<frame_size_R;j++){
      cv::Point3d estimated;
      vid_R >> frame_R;
      vid_L >> frame_L;

      //calculate pupil position
      extractEyeFeatures(pupilName, frame_R, pupil_R);
      extractEyeFeatures(pupilName, frame_L, pupil_L);

      //Estimate 3D position with calibration data
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      estimated = calib->calcCoordinates( pupil_R, pupil_L );
      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
      //write the estimated position and time
      outfile << j << ";" << estimated.x << ";" << estimated.y << ";" << estimated.z << ";" << duration << endl;
      //add to vector for error estimation
      result.push_back(estimated);
      times += duration;
    }
    logfile.write( "average TIME in calculation: " + utils::double2str(times/frame_size_R) + " microsecs" );

    //if file for comparison is given, then calculate the error average and standar deviation
    if( errorList.size() == result.size() ){
      double mean_deg=0, mean_mm=0, mean2d_deg=0, mean2d_mm=0, std_deg=0, std_mm=0, std2d_deg=0, std2d_mm=0;
      string stdoutput = outputPath + "/" + outPrefix + "_" + STD_FILE;
      calib->calculateError(errorList, result, mean_deg, mean_mm, mean2d_deg, mean2d_mm, std_deg, std_mm, std2d_deg, std2d_mm, stdoutput);
      logfile.write( "TOTAL points estimated:        " + utils::uint2str(result.size()) );
      logfile.write( "average ERROR (degree):        " + utils::double2str(mean_deg) + "deg" );
      logfile.write( "average ERROR (milimiters):    " + utils::double2str(mean_mm) + "mm" );
      logfile.write( "average ERROR 2d (degree):     " + utils::double2str(mean2d_deg) + "deg" );
      logfile.write( "average ERROR 2d (milimiters): " + utils::double2str(mean2d_mm) + "mm" );
      logfile.write( "std dev ERROR (degree):        " + utils::double2str(std_deg) + "deg" );
      logfile.write( "std dev ERROR (milimiters):    " + utils::double2str(std_mm) + "mm" );
      logfile.write( "std dev ERROR 2d (degree):     " + utils::double2str(std2d_deg) + "deg" );
      logfile.write( "std dev ERROR 2d (milimiters): " + utils::double2str(std2d_mm) + "mm" );
    }else{
      logfile.writeDebug( utils::uint2str(errorList.size()) + " comparison points provided" );
      logfile.writeDebug( "total points estimated: " + utils::uint2str(result.size()) );
    }

  }else{
    string err_msg = "Not valid input pupil videos: \n";
    err_msg += " - Video File Right: " + fileList[0] + "\n";
    err_msg += " - Video File Left:  " + fileList[1] + "\n";
    cerr << err_msg;
    logfile.writeError(err_msg);
    logfile.write("Application finish with errors");
  }

  //close the streams
  if( outfile.is_open() ) outfile.close();
  if( vid_R.isOpened()  ) vid_R.release();
  if( vid_L.isOpened()  ) vid_L.release();
*/
  //free memory space
  if( algoName == ALGO_INTER ) delete interp;
  else if( algoName == ALGO_GEO ) delete geomod;

  //copy log file to output dir
  string output_log = outputPath + "/" + outPrefix + "_" + LOG_FILE;
  logfile.copylogfile(output_log);

  logfile.write("Application finish");

  return 0;
}

