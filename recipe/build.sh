mkdir build-conda
cd build-conda

cmake .. \
-D CMAKE_BUILD_TYPE:STRING="Release" \
-D USE_OCCT_UTILS:BOOL=FALSE \
-D GBS_BUILD_TESTS:BOOL=FALSE \
-D USE_RENDER:BOOL=TRUE \
-D USE_PYTHON_BINDINGS=TRUE \
-D BUILD_DOC=FALSE \
-D CMAKE_INSTALL_PREFIX=$PREFIX \
-G "Ninja" \
-Wno-dev

ninja install

cd ..