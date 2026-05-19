#!/bin/sh

VICTMSROOT=/usr/ostro06/geodata
TILEMAPNAME=italia3d-correzioni
TILEMAPDIR=$VICTMSROOT/$TILEMAPNAME

VICTMSBUILDER=/usr/vic/bin/vic_geo_raster_quadtree_builder

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
echo "CAMPANIA correzioni"
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
    /usr/ostro06/data_source/italia3d-file_corretti/campania/*.tif

echo "============================================="
echo "LAZIO correzioni"
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
    /usr/ostro06/data_source/italia3d-file_corretti/lazio/*.tif

echo "============================================="
echo "DONE!"
echo "============================================="

