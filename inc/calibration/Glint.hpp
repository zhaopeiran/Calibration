#ifndef _LED_HPP_
#define _LED_HPP_

/*
 * This is a container to hold information about:
 *
 *     1. The calibrated LED position
 *     2. Direction vector towards the glint
 *     3. The glint in the image
 *     4. The label to tell which LED this is in the six-LED configuration
 */
class LED {
public:

  /*
   * Constructor
   */
  LED();

  /*
   * Accessors
   */
  const cv::Point3d &pos() const;
  const cv::Point3d *getGlintDirection3D() const;
  const cv::Point2d &getGlint2D() const {return m_posGlint2D;}
  int getLabel() const {return m_nLabel;}

  /*
   * Set parameters
   */
  void setPos(const cv::Point3d &pos) {m_posLED3D = pos;}
  void setGlintDirection3D(const cv::Point3d &pos) {m_directionGlint3D = pos;}
  void setGlint2D(const cv::Point2d &pos) {m_posGlint2D = pos;}
  void setLabel(int nLabel) {m_nLabel = nLabel;}

private:

  /*
   * Physical position of the LED in the eye camera coordinate system
   */
  cv::Point3d m_posLED3D;

  /*
   * Direction vector towards the glint
   */
  cv::Point3d m_directionGlint3D;

  /*
   * Position of the glint, obtained from PupilTracker
   */
  cv::Point2d m_posGlint2D;

  /*
   * Label to map the glint to the LED int the n-LED camera configuration.
   * This is -1 when unidentified.
   */
  int m_nLabel;

};

#endif // _LED_HPP_
