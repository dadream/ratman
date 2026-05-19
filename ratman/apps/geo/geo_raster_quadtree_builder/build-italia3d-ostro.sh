#!/bin/sh

# Abruzzo                 2
# Basilicata              2
# Calabria                2
# Campania                2
# Emilia-Romagna          1       
# Lazio                   2
# Liguria                 1       
# Lombardia               1       
# Marche                  2
# Molise                  2
# Piemonte                1       
# Puglia                  2
# Sardegna                1       
# Sicilia                 2
# Toscana                 1       
# Trentino Alto-Adige     1       
# Umbria                  2
# Valle d'Aosta           1       
# Veneto                  1        

VICTMSROOT=/usr/ostro07/geodata
TILEMAPNAME=italia3d-v3-global-geodetic
TILEMAPDIR=$VICTMSROOT/$TILEMAPNAME

VICTMSBUILDER=/usr/vic/bin/vic_geo_raster_quadtree_builder

# Used in Italy (Peninsular Part - Zone1) Accuracy 3-4m
PROJ_IT_ZONE1="+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=-104.1,-49.1,-9.9,0.971,-2.917,0.714,-11.68 +units=m +no_defs"
# Used in Italy (Peninsular Part - Zone2) Accuracy 3-4m
PROJ_IT_ZONE2="+proj=tmerc +lat_0=0 +lon_0=15 +k=0.999600 +x_0=2520000 +y_0=0 +ellps=intl +towgs84=-104.1,-49.1,-9.9,0.971,-2.917,0.714,-11.68 +units=m +no_defs"
# Used in Italy (Sardinia - Zone1) Accuracy 3-4m
PROJ_IT_SARDINIA="+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=-168.6,-34.0,38.6,-0.374,-0.679,-1.379,-9.48 +units=m +no_defs"
# Used in Italy (Sicily - Zone2) Accuracy 3-4m
PROJ_IT_SICILY="+proj=tmerc +lat_0=0 +lon_0=15 +k=0.999600 +x_0=2520000 +y_0=0 +ellps=intl +towgs84=-50.2,-50.4,84.8,-0.690,-2.012,0.459,-28.08 +units=m +no_defs"

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
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/campania/*.{tif,TIF}

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
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/lazio/*.{tif,TIF}

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
    --input-tiles-default-srs "$PROJ_IT_SARDINIA" \
    /usr/nodo*/d*/sardegna/*.{tif,TIF}

echo "============================================="
echo "CALABRIA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/Calabria/*.{tif,TIF}

echo "============================================="
echo "SICILIA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_SICILY" \
    /usr/nodo*/d*/Sicilia/*.{tif,TIF}

echo "============================================="
echo "PUGLIA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/Puglia/*.{tif,TIF}

echo "============================================="
echo "TRENTINO"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_ZONE1" \
    /usr/nodo*/d*/trentino/*.{tif,TIF}

echo "============================================="
echo "BASILICATA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/basilicata/*.{tif,TIF}

echo "============================================="
echo "UMBRIA"
echo "============================================="
mpirun C $VICTMSBUILDER --update \
    --quadtree-dir $TILEMAPDIR \
    --color-remap-black-in 0 \
    --color-remap-white-in 255 \
    --color-remap-black-out 15 \
    --color-remap-white-out 255 \
    --color-remap-below-to-black 10 \
    --color-remap-above-to-black 254 \
    --input-tiles-default-srs "$PROJ_IT_ZONE2" \
    /usr/nodo*/d*/umbria/*.{tif,TIF}


echo "============================================="
echo "DONE!"
echo "============================================="

