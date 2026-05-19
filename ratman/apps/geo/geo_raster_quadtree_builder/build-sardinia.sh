mpirun -np 5 ./vic_geo_raster_quadtree_builder \
 --quadtree-proj EPSG:3003 \
 --quadtree-nu 1 \
 --quadtree-nv 1 \
 --quadtree-u0 1236625.5 \
 --quadtree-v0 4175365.5 \
 --quadtree-u1 1769014.5 \
 --quadtree-v1 4699654.5 \
 --quadtree-color-remap-black-out 11 \
 --quadtree-color-remap-black-in 0 \
 --quadtree-color-remap-white-out 255 \
 --quadtree-color-remap-white-in 253 \
 --quadtree-color-remap-below-to-black 10 \
 --quadtree-color-remap-above-to-black 254 \
 --quadtree-root-dir /data2/sardinia-quadtree-gb \
 --input-tiles-default-proj EPSG:3003 \
 --input-tiles-directory /dataraid/img/sardinia2006_geotiff/ \
 > out.log 2> err.log