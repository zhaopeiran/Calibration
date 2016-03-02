#include "geometry.hpp"
#include "calibration/Interpolation.hpp"
#include <math.h>

#define POLY_LENGTH_X 6
#define NUMBER_X 1
#define POLY_LENGTH_Y 6
#define NUMBER_Y 1

//parameters with unit millimeter
#define D 400      //distance between based plane and fixed plane at z axis
#define L 1200     //distance between based plane and camera plane at z axis
#define M 410      //distance between table plane and two eyes plane at y axis

Interpolation::~Interpolation() {
  if( _mapMat_x[EYE_RIGHT] != NULL){ delete _mapMat_x[EYE_RIGHT]; }
  if( _mapMat_y[EYE_RIGHT] != NULL){ delete _mapMat_y[EYE_RIGHT]; }
  if( _mapMat_x[EYE_LEFT] != NULL){ delete _mapMat_x[EYE_LEFT]; }
  if( _mapMat_y[EYE_LEFT] != NULL){ delete _mapMat_y[EYE_LEFT]; }
}
//4,1 + 5,1
//4,3 + 4,4
//4,1 + 4,2
vector<float> Interpolation::createInputFormat(const cv::Point2d &p, const int &n, const int &number){
    vector<float> input;
    input.resize(n);
    if(n == 2 && number == 1){
        // a0*x + a1*y
        input[0] = p.x;
        input[1] = p.y;
    }
    else if(n == 3 && number == 1){
        // a0 + a1*x + a2*x^2
        input[0] = 1;
        input[1] = p.x;
        input[2] = pow(p.x, 2);
    }
    else if(n == 3 && number == 2){
         // a0 + a1*y + a2*y^2
        input[0] = 1;
        input[1] = p.y;
        input[2] = pow(p.y, 2);
    }
    else if(n == 4 && number == 1){
        // a0 + a1*x + a2*y + a3*x^2
        input[0] = 1;
        input[1] = p.x;
        input[2] = p.y;
        input[3] = pow(p.x, 2);
    }
    else if(n == 4 && number == 2){
        // a0 + a1*x + a2*y + a3*y^2
        input[0] = 1;
        input[1] = p.x;
        input[2] = p.y;
        input[3] = pow(p.y, 2);
    }
    else if(n == 4 && number == 3){
        // a0 + a1*x + a2*x^3 + a3*y^2
        input[0] = 1;
        input[1] = p.x;
        input[2] = pow(p.x, 3);
        input[3] = pow(p.y, 2);
    }
    else if(n == 4 && number == 4){
        // a0 + a1*x + a2*y + a3*x^2 * y
        input[0] = 1;
        input[1] = p.x;
        input[2] = p.y;
        input[3] = pow(p.x, 2) * p.y;
    }
    else if(n == 5 && number == 1){
        // a0 + a1*y + a2*x^2 + a3*x*y + a4*x^2*y
        input[0] = 1;
        input[1] = p.y;
        input[2] = pow(p.x, 2);
        input[3] = p.x * p.y;
        input[4] = pow(p.x, 2) * p.y;
    }
    else if(n == 6 && number == 1){
        // a0 + a1*x + a2*y + a3*x^2 + a4*y^2 + a5*x*y
        input[0] = 1;
        input[1] = p.x;
        input[2] = p.y;
        input[3] = pow(p.x, 2);
        input[4] = pow(p.y, 2);
        input[5] = p.x * p.y;
    }
    else if (n == 10 && number == 1){
        // a0 + a1*x + a2*y + a3*x^2 + a4*y^2 + a5*x*y + a6*x^3 + a7*y^3 + a8*x^2*y + a9*x*y^2
        input[0] = 1;
        input[1] = p.x;
        input[2] = p.y;
        input[3] = pow(p.x, 2);
        input[4] = pow(p.y, 2);
        input[5] = p.x * p.y;
        input[6] = pow(p.x, 3);
        input[7] = pow(p.y, 3);
        input[8] = pow(p.x, 2) * p.y;
        input[9] = pow(p.y, 2) * p.x;
    }

    //......
    return input;
}

