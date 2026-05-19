#!/bin/sh

VICTMSROOT=/geodata/scratch/victms/
TILEMAPNAME=blue-marble-global-geodetic
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
    --quadtree-profile "global-geodetic" 



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
    /geodata/blue-marble-2004/world-topo-bathy-200406-3x86400x43200.ecw

echo "============================================="
echo "DONE!"
echo "============================================="

