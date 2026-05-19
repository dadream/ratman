#!/bin/bash
if [ $# -ne 3 ]
    then
    echo "Usage: geodetect.sh <regione> <long_wgs84> <lat_wgs84>"
    exit 1
fi

regione=$1
long=$2
lat=$3

xy=`echo $long $lat | cs2cs +init=epsg:4326 +to +init=epsg:3004 | awk '{ print $1 " " $2 }'`

for i in nodo1 nodo2 nodo3 nodo4 
  do 
  for j in d1 d2 d3 d4
    do 
    for k in `find /usr/$i/$j/$regione -name *.tif`
      do ./vic_geodetect $k $xy
    done
  done
done
