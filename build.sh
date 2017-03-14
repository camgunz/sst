#!/bin/sh

set -e

BUILD_DIR='cbuild'
BUILD_TYPE='Debug'
# BUILD_TYPE='Release'
CBASE_DIR="${HOME}/local"
CBASE_INCLUDE_DIR="${CBASE_DIR}/include"
CBASE_LIBRARIES="${CBASE_DIR}/lib/libcbase.so"

mkdir -p ${BUILD_DIR}
rm -rf ${BUILD_DIR}/*
pushd ${BUILD_DIR} > /dev/null

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCBASE_INCLUDE_DIR=${CBASE_INCLUDE_DIR} \
      -DCBASE_LIBRARIES=${CBASE_LIBRARIES} \
      ..

make

popd > /dev/null

