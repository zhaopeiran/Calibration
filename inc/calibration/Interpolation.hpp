#ifndef INTERPOLATION_HPP_
#define INTERPOLATION_HPP_

#include "calibration/Calibration.hpp"
#include <iostream>


class Interpolation : public Calibration {

public:

  inline Interpolation(LogFileWriter *logFileWriter) : Calibration(logFileWriter), _ismapMatCaculated(false){};

  virtual ~Interpolation();

  /**
   * caculate the coeff. Matrix
   */
  virtual bool calibrate();

  /**
   * estimate the gaze of point with the Interpolation coeff. Matrix and Vergence;
   * @param  the 2 pupil positions in camera frames (unit: px);
   * @return the gaze of point in 3d world coordinate.
   */
  virtual cv::Point3d calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L);

  /**
   * for creating the polynom's coeff.s;
   * @param  first 1 param. is the pupil pos. in 2d frame (unit: px);
   * @param  last 1 param. is the length of polynomes;
   * @return a list of the input polynomes.
   */
  vector<float> createInputFormat(const cv::Point2d &p, const int &n, const int &number);

  /**
   * print private member variable below into log files.
   */
  friend ostream &operator<<( ostream &output, const Interpolation &inter ){
    const Calibration& b(inter);

    output << b << endl;
    output << "Interpolation matrix: ";
    if( inter._ismapMatCaculated ){
        output << endl
               << "right x:" << endl <<*(inter._mapMat_x[EYE_RIGHT]) << endl
               << "right y:" << endl <<*(inter._mapMat_y[EYE_RIGHT]) << endl
               << "left x:" << endl  << *(inter._mapMat_x[EYE_LEFT]) << endl
               << "left y:" << endl  << *(inter._mapMat_y[EYE_LEFT]) << endl;
    }
    else{ output << "NOT SOLVED"; }
    output << std::endl;

    return output;
  }

  inline void setInterPupil(int interpupil){ _interpupil = interpupil; }

private:
  cv::Mat *_mapMat_x[2]; // for estimate PORx
  cv::Mat *_mapMat_y[2]; // for estimate PORy
  bool     _ismapMatCaculated; // _mapMat is free now.
  int      _interpupil;


  /**
   * calculate the estimate data with the coeff. Matrix "_mapMat", just for one line;
   * @param input pupils data;
   * @param length of input pupils data;
   * @param length of output gaze data;
   * @param whether it finished the coeff. Matrix "_mapMat" caculating;
   * @param index of eye: (0:right, 1:left);
   * @return the estimate gaze data.
   */
   cv::Point2d map(const vector<float> input_x, const vector<float> input_y, const int &idx_eye);
};

#endif
