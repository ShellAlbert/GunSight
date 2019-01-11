#-------------------------------------------------
#
# Project created by QtCreator 2018-12-24T14:58:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GunSightV2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        zmainui.cpp \
    zgblhelp.cpp \
    zcapthread.cpp \
    zringbuffer.cpp \
    ztrackthread.cpp \
    zkeydetthread.cpp \
    CSK_Tracker.cpp

HEADERS += \
        zmainui.h \
    zgblhelp.h \
    zcapthread.h \
    zringbuffer.h \
    ztrackthread.h \
    zkeydetthread.h \
    CSK_Tracker.h

INCLUDEPATH += /opt/EmbedSky/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/include

#opencv
INCLUDEPATH += /home/zhangshaoyan/GunScan/opencvlaste/install32/include/opencv4
LIBS += -L/home/zhangshaoyan/GunScan/opencvlaste/install32/lib
LIBS += -lopencv_aruco
LIBS += -lopencv_dnn
LIBS += -lopencv_highgui
LIBS += -lopencv_phase_unwrapping
LIBS += -lopencv_stitching
LIBS += -lopencv_videostab
LIBS += -lopencv_bgsegm
LIBS += -lopencv_dpm
LIBS += -lopencv_imgcodecs
LIBS += -lopencv_photo
LIBS += -lopencv_structured_light
LIBS += -lopencv_xfeatures2d
LIBS += -lopencv_bioinspired
LIBS += -lopencv_face
LIBS += -lopencv_img_hash
LIBS += -lopencv_plot
LIBS += -lopencv_superres
LIBS += -lopencv_ximgproc
LIBS += -lopencv_calib3d
LIBS += -lopencv_features2d
LIBS += -lopencv_imgproc
LIBS += -lopencv_reg
LIBS += -lopencv_surface_matching
LIBS += -lopencv_xobjdetect
LIBS += -lopencv_ccalib
LIBS += -lopencv_flann
LIBS += -lopencv_line_descriptor
LIBS += -lopencv_rgbd
LIBS += -lopencv_text
LIBS += -lopencv_xphoto
LIBS += -lopencv_core
LIBS += -lopencv_fuzzy
LIBS += -lopencv_ml
LIBS += -lopencv_saliency
LIBS += -lopencv_tracking
LIBS += -lopencv_datasets
LIBS += -lopencv_gapi
LIBS += -lopencv_objdetect
LIBS += -lopencv_shape
LIBS += -lopencv_videoio
LIBS += -lopencv_dnn_objdetect
LIBS += -lopencv_hfs
LIBS += -lopencv_optflow
LIBS += -lopencv_stereo
LIBS += -lopencv_video

QMAKE_LFLAGS += -Wl,-rpath-link=/home/zhangshaoyan/armbuild/copyfromrk3399ubuntu1804/arm-linux-gnueabihf
LIBS += -L/home/zhangshaoyan/armbuild/copyfromrk3399ubuntu1804/arm-linux-gnueabihf -lpthread

