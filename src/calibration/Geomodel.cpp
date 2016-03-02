#include "calibration/Geomodel.hpp"
#include "calibration/Cornea.hpp"
#include "utils.hpp"
#include "geometry.hpp"


Geomodel::Geomodel(LogFileWriter *logFileWriter) : Calibration(logFileWriter) {
  // _led[EYE_RIGHT][0].setLabel(0);
  // _led[EYE_RIGHT][0].setPos(cv::Point3d(18, 18, 20));
  // _led[EYE_RIGHT][0].setLabel(1);
  // _led[EYE_RIGHT][1].setPos(cv::Point3d(22, 22, 20));
  // _led[EYE_RIGHT][0].setLabel(0);
  // _led[EYE_LEFT][0].setPos(cv::Point3d(18, 18, 20));
  // _led[EYE_RIGHT][0].setLabel(1);
  // _led[EYE_LEFT][1].setPos(cv::Point3d(22, 22, 20));

  _num_leds[EYE_LEFT]  = 0;
  _num_leds[EYE_RIGHT] = 0;
  _leds[EYE_LEFT].clear();
  _leds[EYE_RIGHT].clear();
  _cornea_center[EYE_RIGHT] = cv::Point3d(0,0,0);
  _cornea_center[EYE_LEFT]  = cv::Point3d(0,0,0);

  _weights[EYE_RIGHT] = 0; 
  _weights[EYE_LEFT]  = 0; 
};



bool Geomodel::associateGlints(uint eye, const std::vector<cv::Point> &glints,
                               const std::vector<cv::Point3d> &glints_3d) {

  if( eye > EYE_MAX ){ return false; }

  const size_t sz = glints.size();

  if( sz != _num_leds[eye] ) { return false; }

  //TODO: Implement glint association
  for(size_t idx=0; idx<sz; idx++){
    _leds[eye][idx].setGlint2D(glints[idx]);
    _leds[eye][idx].setGlintDirection3D(glints_3d[idx]);
    _leds[eye][idx].setLabel(idx);
  }
 
  return true;
}


void Geomodel::getCorneaCentre(uint eye, cv::Point3d &center){
  Cornea cornea;
  std::vector<double> guesses;
  std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > glint_pos;
  std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > led_pos;

  if( eye > EYE_MAX ){ return; }

  for(size_t i = 0; i < _num_leds[eye]; ++i) {

    const cv::Point3d *gl = (_leds[eye][i]).getGlintDirection3D();
    if(gl == NULL) { continue; }

    guesses.push_back(0);
    Eigen::Vector3d eigGlint(gl->x, gl->y, gl->z);
    glint_pos.push_back(eigGlint);

    Eigen::Vector3d eigLED(_leds[eye][i].pos().x, _leds[eye][i].pos().y, _leds[eye][i].pos().z);
    led_pos.push_back(eigLED);

  }

  double error;
  Eigen::Vector3d eigCentre;
  cornea.computeCentre(led_pos, glint_pos, guesses, eigCentre, error);

  center.x = eigCentre(0);
  center.y = eigCentre(1);
  center.z = eigCentre(2);
}