cv::Point2d Interpolation::map(const vector<float> input_x, const vector<float> input_y, const int &idx_eye){
  cv::Point2d result;
  if(_ismapMatCaculated){
    //map x
    cv::Mat inMat_x = cv::Mat::zeros(1, POLY_LENGTH_X, CV_32FC1);
    for(size_t i = 0; i < input_x.size(); i++){
      inMat_x.at<float>(i) = input_x[i];
    }
    cv::Mat outMat_x = cv::Mat::zeros(1, 1, CV_32FC1);
    outMat_x = inMat_x * (*_mapMat_x[idx_eye]);
    result.x = outMat_x.at<float>(0);
    //map y
    cv::Mat inMat_y = cv::Mat::zeros(1, POLY_LENGTH_Y, CV_32FC1);
    for(size_t i = 0; i < input_y.size(); i++){
      inMat_y.at<float>(i) = input_y[i];
    }
    cv::Mat outMat_y = cv::Mat::zeros(1, 1, CV_32FC1);
    outMat_y = inMat_y * (*_mapMat_y[idx_eye]);
    result.y = outMat_y.at<float>(0);
  }
  return result;
}

/* input: a0 + a1*x + a2*y + a3*x^2 + a4*y^2 + a5*x*y            // + a6*x^3 + a7*y^3 + a8*x^2*y + a9*x*y^2
 * inputCount: 6
 * output: { est.x, est.y }
 * outputCount: 2
 * inputDataMatrix: n * 6
 * outputDataMatrix: n * 2
 * mapMat: 6 * 2
 * estMatrix = inputDataMatrix * mapMat: n * 2
 *
 * then compare estMatrix with outputDataMatrix.
 *
 * step:
 * 1. fetch the pupil center pos. from eye features (each frame) and the targets pos.;
 * 2. exclude (0,0) pupil datas;
 * 3. change the each values of pupil center unit from 'px; to 'millimeter';
 * 4. save the {plx, ply, prx, pry} into "inputData";
 * 5. save the {tx, ty } into "outputData";
 * 6. caculate the coeff. Matrix "_mapMat" with func. cvSolve();
 * 7. caculate "estimation" with estimating "inputData";
 * 8. at last caculate the calibration Error;
 */
