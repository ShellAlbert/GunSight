# GunSight
# There is something wonderful that we should remember.
  git clone xxxxx.git   
  modify/git add file   
  git add --all      
  git commit -a   
  git push origin master


./configure -prefix /home/zhangshaoyan/MyWork/experiment/qt5110forarm64  \
-no-dbus \
-eventfd -inotify \
-qt-sqlite \
-qt-libjpeg \
-qt-libpng \
-evdev -mtdev \
-linuxfb \
-qpa linuxfb \
-no-opengl \
-qt-harfbuzz \
-qt-freetype \
-no-openssl \
-qt-zlib \
-qt-pcre \
-nomake examples -nomake tests -skip qtandroidextras -skip qtquickcontrols -skip qtquickcontrols2 -skip qtmacextras -skip qtscript  -skip qtwinextras \
-qreal float \
-xplatform linux-aarch64-gnu-g++ \
-shared \
-strip \
-release \
-confirm-license \
-opensource  \
-no-gstreamer  -no-iconv
#-no-gstreamer -no-assimp -no-qt3d-profile-jobs  -no-qt3d-profile-gl


