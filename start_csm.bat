SET opensg_BIN=../libraries/install/vs11_64/opensg/bin/rel
SET glut_bin=../libraries/install/vs11_64/freeglut/bin
SET invrs_bin=../libraries/install/vs11_64/invrs/bin
SET tiff_bin=../libraries/install/vs11_64/libtiff/bin
SET png_bin=../libraries/install/vs11_64/libpng/bin
SET zlib_bin=../libraries/install/vs11_64/zlib/bin
SET jpeg_bin=../libraries/install/vs11_64/libjpeg-turbo/bin
SET PATH=%opensg_BIN%;%glut_bin%;%invrs_bin%;%tiff_bin%;%png_bin%;%zlib_bin%;%jpeg_bin%;%PATH%; 
start "front cmd" "csmrenderserver.exe" -a 127.0.0.1 -g 800x600+100+100 -w front