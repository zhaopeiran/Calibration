#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_


/*
 *	This class uses the OpenCV right-handed coordinate system:
 *		x+ : right
 *		x- : left
 *		y+ : down
 *		y- : up
 *		z+ : towards monitor
 *		z- : towards viewer
 */


#include <string>
#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

/*	
 *	http://en.wikipedia.org/wiki/Angle_of_view
 *
 *	double fy = h_2 / tan(DEGTORAD(FOV_y) / 2.0);
 *	double fx = fy;
 */


/*
 * The camera model is a pinhole model, the center of projection is at the
 * origin of the camera coordinate system, and the camera's optical axis
 * points out along the negative z axis. The x and y axes of the image plane
 * are aligned with the x and y axes of the camera coordinate system.
 */
class Camera {

public:

  Camera();

  /* Copy contructor */
  Camera(const Camera &other) {
    _focal_length       = other._focal_length;
    _camera_center      = other._camera_center;
    _camera_position    = other._camera_position;
    _resolution[0]      = other._resolution[0];
    _resolution[1]      = other._resolution[1];
    _intrinsic_matrix   = other._intrinsic_matrix.clone();
    _distortion         = other._distortion.clone();
    _rotation_matrix    = other._rotation_matrix.clone();
    _translation_vector = other._translation_vector.clone();
  }

  /* Assignment operator */
  Camera & operator= (const Camera &other) {

    // protect against invalid self-assignment
    if(this != &other) {
      _focal_length       = other._focal_length;
      _camera_center      = other._camera_center;
      _camera_position    = other._camera_position;
      _resolution[0]      = other._resolution[0];
      _resolution[1]      = other._resolution[1];
      _intrinsic_matrix   = other._intrinsic_matrix.clone();
      _distortion         = other._distortion.clone();
      _rotation_matrix    = other._rotation_matrix.clone();
      _translation_vector = other._translation_vector.clone();
    }  
    // by convention, always return *this
    return *this;
  }
  
  friend std::ostream &operator<<( std::ostream &output, const Camera &camera ){
    output << "- Rotation Matrix: "    << std::endl << " " << camera._rotation_matrix    << std::endl;
    output << "- Translation Vector: " << std::endl << " " << camera._translation_vector << std::endl;
    output << "- Intrinsix  Matrix: "  << std::endl << " " << camera._intrinsic_matrix   << std::endl;
    output << "- Distorsion Matrix: "  << std::endl << " " << camera._distortion         << std::endl;
    output << "- Resolution: "   << camera._resolution[0] << "x" <<  camera._resolution[1] << std::endl;
    output << "- Focal Length: " << camera._focal_length    << std::endl;
    output << "- Center: "       << camera._camera_center   << std::endl;
    output << "- Position: "     << camera._camera_position << std::endl;
    return output;
  }

  void setTranslationVector(const double tvec[3]);
  void setRotationMatrix(const double rmat[9]);
  void setIntrinsicMatrix(const double intr[9]);
  void setDistortionMatrix(const double dist[5]);
  inline void setResolution( int resX, int resY){ _resolution[0] = resX; _resolution[1] = resY; }
  inline void setPosition(cv::Point3d camera_position){ _camera_position = camera_position; }

  const cv::Mat& getRotationMatrix() const {return _rotation_matrix;}
  const cv::Mat& getTranslationVector() const {return _translation_vector;}
  const cv::Mat& getIntrisicMatrix() const {return _intrinsic_matrix;}
  const cv::Mat& getDistortion() const {return _distortion;}

  void recalculateMatrix(double focal_length, const cv::Point2d &camera_center);

  /**
   * Unprojects points on the image plane back into 3D space
   *
   * @param image_points two-dimensioanl points from the image plane of the camera
   * @param vec3D The points in 3D space.
   */
  void pixToCam(const std::vector<cv::Point2d> &image_points, std::vector<cv::Point3d> &vec3D) const;
  void pixToWorld(double u, double v, cv::Point3d &p3D) const;

  void worldToPix(const cv::Point3d &p3D, double *u, double *v) const;

private:

  /*
   * 'focal_length' is the focal length of the camera in pixels. A point at 
   * a distance of 1 metre from the camera and offset horizontally from the 
   * optical axis by 1 metre will appear at an x coordinate of 
   * 'focal_length' pixels in the camera image.
   */
  int         _focal_length; //focal length in milimiters
  /*
   * Camera image center
   */
  cv::Point2d _camera_center;

  /*
   * Camera position in workd coordinates
   */
  cv::Point3d _camera_position;

  /*
   * 'resolution' is a two-dimensional vector specifying the image
   * resolution of the camera (resolution (1) is the horizontal resolution,
   * and resolution(2) is the vertical resolution). The point where the
   * optical axis intersects the image plane has the image coordinates
   * (0,0); hence, valid x-coordinates range from -resolution(1)/2 to
   * resolution(1)/2, and valid y-coordinates range from -resolution(2)/2 to
   * resolution(2)/2. Points that fall outside this range cannot be "seen"
   * by the camera.
   */
  int _resolution[2];
  /*
   * 'trans' is the transformation matrix from camera to world coordinates.
   */
  cv::Mat     _intrinsic_matrix;
  cv::Mat     _distortion;
  cv::Mat     _rotation_matrix;
  cv::Mat     _translation_vector;

};


#endif /* _CAMERA_HPP_ */
