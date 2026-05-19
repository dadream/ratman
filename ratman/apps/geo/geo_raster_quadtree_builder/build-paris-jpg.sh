#!/bin/sh

VICTMSROOT=/data2/victms/
TILEMAPNAME=paris-quadtree-from-jpg
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
  --quadtree-profile "none" \
  --quadtree-srs "EPSG:4326" \
  --quadtree-u0 2.2 \
  --quadtree-v0 48.7 \
  --quadtree-u1 2.6 \
  --quadtree-v1 49.1 \
  --quadtree-nu 1 \
  --quadtree-nv 1

echo "============================================="
echo "Starting builder"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
  --quadtree-dir $TILEMAPDIR \
  --input-tiles-default-srs "EPSG:32631" \
  --input-tiles-directory /geodata/scratch/paris-jpg/ \
  --input-tiles-pattern "*.jpg" \

# data1/cbdam/img/paris/ortho_jpg/ \

echo "Done"
