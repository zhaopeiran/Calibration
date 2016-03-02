#ifndef _CALIBRATION_HPP_
#define _CALIBRATION_HPP_

#include <vector>
#include <opencv2/core/core.hpp>

#include "utils.hpp"
#include "LogFileWriter.hpp"
#include "calibration/Camera.hpp"
#include "geometry.hpp"

#define GET_EYENAME(a) ((a == 0)?("Right"):("Left"))
#define EYE_RIGHT 0
#define EYE_LEFT 1
#define EYE_MAX 1


struct eye_feature_t {
  int frame;
  cv::Point2d pupil_center;
  std::vector<cv::Point> corneal_reflections;
};

struct calib_target_t {
  unsigned int frame;
  float pos_x;
  float pos_y;
  float pos_z;
};

/**
 * The Calibration class provides methods to calculate the Gaze Tracking calibration.
 * This class must be used to initially calibrate the Gaze Tracking System. After the
 * calibration a call to calcCoordinates() will calculate the point on the screen
 * where a user is looking at.
 */
class Calibration {

public:

protected:
  unsigned int            _num_frames;
  Camera                  _cameras[2];      //one camera per eye
  vector<eye_feature_t>   _eye_features[2];
  vector<calib_target_t>  _calib_targets;
  LogFileWriter          *_logFileWriter;

public:
  /**
   * Default construtor
   */
  inline Calibration(LogFileWriter *logFileWriter)
    : _logFileWriter(logFileWriter) {
    _num_frames = 0;
    _eye_features[EYE_RIGHT].clear();
    _eye_features[EYE_LEFT].clear();
    _calib_targets.clear();
  }

  /**
   * Default destructor
   */
  virtual inline ~Calibration() {};

  inline vector<eye_feature_t>* get_eyefeatures(unsigned int eye){ if( eye <= EYE_MAX) return &_eye_features[eye]; else return NULL; }

  /**
   * Add a new measurement in form of pupil center and corneal reflections for right and left eye.
   *
   */
  inline void addCalibrationData(unsigned int eye, unsigned int frame,
                                 const cv::Point2d &pupil_center, const std::vector<cv::Point> *corneal_reflections=NULL){
    eye_feature_t tmp;
    tmp.frame               = frame;
    tmp.pupil_center        = pupil_center;
    if( corneal_reflections != NULL)
        tmp.corneal_reflections = *corneal_reflections;
    _eye_features[eye].push_back(tmp);
    _num_frames++;
  }


  inline void addCalibrationTarget(unsigned int frame, float pos_x, float pos_y, float pos_z){
    calib_target_t tmp;
    tmp.frame = frame;
    tmp.pos_x = pos_x;
    tmp.pos_y = pos_y;
    tmp.pos_z = pos_z;
    _calib_targets.push_back(tmp);
  }

  inline Camera* getCamera(unsigned int eye){
    if( eye > EYE_MAX ){ return NULL; }
    else{
      return &_cameras[eye];
    }
  }

  friend ostream &operator<<( ostream &output, const Calibration &model ){
    output << endl;
    output << "Num frames: " << model._num_frames << endl;
    for(unsigned int idx_eye=0; idx_eye<2; idx_eye++){
      output << "Camera " << GET_EYENAME(idx_eye) << " " << endl << model._cameras[idx_eye] << endl;
      output << "Num Eye Features " << GET_EYENAME(idx_eye) << ": " <<  model._eye_features[idx_eye].size() << endl;
      output  << endl;
    }
    output << "Num Calibration Targets: " << model._calib_targets.size() << endl;
    return output;
  }


  /**
   *
   * Calculates the calibration coefficients.
   *
   * If the method returns false, the calibration is not accurate enough
   * and has to be repeated.
   *
   * @return true if the calirbation succed, false otherwise of measurements (calibration ata) with a gap
   * larger than the accuracyThreshold does not exceed maxExceedence.
   * False otherwise.
   *
   */
  virtual bool calibrate() = 0;

