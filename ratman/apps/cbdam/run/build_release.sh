#!/bin/sh

# Script for creating an external_release for cbdam module.
#
# 1. execute make to update and check cbdam module integrity.
# 2. delete previous external_release
# 3. create dir structure
# 4. copy all requested files
# 5. execute qmake, make of  external_release
# 6. if no error: make clean
# 7. create tar file

SRC_DIR=`pwd`
DEST_DIR=release_dir
OSINFO=`uname`
if [ "$OSINFO" = "Linux" ]; then
OS_MAKE=make
else
OS_MAKE=nmake
fi

if [ 0 = 1 ]; then

exit
echo recompile cbdam module 
qmake cbdam.pro
$OS_MAKE qmake
$OS_MAKE

if [ "$?" != "0" ]; then 
    echo unable to compile cbdam
    echo build $DEST_DIR failed
    exit 1
fi

fi

echo remove $DEST_DIR
rm -rf $DEST_DIR

echo create directory tree
echo mkdir $DEST_DIR
mkdir $DEST_DIR
echo mkdir $DEST_DIR/lib
mkdir $DEST_DIR/lib
echo mkdir $DEST_DIR/extra
mkdir $DEST_DIR/extra
echo mkdir $DEST_DIR/extra/lib
mkdir $DEST_DIR/extra/lib
echo mkdir $DEST_DIR/extra/include
mkdir $DEST_DIR/extra/include
echo mkdir $DEST_DIR/opossum
mkdir $DEST_DIR/opossum
echo mkdir $DEST_DIR/builder
mkdir $DEST_DIR/builder
echo mkdir $DEST_DIR/viewer
mkdir $DEST_DIR/viewer

if [ "$OSINFO" = "Linux" ]; then
echo copy linux libs to $DEST_DIR/lib
cp /usr/local/lib/libvic_os.a $DEST_DIR/lib
cp /usr/local/lib/libvic_cbdam_base.a $DEST_DIR/lib/
cp /usr/local/lib/libvic_opossum.a $DEST_DIR/lib/
cp /usr/local/lib/libvic_opossum_buildings.a $DEST_DIR/lib/
cp /usr/local/lib/libvic_vfs.a $DEST_DIR/lib/
cp /usr/local/lib/libvic_curlstream.a $DEST_DIR/lib/
cp /usr/local/lib/libvic_xml.a $DEST_DIR/lib/
cp /usr/local/lib/libsl.a $DEST_DIR/lib/
cp /usr/lib/libshp.so $DEST_DIR/extra/lib
cp -r /usr/include/libshp/ $DEST_DIR/extra/include
cp $CURL_DIR/lib/libcurl.lib $DEST_DIR/extra/lib

#echo copy linux libs to $DEST_DIR/extra_lib
#cp /usr/local/lib/liblzo.a $DEST_DIR/extra_lib/

else
echo copy win32 libs to $DEST_DIR/lib

cp /usr/local/lib/vic_os.lib $DEST_DIR/lib
cp /usr/local/lib/vic_cbdam_base.lib $DEST_DIR/lib/
cp /usr/local/lib/vic_opossum.lib $DEST_DIR/lib/
cp /usr/local/lib/vic_opossum_buildings.lib $DEST_DIR/lib/
cp /usr/local/lib/vic_vfs.lib $DEST_DIR/lib/
cp /usr/local/lib/vic_curlstream.lib $DEST_DIR/lib/
cp /usr/local/lib/vic_xml.lib $DEST_DIR/lib/
cp /usr/local/lib/libsl.a $DEST_DIR/lib/

echo copy win32 libs to $DEST_DIR/extra_lib
mkdir $DEST_DIR/extra/include/libshp
cp $PROGRAMFILES/develop_external_packages/shapelib-1.2.10/libshp/shapelib.lib $DEST_DIR/extra/lib/
cp $PROGRAMFILES/develop_external_packages/shapelib-1.2.10/libshp/shapelib.dll $DEST_DIR/builder/
cp $PROGRAMFILES/develop_external_packages/shapelib-1.2.10/libshp/shapefil.h $DEST_DIR/extra/include/libshp/
cp $CURL_DIR/lib/libcurl.lib $DEST_DIR/extra/lib
cp $PROGRAMFILES/develop_external_packages/glew/lib/glew32.lib $DEST_DIR/extra/lib
cp $PROGRAMFILES/develop_external_packages/glut-3.7.6-bin/glut32.lib $DEST_DIR/extra/lib

fi


echo copy include files to $DEST_DIR/opossum
cp src/vic/opossum/scene_compiler.hpp $DEST_DIR/opossum
cp src/vic/opossum/scene_renderer.hpp $DEST_DIR/opossum

echo copy application files to $DEST_DIR/builder/ and $DEST_DIR/viewer/
cp apps/opossum_builder/opossum_builder.cpp $DEST_DIR/builder/
cp apps/opossum_builder/external_opossum_builder_pro $DEST_DIR/builder/builder.pro
cp apps/opossum_viewer/*.h $DEST_DIR/viewer/
cp apps/opossum_viewer/*.hpp $DEST_DIR/viewer/
cp apps/opossum_viewer/*.cpp $DEST_DIR/viewer/
cp apps/opossum_viewer/*.c $DEST_DIR/viewer/
cp apps/opossum_viewer/*.ui $DEST_DIR/viewer/
cp apps/opossum_viewer/external_opossum_viewer_pro $DEST_DIR/viewer/viewer.pro

echo build applications
cd  $DEST_DIR/builder/

if ! qmake; then
    exit 1
fi
if ! $OS_MAKE; then
    exit 1
fi
$OS_MAKE clean

cd $SRC_DIR
cd $DEST_DIR/viewer/
if ! qmake; then
    exit 1
fi
if ! $OS_MAKE; then
    exit 1
fi
$OS_MAKE clean

cd $SRC_DIR
echo remove Makefiles
rm $DEST_DIR/viewer/Makefile
rm $DEST_DIR/builder/Makefile

cp apps/readme_opossum $DEST_DIR/readme.txt

cd $SRC_DIR
echo $DEST_DIR contains files to be exported.

