#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include <vector>
#include "opencv2/core/core.hpp"

#define POW2(X)	  ((X)*(X))
#define DOT(U,V)  ((U).x * (V).x + (U).y * (V).y + (U).z * (V).z)
#define NORM(V)   sqrt(DOT(V,V))           // norm = length of  vector
#define DIST(U,V) norm(U-V)                // distance = norm of difference
#define ABS(X)    ((X) >= 0 ? (X) : -(X))  //  absolute value

static const double PI          = 3.14159265358979323846;
static const double PI2         = 6.28318530717958647692;
static const double RAD_TO_DEG  = 180.0 / PI;
static const double DEG_TO_RAD  = PI / 180.0;
static const double SMALL_NUM   = 0.00000001; // anything that avoids division overflow


namespace geometry{

  /**
   * calculates the average point of list of points. <br /> the average is 
   * calculated using x=sum_of_all_x/num_of_points, y=sum_of_all_y/num_of_points, 
   * z=sum_of_all_z/num_of_points.
   * @param points
   * @return Point if the average is calculated, Point(0,0,0) if could not be calculated 
   */
  cv::Point3d calcAverage(std::vector<cv::Point3d> points);

  /**
   * calculates the average point of list of points. <br /> the average is 
   * calculated using x=sum_of_all_x/num_of_points, y=sum_of_all_y/num_of_points, 
   * z=sum_of_all_z/num_of_points.
   * @param points
   * @return Point if the average is calculated, Point(0,0) if could not be calculated 
   */
  cv::Point2d calcAverage(std::vector<cv::Point2d> points);

  /** 
   * calculates midpoint of the shortest distance between 2 rays in 3D.
   * Check http://geomalgorithms.com/a07-_distance.html for more info
   * 
   * @param u vector in 3d space to intersect
   * @param v vector in 3d space to intersect
   * 
   * @return the midpoint of intersection. Point 0 otherwise.
   */
  cv::Point3d intersectPoint(const cv::Point3d &u, const cv::Point3d &v);

  /** 
   * calculates the cartesian difference distance between two points.
   * 
   * @param p1 point 1
   * @param p2 point 2
   * 
   * @return distance in milimiters
   */
  double cartesianDiff(const cv::Point3d &p1, const cv::Point3d &p2);

  /** 
   * calculates the cartesian difference distance between two points.
   * 
   * @param p1 point 1
   * @param p2 point 2
   * 
   * @return distance in milimiters
   */
  double cartesian2dDiff(const cv::Point3d &p1, const cv::Point3d &p2);


  /** 
   * calculates the cartesian difference distance between two points.
   * 
   * @param p1 point 1
   * @param p2 point 2
   * 
   * @return distance in milimiters
   */
  double cartesian2dDiff(const cv::Point2d &p1, const cv::Point2d &p2);

  /** 
   * calculates the angular difference distance between two points.
   * Check: http://stackoverflow.com/questions/14066933/direct-way-of-computing-clockwise-angle-between-2-vectors
   * @param p1 point 1
   * @param p2 point 2
   * 
   * @return distance in degrees
   */
  double angularDiff(const cv::Point3d &p1, const cv::Point3d &p2);

  /** 
   * calculates the angular difference distance between two points.
   * Check: http://stackoverflow.com/questions/14066933/direct-way-of-computing-clockwise-angle-between-2-vectors
   * @param p1 point 1
   * @param p2 point 2
   * 
   * @return distance in degrees
   */
  double angular2dDiff(const cv::Point3d &p1, const cv::Point3d &p2);

  /**
   * calculate the midpoint based on two gaze vectors.
   * @param two eye center's positions of right and left eye respectively.
   * @param two gaze vectors of corresponding two eye center's positions of right and left eye respectively.
   * @return the midpoint.
   */
  cv::Point3d get_midpoint_of2vectors(const cv::Point3d &eyeR, const cv::Point3d &gazeR,
                                      const cv::Point3d &eyeL, const cv::Point3d &gazeL);


};

#endif /* GEOMETRY_HPP_ */
