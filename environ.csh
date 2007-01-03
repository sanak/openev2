setenv WC_ROOT `pwd`/..
setenv INSTALL_PREFIX /data/localinst

setenv CURRENTARCH `uname -s`
setenv LIB_ARCH output/lib/lib.${CURRENTARCH}

setenv PKG_CONFIG_PATH ${INSTALL_PREFIX}/lib/pkgconfig:/usr/lib/pkgconfig

#
# openev2
#
setenv EV2_TOP ${WC_ROOT}/openev2
if ( $?PYTHONPATH == 0 ) then
setenv LD_LIBRARY_PATH ${INSTALL_PREFIX}/lib:${EV2_TOP}/${LIB_ARCH}
else
setenv LD_LIBRARY_PATH ${INSTALL_PREFIX}/lib:${EV2_TOP}/${LIB_ARCH}:$LD_LIBRARY_PATH
endif

setenv PATH ${INSTALL_PREFIX}/bin:$PATH
setenv PYTHONHOME ${INSTALL_PREFIX}

if ( $?PYTHONPATH == 0 ) then
    setenv PYTHONPATH ${EV2_TOP}/${LIB_ARCH}
else
    setenv PYTHONPATH ${EV2_TOP}/${LIB_ARCH}:$PYTHONPATH
endif

setenv OPENEV_HOME ${EV2_TOP}/resource

setenv GEOTIFF_CSV ${INSTALL_PREFIX}/share/gdal
setenv GDAL_DATA ${INSTALL_PREFIX}/share/gdal
setenv PROJ_LIB ${INSTALL_PREFIX}/share/proj
