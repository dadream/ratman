#!/bin/sh

VICTMSROOT=/usr/ostro06/geodata
TILEMAPNAME=sardinia3d-v1-epsg3003
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
  --quadtree-profile "none" \
  --quadtree-srs "EPSG:3003" \
  --quadtree-u0 1236626 \
  --quadtree-v0 4175366 \
  --quadtree-u1 1760914 \
  --quadtree-v1 4699654 \

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
    --input-tiles-level 6 \
    /usr/nodo*/d*/sardegna/*.{tif,TIF}

echo "============================================="
echo "DONE!"
echo "============================================="

