#!/bin/sh

VICTMSROOT=/geodata/scratch/victms/
TILEMAPNAME=italia3d-v2-global-geodetic
TILEMAPDIR=$VICTMSROOT/$TILEMAPNAME

VICTMSBUILDER=/home/lvvr/svnarea/vic/trunk/software/geo/apps/geo_raster_quadtree_builder/vic_geo_raster_quadtree_builder

echo "============================================="
echo "Removing previous quadtree directory"
echo "============================================="
rm -rf $TILEMAPDIR

echo "============================================="
echo "Creating new quadtree directory"
echo "============================================="
$VICTMSBUILDER --create \
  --quadtree-base-dir $VICTMSROOT \
  --quadtree-name     $TILEMAPNAME \
  --quadtree-profile "global-geodetic" \

echo "============================================="
echo "SARDEGNA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "EPSG:3003" \
    --input-tiles-directory /geodata/cgr2006/050cm/EPSG-3003/sardegna \
    --input-tiles-pattern "*.jpg" 

echo "============================================="
echo "LAZIO"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "EPSG:3004" \
    --input-tiles-directory /geodata/cgr2006/050cm/EPSG-3004/lazio \
    --input-tiles-pattern "*.jpg" 
echo "============================================="
echo "CAMPANIA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "EPSG:3004" \
    --input-tiles-directory /geodata/cgr2006/050cm/EPSG-3004/campania \
    --input-tiles-pattern "*.jpg" 
echo "============================================="
echo "DONE!"
echo "============================================="

