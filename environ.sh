export WC_ROOT=`pwd`/..
export INSTALL_PREFIX=/data/localinst

CURRENTARCH=`uname -s`
LIB_ARCH=output/lib/lib.${CURRENTARCH}

export PKG_CONFIG_PATH=/data/localinst/lib/pkgconfig:/usr/lib/pkgconfig

#
# openev2
#
EV2_TOP=${WC_ROOT}/openev2
export LD_LIBRARY_PATH=${INSTALL_PREFIX}/lib:${EV2_TOP}/${LIB_ARCH}:$LD_LIBRARY_PATH
export PATH=${INSTALL_PREFIX}/bin:$PATH

export PYTHONHOME=${INSTALL_PREFIX}
export PYTHONPATH=${EV2_TOP}/${LIB_ARCH}:$PYTHONPATH

export OPENEV_HOME=${EV2_TOP}/resource

export GEOTIFF_CSV=${INSTALL_PREFIX}/share/gdal
export GDAL_DATA=${INSTALL_PREFIX}/share/gdal
export PROJ_LIB=${INSTALL_PREFIX}/share/proj
