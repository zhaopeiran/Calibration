    message( "Gaze Tracker command line" )
    unix:!macx:message( "Linux" )
    TEMPLATE = app
    BINDIR  = "../bin"
    win32::DESTDIR = "../bin"
    INCDIR  = "../inc"
    SRCDIR  = "../src"

    # EXTERNAL LIBRARIES
    # OpenCV
    unix:OPENCV_LIB  = "/usr/include/opencv2"
    # GNU Scientific Library
    unix:GSL_LIB  = "/usr/include" 
    macx:GSL_LIB  = "opt/local/include"
    
    macx:CONFIG -=app_bundle
    CONFIG += debug
    CONFIG += static
    CONFIG(release, debug|release) {
        message( "Release" )
        TARGET = $$BINDIR/gazetracker_cmd
        DEFINES += RELEASE_MODE
    }
    CONFIG(debug, debug|release) {
        message( "Debug" )
        TARGET = $$BINDIR/gazetracker_cmd_d
        unix:QMAKE_CXXFLAGS += -g
        DEFINES += DEBUG_MODE
    }
    
    OBJECTS_DIR = obj
    MOC_DIR     = moc
    RCC_DIR     = qrc
    UI_DIR      = ui
    
    unix:QMAKE_CXXFLAGS += -std=c++11 -Wno-parentheses
    LIBS += -lopencv_core
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_highgui
    LIBS += -lopencv_video
    LIBS += -lopencv_calib3d
    # LIBS += -lopencv_ml
    # LIBS += -lopencv_features2d
    # LIBS += -lopencv_objdetect
    # LIBS += -lopencv_contrib
    # LIBS += -lopencv_legacy
    # LIBS += -lopencv_flann
    LIBS += -lgsl -lgslcblas
    
    DEPENDPATH += \
    "$$SRCDIR" \
    "$$INCDIR"
    INCLUDEPATH += \
    "$$INCDIR" \
    "$OPENCV_LIB" \
    "GSL_LIB"
    
    # Input
    HEADERS += \
    "$$INCDIR/geometry.hpp" \
    "$$INCDIR/utils.hpp" \
    "$$INCDIR/LogFileWriter.hpp" \
    "$$INCDIR/video/ImageSource.hpp" \
    "$$INCDIR/video/LiveSource.hpp" \
    "$$INCDIR/video/VideoSource.hpp" \
    "$$INCDIR/detection/find_best_edge.hpp" \
    "$$INCDIR/detection/PUPIL_BLOB_DETECTION.hpp" \
    "$$INCDIR/detection/canny_ml.hpp" \
    "$$INCDIR/detection/algo.hpp" \
    "$$INCDIR/detection/zero_around_region_th_border.hpp" \
    "$$INCDIR/detection/th_angular_histo.hpp" \
    "$$INCDIR/detection/remove_points_with_low_angle.hpp" \
    "$$INCDIR/detection/peek.hpp" \
    "$$INCDIR/detection/optimize_pos.hpp" \
    "$$INCDIR/detection/get_curves.hpp" \
    "$$INCDIR/detection/canny_neu.hpp" \
    "$$INCDIR/detection/bw_select.hpp" \
    "$$INCDIR/detection/cornealReflection.hpp" \
    "$$INCDIR/detection/Pupil_contours_detection.hpp" \
    "$$INCDIR/calibration/Calibration.hpp" \
    "$$INCDIR/calibration/Interpolation.hpp" \
    "$$INCDIR/calibration/Geomodel.hpp" \
    "$$INCDIR/calibration/Camera.hpp" \
    "$$INCDIR/calibration/Led.hpp" \
    "$$INCDIR/calibration/Cornea.hpp"
    SOURCES += \
    "$$SRCDIR/gazetracker_cmd.cpp" \
    "$$SRCDIR/geometry.cpp" \
    "$$SRCDIR/utils.cpp" \
    "$$SRCDIR/video/LiveSource.cpp" \
    "$$SRCDIR/video/VideoSource.cpp" \
    "$$SRCDIR/calibration/Interpolation.cpp" \
    "$$SRCDIR/calibration/Geomodel.cpp" \
    "$$SRCDIR/calibration/Camera.cpp" \
    "$$SRCDIR/calibration/Cornea.cpp"
