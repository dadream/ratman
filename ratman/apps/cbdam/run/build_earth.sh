#! /bin/sh

###########

CBDAM_SUBSAMPLING=3
CBDAM_INPUT_HEIGHT_FIELD=/data1/cbdam/img/srtm/world_3arcsec/
FILE_TYPE=hgt
CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE=3
CBDAM_INPUT_HEIGHT_FIELD_PLANET_RADIUS=6378000
TILE_WIDTH=1201
TILE_HEIGHT=1201
FILE_TYPE=hgt
X_TILE_COUNT=360
Y_TILE_COUNT=180
CACHE_SIZE=64

CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE=50
CBDAM_OUTPUT_HEIGHT_FIELD="/data1/cbdam/data/new_height_earth_3arcsec_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE"_sub"$CBDAM_SUBSAMPLING
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD"_"`date '+%F'`".log"

echo "================================================="
echo "STARTING HEIGHT" $CBDAM_INPUT_HEIGHT_FIELD
echo "================================================="

../apps/builder/cbdam_builder \
    --subsampling $CBDAM_SUBSAMPLING \
    --spherical-latlon \
    --spherical-planet-radius $CBDAM_INPUT_HEIGHT_FIELD_PLANET_RADIUS \
    --height-scale $CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE \
    --tolerance $CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE \
    --output-file $CBDAM_OUTPUT_HEIGHT_FIELD \
    --tiled \
    --x-tile-count $X_TILE_COUNT \
    --y-tile-count $Y_TILE_COUNT \
    --one-row-overlap \
    --cache-size $CACHE_SIZE \
    --file-type $FILE_TYPE \
    --tile-width $TILE_WIDTH \
    --tile-height $TILE_HEIGHT \
    --swap-bytes \
    $CBDAM_INPUT_HEIGHT_FIELD 2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

echo "================================================="
echo "FINISHED"
echo "================================================="
