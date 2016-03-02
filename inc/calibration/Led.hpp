#ifndef _LED_HPP_
#define _LED_HPP_

#include <opencv2/core/core.hpp>

/*
 * This is a container to hold information about:
 *
 *     1. The calibrated LED position
 *     2. Direction vector towards the glint
 *     3. The glint in the image
 *     4. The label to tell which LED this is in the six-LED configuration
 */
class Led {
public:

  /*
   * Default Constructor
   */
  Led(): _nLabel(-1) {}

  /*
   * Constructor
   */
  Led(uint nlabel, const cv::Point3d &pos): 
    _nLabel(nlabel), _posLED3D(pos)  {};

  /*
   * Accessors
   */
  const cv::Point3d &pos() const{ return _posLED3D; }
  const cv::Point3d *getGlintDirection3D() const{
    if(_directionGlint3D.x == 0 &&
       _directionGlint3D.y == 0 &&
       _directionGlint3D.z == 0) {
      return NULL;
    }
    return &_directionGlint3D;
  }
  const cv::Point2d &getGlint2D() const {return _posGlint2D;}
  uint getLabel() const {return _nLabel;}

  /*
   * Set parameters
   */
  void setPos(const cv::Point3d &pos) {_posLED3D = pos;}
  void setGlintDirection3D(const cv::Point3d &pos) {_directionGlint3D = pos;}
  void setGlint2D(const cv::Point2d &pos) {_posGlint2D = pos;}
  void setLabel(uint nLabel) {_nLabel = nLabel;}


  friend std::ostream &operator<<( std::ostream &output, const Led &led ){
    output << "- Label: " << led._nLabel << std::endl;
    output << "- Position Led   3d: " << led._posLED3D << std::endl;
    output << "- Dir Glint 3d: " << led._directionGlint3D << std::endl;
    output << "- Pos Glint 2d: " << led._posGlint2D << std::endl;
    return output;
  }


private:

  /*
   * Label to map the glint to the LED int the n-LED camera configuration.
   * This is -1 when unidentified.
   */
  uint _nLabel;

  /*
   * Physical position of the LED in the eye camera coordinate system
   */
  cv::Point3d _posLED3D;

  /*
   * Direction vector towards the glint
   */
  cv::Point3d _directionGlint3D;

  /*
   * Position of the glint, obtained from PupilTracker
   */
  cv::Point2d _posGlint2D;

};

#endif // _LED_HPP_
