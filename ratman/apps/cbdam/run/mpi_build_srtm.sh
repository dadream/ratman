#! /bin/sh

###########

CBDAM_INPUT_HEIGHT_FIELD_DIR=/usr/ostro06/data_source/srtm/
CBDAM_INPUT_HEIGHT_FIELD_PATTERN=*.TIF
CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE=1

CBDAM_SPHERICAL_RADIUS=6378000
CBDAM_MIN_SAMPLE_SPACING=0.00083
CBDAM_PATCH_DIM=64
CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE=2
CBDAM_OUTPUT_HEIGHT_FIELD="/usr/ostro06/geodata/cbdam-srtm-v2-global-geodetic/global_srtm_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD".log"
CBDAM_INPUT_U0=-180.0
CBDAM_INPUT_V0=-90.0
CBDAM_INPUT_U1=180.0
CBDAM_INPUT_V1=90.0

echo "================================================="
echo "STARTING HEIGHT"
echo "================================================="

mpirun C /usr/vic/bin/vic_cbdam_mpi_builder \
    --cylindrical-latlon \
    --spherical-planet-radius $CBDAM_SPHERICAL_RADIUS \
    --min-sample-spacing $CBDAM_MIN_SAMPLE_SPACING \
    --u0 $CBDAM_INPUT_U0 \
    --v0 $CBDAM_INPUT_V0 \
    --u1 $CBDAM_INPUT_U1 \
    --v1 $CBDAM_INPUT_V1 \
    --patch-dim $CBDAM_PATCH_DIM \
    --height-scale $CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE \
    --tolerance $CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE \
    --output-file $CBDAM_OUTPUT_HEIGHT_FIELD \
    --pattern $CBDAM_INPUT_HEIGHT_FIELD_PATTERN \
    $CBDAM_INPUT_HEIGHT_FIELD_DIR  2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

#    --keep-graph \
#    --reuse-graph \

echo "================================================="
echo "FINISHED HEIGHT"
echo "================================================="

echo "================================================="
echo "RUN..."
echo "================================================="

echo ../apps/viewer/vic_cbdam_viewer --elevation $CBDAM_OUTPUT_HEIGHT_FIELD
echo "================================================="

