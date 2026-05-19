#!/bin/sh 

E_BADARGS=65
E_NOFILE=66

if [ $# -ne 2 ]
then
  echo "Usage: `basename $0` input-dir output-dir"
  exit $E_BADARGS
fi

echo "Creating output directory: $2"
mkdir -p $2
#FIXME CHECK ERROR

echo "===== CONVERTING..." 
for tif in $1/*.tif 
do 
  jpg=$2/`basename ${tif%%.tif}.jpg` 
  echo "Generating: $jpg"
  gdal_translate -of JPEG -co "QUALITY=85" -co "WORLDFILE=yes" $tif $jpg
done
for tif in $1/*.TIF 
do 
  jpg=$2/`basename ${tif%%.TIF}.jpg` 
  echo "Generating: $jpg"
  gdal_translate -of JPEG -co "QUALITY=85" -co "WORLDFILE=yes" $tif $jpg
done

echo "===== DONE."

exit 0