cs2cs +init=epsg:4326 +to +init=epsg:3004 FILE | awk '{ print $1 " " $2 }' > FILE_OUT