  /**
   * Calculates the coordinates based on gaze-vectors.
   *
   * Should only be called after the coefficients have been caluculated.
   *
   * @param pupil positions of rigth and left eye respectively.
   * @return Estimated point on space.
   */
  virtual cv::Point3d calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L) = 0;


  inline void calculateError(const vector<cv::Point3d> &list_real, const vector<cv::Point3d> &list_estimated, double &mean_deg, double &mean_mm, double &mean2d_deg, double &mean2d_mm, double &std_deg, double &std_mm, double &std2d_deg, double &std2d_mm, const string &stdoutput = ""){
    mean_deg   = 0.0;
    mean_mm    = 0.0;
    mean2d_deg = 0.0;
    mean2d_mm  = 0.0;
    std_deg    = 0.0;
    std_mm     = 0.0;
    std2d_deg  = 0.0;
    std2d_mm   = 0.0;
  
    if( list_real.size() == list_estimated.size() && list_real.size() > 0 ){
      vector<double> mean_deg_vec, mean_mm_vec, mean2d_deg_vec, mean2d_mm_vec;

      //create stadistics file
      ofstream stdfile;
      if(stdoutput != "" ){
        stdfile.open(stdoutput, ios::out );
        if( !stdfile.is_open() ){
          string err_msg = "Not posible to create stadistics output: \n";
          err_msg += " - Stadistics file:      " + stdoutput    + "\n";
          cerr << err_msg;
          _logFileWriter->writeError(err_msg);
          _logFileWriter->write("Application finish with errors");
        }else{
          //write the header
          stdfile << "mean_deg, " << "mean_mm," << "mean2d_deg," << "mean2d_mm" << endl;
        }
      }


      for( size_t idx=0; idx<list_real.size(); idx++ ){
        double tmp_mean_deg   = geometry::angularDiff(list_real[idx], list_estimated[idx]);
        double tmp_mean_mm    = geometry::cartesianDiff(list_real[idx], list_estimated[idx]);
        double tmp_mean2d_deg = geometry::angular2dDiff(list_real[idx], list_estimated[idx]);
        double tmp_mean2d_mm  = geometry::cartesian2dDiff(list_real[idx], list_estimated[idx]);
        mean_deg   += tmp_mean_deg;
        mean_mm    += tmp_mean_mm;
        mean2d_deg += tmp_mean2d_deg;
        mean2d_mm  += tmp_mean2d_mm;        
         _logFileWriter->writeDebug(
                                    "\n  P1: " + utils::double2str(list_real[idx].x) + ", " +
                                    utils::double2str(list_real[idx].y) + ", " +
                                    utils::double2str(list_real[idx].z) + "]\n" +
                                    "  P2: " + utils::double2str(list_estimated[idx].x) + ", " +
                                    utils::double2str(list_estimated[idx].y) + ", " +
                                    utils::double2str(list_estimated[idx].z) + "]\n" +
                                    "  ErrorDeg:   " + utils::double2str(tmp_mean_deg) + '\n' +
                                    "  ErrorMM:    " + utils::double2str(tmp_mean_mm) + '\n' +
                                    "  Error2dDeg: " + utils::double2str(tmp_mean2d_deg) + '\n' +
                                    "  Error2dMM:  " + utils::double2str(tmp_mean2d_mm) );

        if( stdfile.is_open() ){
          stdfile << tmp_mean_deg << ", " << tmp_mean_mm << ", " << tmp_mean2d_deg << ", " << tmp_mean2d_mm << endl;
        }
        mean_deg_vec.push_back(tmp_mean_deg);
        mean_mm_vec.push_back(tmp_mean_mm);
        mean2d_deg_vec.push_back(tmp_mean2d_deg);
        mean2d_mm_vec.push_back(tmp_mean2d_mm);
      }

      mean_deg /= list_real.size();
      mean_mm  /= list_real.size();
      mean2d_deg /= list_real.size();
      mean2d_mm  /= list_real.size();

      double accum;
      accum = 0.0;
      std::for_each (std::begin(mean_deg_vec), std::end(mean_deg_vec), [&](const double d) { accum += pow((d - mean_deg),2); });
      if( mean_deg_vec.size() > 0 ) std_deg = sqrt(accum / (mean_deg_vec.size()));
      accum = 0.0;
      std::for_each (std::begin(mean_mm_vec), std::end(mean_mm_vec), [&](const double d) { accum += pow((d - mean_mm),2); });
      if( mean_mm_vec.size() > 0 ) std_mm = sqrt(accum / (mean_mm_vec.size()));
      accum = 0.0;
      std::for_each (std::begin(mean2d_deg_vec), std::end(mean2d_deg_vec), [&](const double d) { accum += pow((d - mean2d_deg),2); });
      if( mean2d_deg_vec.size() > 0 ) std2d_deg = sqrt(accum / (mean2d_deg_vec.size()));
      accum = 0.0;
      std::for_each (std::begin(mean2d_mm_vec), std::end(mean2d_mm_vec), [&](const double d) { accum += pow((d - mean2d_mm),2); });
      if( mean2d_mm_vec.size() > 0 ) std2d_mm = sqrt(accum / (mean2d_mm_vec.size()));

      if( stdfile.is_open() ) stdfile.close();
    }
  }

};

#endif /* _CALIBRATION_HPP_ */
