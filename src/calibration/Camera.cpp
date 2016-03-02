#include "calibration/Camera.hpp"
#include <math.h>
#include <vector>

Camera::Camera() {

  _focal_length       = 0;
  _camera_center      = cv::Point2d(0,0);
  _camera_position    = cv::Point3d(0,0,0);
  _resolution[0]      = 0;
  _resolution[1]      = 0;
  _intrinsic_matrix   = cv::Mat::zeros(3, 3, CV_64FC1);
	_distortion         = cv::Mat::zeros(1, 5, CV_64FC1);
  _rotation_matrix    = cv::Mat::zeros(3, 3, CV_64FC1);
	_translation_vector = cv::Mat::zeros(1, 3, CV_64FC1);

}


void Camera::setTranslationVector(const double tvec[3]){
	_translation_vector.at<double>(0) = tvec[0];
	_translation_vector.at<double>(1) = tvec[1];
	_translation_vector.at<double>(2) = tvec[2];
}

void Camera::setRotationMatrix(const double rmat[9]){
	_rotation_matrix.at<double>(0, 0) = rmat[0];
	_rotation_matrix.at<double>(0, 1) = rmat[1];
	_rotation_matrix.at<double>(0, 2) = rmat[2];
	_rotation_matrix.at<double>(1, 0) = rmat[3];
	_rotation_matrix.at<double>(1, 1) = rmat[4];
	_rotation_matrix.at<double>(1, 2) = rmat[5];
	_rotation_matrix.at<double>(2, 0) = rmat[6];
	_rotation_matrix.at<double>(2, 1) = rmat[7];
	_rotation_matrix.at<double>(2, 2) = rmat[8];
}

/*
 * The matrix is to be given in the row-major order:
 * The first three elements should be the first row and the following
 * three elements the second row and the last three should make for
 * the last row.
 */
void Camera::setIntrinsicMatrix(const double intr[9]) {
	_intrinsic_matrix.at<double>(0, 0) = intr[0];
	_intrinsic_matrix.at<double>(0, 1) = intr[1];
	_intrinsic_matrix.at<double>(0, 2) = intr[2];
	_intrinsic_matrix.at<double>(1, 0) = intr[3];
	_intrinsic_matrix.at<double>(1, 1) = intr[4];
	_intrinsic_matrix.at<double>(1, 2) = intr[5];
	_intrinsic_matrix.at<double>(2, 0) = intr[6];
	_intrinsic_matrix.at<double>(2, 1) = intr[7];
	_intrinsic_matrix.at<double>(2, 2) = intr[8];
}

void Camera::setDistortionMatrix(const double dist[5]) {
	_distortion.at<double>(0) = dist[0];
	_distortion.at<double>(1) = dist[1];
	_distortion.at<double>(2) = dist[2];
	_distortion.at<double>(3) = dist[3];
	_distortion.at<double>(4) = dist[4];

}


void Camera::recalculateMatrix(double focal_length, const cv::Point2d &camera_center) {
  _focal_length  = focal_length;
  _camera_center = camera_center;

	double intrMat[9] = {focal_length, 0.0, camera_center.x, 0.0, focal_length, camera_center.y, 0.0, 0.0, 1.0};
	setIntrinsicMatrix(intrMat);
}


/*
 *	It is important to notice that u and v are in a 2D coordinate system with the origin at the upper
 *	left corner of the image.
 */
void Camera::pixToCam(const std::vector<cv::Point2d> &image_points, std::vector<cv::Point3d> &vec3D) const {

	if(vec3D.size() == 0) {
		vec3D.resize(image_points.size());
	}

	std::vector<cv::Point2d> dstv(image_points.size());

	// gives u' and v'
	cv::undistortPoints(image_points,		// input points
                      dstv,				// output points
                      _intrinsic_matrix,	// intrinsic camera matrix
                      _distortion,			// _distortion coefficients
                      cv::Mat(),			// rectification transformation in the object space, unused
                      cv::Mat());	// use this matrix to place u' and v' to the same coord system as u and v

  // after undistort the units are in mm

  for(size_t i = 0; i < dstv.size(); ++i) {
    // normalize
    const double x   = dstv[i].x;
    const double y   = dstv[i].y;
    const double len = std::sqrt(x*x + y*y + 1);

    cv::Point3d &curPoint = vec3D[i];
    curPoint.x = x / len;
    curPoint.y = y / len;
    curPoint.z = 1 / len;
  }

}


/*
 *	It is important to notice that u and v are in a 2D coordinate system with the origin at the upper
 *	left corner of the image.
 */
void Camera::pixToWorld(double u, double v, cv::Point3d &p3D) const {


	std::vector<cv::Point2d> image_point(1);
	image_point[0].x = u;
	image_point[0].y = v;

	std::vector<cv::Point3d> object_point(1);
	pixToCam(image_point, object_point);

	object_point[0];

  //Y coordinate is inverted in camera
  object_point[0].y *= -1; 

  // std::cout << "Unit2d  Before trans (mm): " << std::endl << object_point[0] << std::endl;

  //locate the point 3D
  object_point[0].x *= _camera_position.z; 
  object_point[0].y *= _camera_position.z;

  // std::cout << "Point3d  Before trans (mm): " << std::endl << object_point[0] << std::endl;

  //translate vector to world coordinates
  cv::Mat tmpMat(3, 1, cv::DataType<double>::type);
  tmpMat.at<double>(0,0) = object_point[0].x + _translation_vector.at<double>(0);
  tmpMat.at<double>(1,0) = object_point[0].y + _translation_vector.at<double>(1);
  tmpMat.at<double>(2,0) = object_point[0].z + _translation_vector.at<double>(2);

  p3D.x = tmpMat.at<double>(0,0);
  p3D.y = tmpMat.at<double>(1,0);
  p3D.z = tmpMat.at<double>(2,0);

  // std::cout << "Point3d: " << std::endl << ">>>>>" << p3D << std::endl;
}



/*
 *	http://opencv.willowgarage.com/documentation/cpp/camera_calibration_and_3d_reconstruction.html
 *	It is important to notice that u and v are in a 2D coordinate system with the origin at the upper
 *	left corner of the image.
 */
void Camera::worldToPix(const cv::Point3d &p3D, double *u, double *v) const {

	// A list containing a single point
	std::vector<cv::Point3f> pointList;
	pointList.push_back(cv::Point3f(p3D.x, -p3D.y, p3D.z)); //Y coordinate is inverted

	// Get projection using opencv
	std::vector<cv::Point2f> projectedPointsList;
	cv::projectPoints(pointList, _rotation_matrix, _translation_vector, _intrinsic_matrix, _distortion, projectedPointsList);
	const cv::Point2f &displayPoint = projectedPointsList[0];

	// Write the result
	*u = displayPoint.x;
	*v = displayPoint.y;
}