bool Geomodel::calibrate(){
  _logFileWriter->writeDebug("bool Geomodel::calibrate(int accuracyThreshold, int maxExceedence)");

  //Calculate Corneal center from reflections using Hennessey et al.'s method
  //and calculate average value from all the frames

  //for each eye
  for( uint idx_eye=0; idx_eye<2; idx_eye++ ){

    if( _num_leds[idx_eye] < MIN_GLINTS ){
      // can not calculate the cornea center accurately with less than 2 glint points
      _logFileWriter->writeWarn("Cannot calculate cornea center with less than 2 glint points");
      return false;
    }

    vector<cv::Point3d> centers(_eye_features[idx_eye].size());

    //for each frame
    for( size_t idx_frame=0; idx_frame<_eye_features[idx_eye].size(); idx_frame++ ){


      std::vector<cv::Point3d> glint_3d(_num_leds[idx_eye]);
      std::vector<cv::Point>   glint_2d(_num_leds[idx_eye]);

      //Calculate Corneal center from reflections using Hennessey et al.'s method

      // if this frame has a glint
      if( _eye_features[idx_eye][idx_frame].corneal_reflections.size() >= _num_leds[idx_eye] ){
        //for each led
        for( uint idx_led=0; idx_led<_num_leds[idx_eye]; idx_led++ ){

          glint_2d[idx_led] = _eye_features[idx_eye][idx_frame].corneal_reflections[idx_led];

          const double x = glint_2d[idx_led].x;
          const double y = glint_2d[idx_led].y;

          //get the position of the led in world coordinates
          _cameras[idx_eye].pixToWorld(x, y, glint_3d[idx_led]);
        
          if(glint_3d[idx_led].z <= 0) {	// must be in front of the camera
            string warning = "Glints Must be in front of the Camera. pixToWorld() failed:\n ";
            warning += "-2d(" + utils::double2str(x) + "," + utils::double2str(y);
            warning += ")\n -3d(" + utils::double2str(glint_3d[idx_led].x);
            warning += "," + utils::double2str(glint_3d[idx_led].y);
            warning += "," + utils::double2str(glint_3d[idx_led].z) + ")";
            return false;
          }
        } // end for each led
       
        //associate each glint in the image with a led (ligth source)
        if( !associateGlints(idx_eye, glint_2d, glint_3d) ){
          _logFileWriter->writeWarn("Impossible to associate glint to led");
          return false;
        }

        //calculate corneal center for this eye and frame
        getCorneaCentre(idx_eye, centers[idx_frame]);

      } //end if has a glint

    } // end for each frame

    _cornea_center[idx_eye] = geometry::calcAverage(centers);


    size_t idx_frame   = 0;
    size_t idx_target  = 0;
    size_t num_targets = _calib_targets.size();
    vector<float> distances(_num_frames);
    float sum_distance = 0;

    // Compute distance to each of the calibration points
    while( idx_target < num_targets && idx_frame < _num_frames ){

      //calibration point belongs to this frame?
      if( _calib_targets[idx_target].frame <= idx_frame ){
        //get the proyection of the pupil in 3d world
        cv::Point2d pupil2d = (_eye_features[idx_eye])[idx_frame].pupil_center;
        cv::Point3d pupil3d;
        _cameras[idx_eye].pixToWorld(pupil2d.x, pupil2d.y, pupil3d);

        //get the calibration target point
        cv::Point3d calib3d = cv::Point3d(_calib_targets[idx_target].pos_x, _calib_targets[idx_target].pos_y, _calib_targets[idx_target].pos_z );

        //calculate optical and visual axis
        cv::Point3d oa = pupil3d - _cornea_center[idx_eye];
        cv::Point3d va = calib3d - _cornea_center[idx_eye];

        //calculate distance for this frame
        float distance = DIST( oa, va );
        distances.push_back( distance );
        sum_distance += distance;
        idx_frame++;
      }else{
        //check next calibration point
        idx_target++;
      }
    }

    // Compute weights as reciprocal of distance and normalize so they
    // sum to one
    _weights[idx_eye] = 0;
    for(size_t idx_dist; idx_dist< distances.size(); idx_dist++){
      _weights[idx_eye] += 1 / ( distances[idx_dist] * sum_distance );
    }

  } // end for each eye

  return true;
}

cv::Point3d Geomodel::calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L){
  _logFileWriter->writeDebug("cv::Point3d Geomodel::calcCoordinates(const cv::Point &pupil_R, const cv::Point &pupil_L)");

  //calculate optical calibrated vector for each eye

  //get the proyection of the pupil in 3d world
  cv::Point3d pupil3d_R, pupil3d_L;
  _cameras[EYE_RIGHT].pixToWorld(pupil_R.x, pupil_R.y, pupil3d_R);
  _cameras[EYE_LEFT].pixToWorld(pupil_L.x, pupil_L.y, pupil3d_L);

  //calculate visual axis
  cv::Point3d va_R = pupil3d_R - _cornea_center[EYE_RIGHT];
  cv::Point3d va_L = pupil3d_L - _cornea_center[EYE_LEFT];

  //apply weight correction
  va_R.x += _weights[EYE_RIGHT];
  va_R.y += _weights[EYE_RIGHT];
  va_R.z += _weights[EYE_RIGHT];
  va_L.x += _weights[EYE_LEFT];
  va_L.y += _weights[EYE_LEFT];
  va_L.z += _weights[EYE_LEFT];

  //calculate vergence intersection
  return geometry::intersectPoint(va_R, va_L);
}

