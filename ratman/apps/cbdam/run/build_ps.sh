#! /bin/sh

###########

CBDAM_SUBSAMPLING=0
CBDAM_INPUT_HEIGHT_FIELD=/data1/cbdam/img/ps/ps_height_1k.png
CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE=0.1
CBDAM_INPUT_HEIGHT_FIELD_SAMPLE_SPACING=160

CBDAM_TMP_DIR=/data1/cbdam/data/ps/
CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE=1.5
CBDAM_OUTPUT_HEIGHT_FIELD="/data1/cbdam/data/ps/height_"`basename $CBDAM_INPUT_HEIGHT_FIELD .png`"_tol"$CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE"_sub"$CBDAM_SUBSAMPLING
CBDAM_OUTPUT_HEIGHT_FIELD_LOG=$CBDAM_OUTPUT_HEIGHT_FIELD".log"


CBDAM_INPUT_COLOR_FIELD=/data1/cbdam/img/ps/ps_texture_1k.png
CBDAM_OUTPUT_COLOR_FIELD_TOLERANCE=5
CBDAM_OUTPUT_COLOR_FIELD="/data1/cbdam/data/ps/color_"`basename $CBDAM_INPUT_COLOR_FIELD .png`"_tol"$CBDAM_OUTPUT_COLOR_FIELD_TOLERANCE"_sub"$CBDAM_SUBSAMPLING
CBDAM_OUTPUT_COLOR_FIELD_LOG=$CBDAM_OUTPUT_COLOR_FIELD".log"


echo "================================================="
echo "STARTING HEIGHT"
echo "================================================="

 ../apps/builder/vic_cbdam_builder \
    --tmp-dir $CBDAM_TMP_DIR \
    --subsampling $CBDAM_SUBSAMPLING \
    --planar \
    --planar-sample-spacing $CBDAM_INPUT_HEIGHT_FIELD_SAMPLE_SPACING \
    --patch-dim 32 \
    --height-scale $CBDAM_INPUT_HEIGHT_FIELD_HEIGHT_SCALE \
    --tolerance $CBDAM_OUTPUT_HEIGHT_FIELD_TOLERANCE \
    --output-file $CBDAM_OUTPUT_HEIGHT_FIELD \
    $CBDAM_INPUT_HEIGHT_FIELD  2>&1 \
    | tee $CBDAM_OUTPUT_HEIGHT_FIELD_LOG

echo "================================================="
echo "FINISHED HEIGHT"
echo "================================================="

echo "================================================="
echo "STARTING COLOR"
echo "================================================="

 ../apps/builder/vic_cbdam_builder \
    --tmp-dir $CBDAM_TMP_DIR \
    --color \
    --subsampling $CBDAM_SUBSAMPLING \
    --planar \
    --patch-dim 64 \
    --tolerance $CBDAM_OUTPUT_COLOR_FIELD_TOLERANCE \
    --output-file $CBDAM_OUTPUT_COLOR_FIELD \
    $CBDAM_INPUT_COLOR_FIELD 2>&1 \
   | tee $CBDAM_OUTPUT_COLOR_FIELD_LOG

echo "================================================="
echo "FINISHED COLOR"
echo "================================================="

echo "================================================="
echo "RUN..."
echo "================================================="

echo ../apps/viewer/vic_cbdam_viewer --color-file $CBDAM_OUTPUT_COLOR_FIELD $CBDAM_OUTPUT_HEIGHT_FIELD
echo "================================================="

