#include <vector>
#include <iostream>

#include "LogFileWriter.hpp"
#include "geometry.hpp"

using namespace std;

cv::Point3d geometry::calcAverage(std::vector<cv::Point3d> points) {
    int amount = points.size();

    if (amount == 0) { return cv::Point3d(0,0,0); }

    int sumX = 0;
    int sumY = 0;
    int sumZ = 0;
    for (std::vector<cv::Point3d>::iterator it = points.begin(); it != points.end();
            ++it) {
        sumX += it->x;
        sumY += it->y;
        sumZ += it->z;
    }

    int x = sumX / amount;
    int y = sumY / amount;
    int z = sumZ / amount;

    return cv::Point3f(x, y, z);
}

cv::Point2d geometry::calcAverage(std::vector<cv::Point2d> points) {
    int amount = points.size();

    if (amount == 0) { return cv::Point2d(0,0); }

    int sumX = 0;
    int sumY = 0;
    for (std::vector<cv::Point2d>::iterator it = points.begin(); it != points.end();
            ++it) {
        sumX += it->x;
        sumY += it->y;
    }
    int x = sumX / amount;
    int y = sumY / amount;

    return cv::Point2f(x, y);
}



cv::Point3d geometry::intersectPoint(const cv::Point3d &u, const cv::Point3d &v){

  cv::Point3d w = u - v;

  float    a = u.dot(u);         // always >= 0
  float    b = u.dot(v);
  float    c = v.dot(v);         // always >= 0
  float    d = u.dot(w);
  float    e = v.dot(w);
  float    D = a*c - b*b;        // always >= 0
  float    sc, tc;

  // compute the line parameters of the two closest points
  if (D < SMALL_NUM) {          // the lines are almost parallel
    sc = 0.0;
    tc = (b>c ? d/b : e/c);    // use the largest denominator
  }
  else {
    sc = (b*e - c*d) / D;
    tc = (a*e - b*d) / D;
  }

  // get the midpoint of the two closest points
  return (w + (sc * u) - (tc * v)) * 0.5;  // =  (L1(sc) - L2(tc)) / 2
}

cv::Point3d geometry::get_midpoint_of2vectors(const cv::Point3d &eyeR, const cv::Point3d &gazeR,
                                                   const cv::Point3d &eyeL, const cv::Point3d &gazeL){
    cv::Point3d norm, MR, ML, mid_point;
    //std::cout << eyeR << " " << gazeR << " " << eyeL << " " << gazeL << std::endl;
    norm.x = gazeL.y * gazeR.z - gazeL.z * gazeR.y;    //cross product
    norm.y = gazeL.z * gazeR.x - gazeL.x * gazeR.z;
    norm.z = gazeL.x * gazeR.y - gazeL.y * gazeR.x;
    //std:: cout << norm << std::endl;
    double temp;
    temp = -( (eyeR.x-eyeL.x)*(gazeL.y*norm.z-gazeL.z*norm.y)+
              (eyeR.y-eyeL.y)*(gazeL.z*norm.x-gazeL.x*norm.z)+
              (eyeR.z-eyeL.z)*(gazeL.x*norm.y-gazeL.y*norm.x) )/
            ( (gazeR.x*(gazeL.y*norm.z-gazeL.z*norm.y))+
              (gazeR.y*(gazeL.z*norm.x-gazeL.x*norm.z))+
              (gazeR.z*(gazeL.x*norm.y-gazeL.y*norm.x)) );
    MR.x = gazeR.x * temp + eyeR.x;
    MR.y = gazeR.y * temp + eyeR.y;
    MR.z = gazeR.z * temp + eyeR.z;
    //std:: cout << MR << std::endl;
    temp = -( (eyeL.x-eyeR.x)*(gazeR.y*norm.z-gazeR.z*norm.y)+
              (eyeL.y-eyeR.y)*(gazeR.z*norm.x-gazeR.x*norm.z)+
              (eyeL.z-eyeR.z)*(gazeR.x*norm.y-gazeR.y*norm.x) )/
            ( (gazeL.x*(gazeR.y*norm.z-gazeR.z*norm.y))+
              (gazeL.y*(gazeR.z*norm.x-gazeR.x*norm.z))+
              (gazeL.z*(gazeR.x*norm.y-gazeR.y*norm.x)) );
    ML.x = gazeL.x * temp + eyeL.x;
    ML.y = gazeL.y * temp + eyeL.y;
    ML.z = gazeL.z * temp + eyeL.z;
    //std:: cout << ML << std::endl << std::endl;
    mid_point.x = (MR.x + ML.x)/2;
    mid_point.y = (MR.y + ML.y)/2;
    mid_point.z = (MR.z + ML.z)/2;
    return mid_point;
}


double geometry::angularDiff(const cv::Point3d &p1, const cv::Point3d &p2){
  double dot = p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
  double lenSq1 = p1.x*p1.x + p1.y*p1.y + p1.z*p1.z;
  double lenSq2 = p2.x*p2.x + p2.y*p2.y + p2.z*p2.z;
  return acos(dot/sqrt(lenSq1 * lenSq2)) * RAD_TO_DEG;
}

double geometry::angular2dDiff(const cv::Point3d &p1, const cv::Point3d &p2){
  double dot = p1.x*p2.x + p1.y*p2.y;
  double det = p1.x*p2.y + p1.y*p2.x;
  return atan2(det, dot) * RAD_TO_DEG;
}

double geometry::cartesianDiff(const cv::Point3d &p1, const cv::Point3d &p2){
  // cout << "Point1: " << p1;
  // cout << " Point2: " << p2;
  // cout << " PX: " << pow((p2.x-p1.x),2);
  // cout << " PY: " << pow((p2.y-p1.y),2);
  // cout << " PZ: " << pow((p2.z-p1.z),2);
  // cout << " OUT: " <<  sqrt( pow((p2.x-p1.x),2) + pow((p2.y-p1.y),2) + pow((p2.z-p1.z),2) ) << endl;
  return sqrt( pow((p2.x-p1.x),2) + pow((p2.y-p1.y),2) + pow((p2.z-p1.z),2) );
}

double geometry::cartesian2dDiff(const cv::Point3d &p1, const cv::Point3d &p2){
  return sqrt( pow((p2.x-p1.x),2) + pow((p2.y-p1.y),2) );
}

double geometry::cartesian2dDiff(const cv::Point2d &p1, const cv::Point2d &p2){
  return sqrt( pow((p2.x-p1.x),2) + pow((p2.y-p1.y),2) );
}
