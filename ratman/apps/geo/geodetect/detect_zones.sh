#!/bin/bash
if [ $# -ne 2 ]
    then
    echo "Usage: detect_zones <dir_base> <coord_file>"
    exit 1
fi

DIR_BASE=$1
COORD_FILE=$2

VIC_GEODETECT=/usr/vic/bin/vic_geodetect

exec 3<$COORD_FILE

while read -u 3 line
do
  for i in `find $DIR_BASE -name "*.tif" -o -name "*.TIF"`
  do 
    $VIC_GEODETECT $i $line
  done
done

exec 3<&-
