#ifndef _GEOMODEL_HPP_
#define _GEOMODEL_HPP_

#include "calibration/Led.hpp"
#include "calibration/Calibration.hpp"


#define MIN_GLINTS 2

class Geomodel : public Calibration {

private:
  unsigned int _num_leds[2];
  vector<Led>  _leds[2]; //2 leds per eye
  cv::Point3d  _cornea_center[2];
  float        _weights[2]; //weights factors to add to gaze position

public:

  Geomodel(LogFileWriter *logFileWriter);

  inline ~Geomodel() {};

  /** 
   * Add a led to the environment
   * 
   * @param eye      right eye #EYE_RIGHT or left #EYE_LEFT
   * @param position Physical position of the LEDs in the eye camera coordinate system
   */
  inline void addLed(unsigned int eye, cv::Point3d position){ 
    if( eye > EYE_MAX ){ return; }
    else{
      _leds[eye].push_back(Led(_leds[eye].size(),position));
      _num_leds[eye]++;
    }
  }


  bool        calibrate();
  cv::Point3d calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L);

  friend ostream &operator<<( ostream &output, const Geomodel &model ){
    output << endl;
    for(unsigned int idx_eye=0; idx_eye<2; idx_eye++){
      for (size_t idx_led = 0; idx_led < model._leds[idx_eye].size(); idx_led++) {
        output << "Led " << GET_EYENAME(idx_eye) << " " << endl << (model._leds[idx_eye])[idx_led] << endl; 
      }
      output << "Cornea Center " << GET_EYENAME(idx_eye) << " " << endl << model._cornea_center[idx_eye] << endl;
      output  << endl;
      output << "Weights " << GET_EYENAME(idx_eye) << " " << endl << model._weights[idx_eye] << endl;
      output  << endl;
    }
    return output;
  }

private:

  bool associateGlints(uint eye, const std::vector<cv::Point> &glints,
                                 const std::vector<cv::Point3d> &glints_3d);

  void getCorneaCentre(uint eye, cv::Point3d &center);
};

#endif //_GEOMODEL_HPP_
