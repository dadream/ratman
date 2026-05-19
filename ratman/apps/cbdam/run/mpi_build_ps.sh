#! /bin/sh

###########

CBDAM_INPUT_HEIGHT_FIELD_DIR=/data1/cbdam/img/ps/
CBDAM_INPUT_HEIGHT_FIELD_PATTERN=ps_height_1k.tif 

CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE=0.1

CBDAM_TMP_DIR=/data1/cbdam/data/ps/
CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE=0
CBDAM_OUTPUT_HEIGHT_FIELD="/data1/cbdam/data/ps/height_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD".log"


CBDAM_INPUT_COLOR_FIELD_DIR=/data1/cbdam/img/ps/
CBDAM_INPUT_COLOR_FIELD_PATTERN=ps_texture_1k.tif

CBDAM_OUTPUT_COLOR_FIELD_TOLERANCE=0
CBDAM_OUTPUT_COLOR_FIELD="/data1/cbdam/data/ps/color_tol"$CBDAM_OUTPUT_COLOR_FIELD_TOLERANCE
CBDAM_OUTPUT_COLOR_FIELD_LOG=$CBDAM_OUTPUT_COLOR_FIELD".log"


echo "================================================="
echo "STARTING HEIGHT"
echo "================================================="

mpirun -np 1 ../apps/mpi_builder/vic_cbdam_mpi_builder \
    --tmp-dir $CBDAM_TMP_DIR \
    --planar \
    --patch-dim 64 \
    --height-scale $CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE \
    --tolerance $CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE \
    --output-file $CBDAM_OUTPUT_HEIGHT_FIELD \
    --pattern $CBDAM_INPUT_HEIGHT_FIELD_PATTERN \
    $CBDAM_INPUT_HEIGHT_FIELD_DIR  2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

echo "================================================="
echo "FINISHED HEIGHT"
echo "================================================="

echo "================================================="
echo "RUN..."
echo "================================================="

echo ../apps/viewer/vic_cbdam_viewer $CBDAM_OUTPUT_HEIGHT_FIELD
echo "================================================="