bool Interpolation::calibrate(){
  _logFileWriter->writeDebug("bool Interpolation::calibrate(int accuracyThreshold, int maxExceedence)");
  //ready
  if( _eye_features[EYE_RIGHT].size() != _eye_features[EYE_LEFT].size() ){
    _logFileWriter->writeError("Frames in input files are different for each eye. RIGHT: " 
                               + utils::uint2str(_eye_features[EYE_RIGHT].size())
                               + " LEFT: " + utils::uint2str(_eye_features[EYE_LEFT].size()));
    return false;
  }
  else if(_eye_features[EYE_RIGHT].size() != _calib_targets.size()){
    _logFileWriter->writeError("Frames are different between targets file and pupil file. PUPIL: "
                               + utils::uint2str(_eye_features[EYE_RIGHT].size())
                               + " TARGETS: " + utils::uint2str(_calib_targets.size()));

    return false;
  }

  const int inputCount_x = POLY_LENGTH_X;
  const int inputCount_y = POLY_LENGTH_Y;
  vector <vector<float>> inputData_x[2]; // n*6
  vector <vector<float>> inputData_y[2]; // n*6
  vector<double> outputData_x;      // n*1
  vector<double> outputData_y;      // n*1

  //get "inputData", "outputData" and the length of valid frames;
  for(unsigned int i = 0;i < _calib_targets.size();i++){
    if( ((_eye_features[EYE_RIGHT])[i].pupil_center.x != 0 || (_eye_features[EYE_RIGHT])[i].pupil_center.y != 0) &&    // zero filter
       ((_eye_features[EYE_LEFT])[i].pupil_center.x != 0 || (_eye_features[EYE_LEFT])[i].pupil_center.y != 0) ){
        for(unsigned int idx_eye = 0; idx_eye < 2; idx_eye++){
            vector<float> in_x = createInputFormat((_eye_features[idx_eye])[i].pupil_center, inputCount_x, NUMBER_X);
            vector<float> in_y = createInputFormat((_eye_features[idx_eye])[i].pupil_center, inputCount_y, NUMBER_Y);
            inputData_x[idx_eye].push_back(in_x);
            inputData_y[idx_eye].push_back(in_y);
        }
        outputData_x.push_back(_calib_targets[i].pos_x);
        outputData_y.push_back(_calib_targets[i].pos_y);
        //pos_z = L - D
    }
  }
  size_t lengthOfVaildFrames = outputData_x.size();
  //determine "_mapMat";
  if(!_ismapMatCaculated){
    cv::Mat inputMat_x[2];
    cv::Mat inputMat_y[2];
    inputMat_x[EYE_RIGHT] = cv::Mat::zeros(lengthOfVaildFrames, inputCount_x, CV_32FC1);
    inputMat_y[EYE_RIGHT] = cv::Mat::zeros(lengthOfVaildFrames, inputCount_y, CV_32FC1);
    inputMat_x[EYE_LEFT] = cv::Mat::zeros(lengthOfVaildFrames, inputCount_x, CV_32FC1);
    inputMat_y[EYE_LEFT] = cv::Mat::zeros(lengthOfVaildFrames, inputCount_y, CV_32FC1);
    cv::Mat outputMat_x = cv::Mat::zeros(lengthOfVaildFrames, 1, CV_32FC1);
    cv::Mat outputMat_y = cv::Mat::zeros(lengthOfVaildFrames, 1, CV_32FC1);
    for(size_t i = 0; i < lengthOfVaildFrames; i++) {
        for(unsigned int j = 0; j < inputCount_x;j++){
            inputMat_x[EYE_RIGHT].at<float>(i,j) = (inputData_x[EYE_RIGHT])[i][j];
            inputMat_x[EYE_LEFT].at<float>(i,j) = (inputData_x[EYE_LEFT])[i][j];
        }
        for(unsigned int j = 0; j < inputCount_y;j++){
            inputMat_y[EYE_RIGHT].at<float>(i,j) = (inputData_y[EYE_RIGHT])[i][j];
            inputMat_y[EYE_LEFT].at<float>(i,j) = (inputData_y[EYE_LEFT])[i][j];
        }

        outputMat_x.at<float>(i,0) = outputData_x[i]; //right?
        outputMat_y.at<float>(i,0) = outputData_y[i];
    }

    try {
        for(unsigned int idx_eye = 0; idx_eye < 2; idx_eye++){
            _mapMat_x[idx_eye] = new cv::Mat(cv::Mat::zeros(inputCount_x, 1, CV_32FC1));
            _mapMat_y[idx_eye] = new cv::Mat(cv::Mat::zeros(inputCount_y, 1, CV_32FC1));
            if(!cv::solve(inputMat_x[idx_eye], outputMat_x, *_mapMat_x[idx_eye], cv::DECOMP_QR))  //DECOMP_SVD   DECOMP_LU   DECOMP_QR
                return false;
            if(!cv::solve(inputMat_y[idx_eye], outputMat_y, *_mapMat_y[idx_eye], cv::DECOMP_QR))
                return false;
        }
    }
    catch (cv::Exception &e) {
        _logFileWriter->write("Could not solve: " + e.msg);
        return false;
    }
    _ismapMatCaculated = true;   //mapMat has been caculated now.
  }
  /*
  //print data:
  std::cout << "all vector sizes:" << std::endl
            <<"pupil_Data[EYE_RIGHT]: " << (pupil_Data[EYE_RIGHT]).size() << std::endl
            <<"pupil_Data[EYE_LEFT]: " << (pupil_Data[EYE_LEFT]).size() << std::endl
            <<"inputData[EYE_RIGHT]: " << inputData[EYE_RIGHT].size() << std::endl
            <<"inputData[EYE_LEFT]: " << inputData[EYE_LEFT].size() << std::endl
            <<"outputData: " << outputData.size() << std::endl
            <<"this_err: " << this_err.size() << std:: endl
            << "_calib_targets.size():" << _calib_targets.size() << std::endl
            << "eye_features[EYE_RIGHT].size(): " << _eye_features[EYE_RIGHT].size() << std::endl
            << "eye_features[EYE_LEFT].size():  " << _eye_features[EYE_LEFT].size() << std::endl;

  std::cout << std::endl
            << "pupil_Data[EYE_RIGHT]:      pupil_Data[EYE_LEFT]:        outputData:                         estimation:                      err:"
            << std::endl;
  for(size_t i = 0; i < lengthOfVaildFrames;i++){   
      std::cout << (pupil_Data[EYE_RIGHT])[i] << ",                 ";
      std::cout << (pupil_Data[EYE_LEFT])[i]  << ",                 ";
      std::cout << outputData[i]              << ",                 ";
      std::cout << estimation[i]              << ",                 ";
      std::cout << this_err[i]                << std::endl;
  }

  std::cout << std::endl << "*_mapMat[EYE_RIGHT]: " << std::endl
            << *(_mapMat[EYE_RIGHT]) << std::endl << std::endl
            << "*_mapMat[EYE_LEFT]: " << std::endl
            << *(_mapMat[EYE_LEFT]) << std::endl;

  */
  return true;
}

