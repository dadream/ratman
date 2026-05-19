#!/bin/sh

VICTMSROOT=/usr/ostro06/geodata
TILEMAPNAME=blue-marble-global-geodetic
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
echo "BLUE MARBLE"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    /usr/ostro06/data_source/world-topo-bathy-200406-3x86400x43200.ecw

echo "============================================="
echo "DONE!"
echo "============================================="

