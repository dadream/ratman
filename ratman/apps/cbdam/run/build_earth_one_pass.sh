#! /bin/sh

###########

CBDAM_SUBSAMPLING=0
CBDAM_INPUT_HEIGHT_FIELD=/scratchfs/srtm90_filtered
FILE_TYPE=hgt
CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE=1
CBDAM_INPUT_HEIGHT_FIELD_PLANET_RADIUS=6378000
TILE_WIDTH=6000
TILE_HEIGHT=6000
FILE_TYPE=hgt
#X_TILE_COUNT=360
#Y_TILE_COUNT=180
X_TILE_COUNT=72
Y_TILE_COUNT=36
CACHE_SIZE=5
CBDAM_TMP_DIR=/scratchfs/cbdam_tmp

CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE=12
CBDAM_OUTPUT_HEIGHT_FIELD="/data1/cbdam/data/earth_rms/earth_3arcsec_rms_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE"_sub"$CBDAM_SUBSAMPLING
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD"_"`date '+%F'`".log"

echo "================================================="
echo "STARTING HEIGHT RMS " $CBDAM_INPUT_HEIGHT_FIELD
echo "================================================="

../apps/builder/vic_cbdam_builder \
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
    --tmp-dir $CBDAM_TMP_DIR \
    $CBDAM_INPUT_HEIGHT_FIELD 2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

echo "================================================="
echo "FINISHED"
echo "================================================="
CBDAM_OUTPUT_HEIGHT_FIELD="/data1/cbdam/data/earth_amax/earth_3arcsec_amax_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE"_sub"$CBDAM_SUBSAMPLING
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD"_"`date '+%F'`".log"
echo
echo
echo "================================================="
echo "STARTING HEIGHT AMAX " $CBDAM_INPUT_HEIGHT_FIELD
echo "================================================="

 ../apps/builder/vic_cbdam_builder \
    --subsampling $CBDAM_SUBSAMPLING \
    --spherical-latlon \
    --spherical-planet-radius $CBDAM_INPUT_HEIGHT_FIELD_PLANET_RADIUS \
    --height-scale $CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE \
    --use-amax-error \
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
    --tmp-dir $CBDAM_TMP_DIR \
    $CBDAM_INPUT_HEIGHT_FIELD 2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

echo "================================================="
echo "FINISHED"
echo "================================================="