/* step:
 * 1. append this 2 pupil positions: (actually 4 values: {plx, ply, prx, pry})
 * 2. Multi. with coeff. Matrix.
 */
cv::Point3d Interpolation::calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L){
  // _logFileWriter->writeDebug("cv::Point3d Interpolation::calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L)");
  if(!_ismapMatCaculated){
    cerr << "_mapMat is free now, calibration are not finished.";
    _logFileWriter->writeError("_mapMat is free now, calibration are not finished.");
    return cv::Point3d(0,0,0);
  }
  //first part: interpolation
  vector<float> input_line_x[2];
  vector<float> input_line_y[2];
  cv::Point2d output_line[2];
    //PoR_right
  input_line_x[EYE_RIGHT] = createInputFormat(pupil_R, POLY_LENGTH_X, NUMBER_X);
  input_line_y[EYE_RIGHT] = createInputFormat(pupil_R, POLY_LENGTH_Y, NUMBER_Y);
  output_line[EYE_RIGHT] = map(input_line_x[EYE_RIGHT], input_line_y[EYE_RIGHT], EYE_RIGHT);

    //PoR_left
  input_line_x[EYE_LEFT] = createInputFormat(pupil_L, POLY_LENGTH_X, NUMBER_X);
  input_line_y[EYE_LEFT] = createInputFormat(pupil_L, POLY_LENGTH_Y, NUMBER_Y);
  output_line[EYE_LEFT] = map(input_line_x[EYE_LEFT], input_line_y[EYE_LEFT], EYE_LEFT);

  //second part: vergence
  cv::Point2d fixRight, fixLeft;
  fixRight = output_line[EYE_RIGHT];
  fixLeft  = output_line[EYE_LEFT];

  cout << fixRight.x << ";" << fixRight.y << ";" << fixLeft.x << ";" << fixLeft.y << endl;
 
  double deltaX = std::abs(fixRight.x - fixLeft.x);
  cv::Point3d PoR;
  if(fixRight.x > fixLeft.x){               //in front of fixed plane
    PoR.z = _interpupil * D / (_interpupil-deltaX);
    PoR.x = ((_interpupil * fixRight.x)-(deltaX * _interpupil/2)) / (_interpupil - deltaX);
    PoR.y = ((fixRight.y+fixLeft.y)/2 - M) * PoR.z / D + M;
  }
  else if(fixRight.x == fixLeft.x){         //on fixed plane
    PoR.x = fixRight.x;
    PoR.y = (fixRight.y + fixLeft.y) / 2;
    PoR.z = D;
  }else{                                    //between based plane and fixed plane
    PoR.z = _interpupil * D / (_interpupil+deltaX);
    PoR.x = ((_interpupil * fixRight.x)+(deltaX * _interpupil/2)) / (_interpupil + deltaX);
    PoR.y = ((fixRight.y+fixLeft.y)/2 - M) * PoR.z / D + M;
  }

/*
  //second part midpoint
  cv::Point3d eyeR, eyeL, gazeR, gazeL;
  eyeR.x = _interpupil/2;
  eyeR.y = M;
  eyeR.z = L;
  eyeL.x = -_interpupil/2;
  eyeL.y = M;
  eyeL.z = L;
  gazeR.x = fixRight.x - eyeR.x;
  gazeR.y = fixRight.y - eyeR.y;
  gazeR.z = L - D - eyeR.z;
  gazeL.x = fixLeft.x - eyeL.x;
  gazeL.y = fixLeft.y - eyeL.y;
  gazeL.z = L - D - eyeL.z;
  cv::Point3d PoR = geometry::get_midpoint_of2vectors(eyeR, gazeR, eyeL, gazeL);
*/
  return PoR;
}
