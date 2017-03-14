#!/bin/sh

set -e

BUILD_DIR='cbuild'

valgrind --tool=memcheck --leak-check=full ${BUILD_DIR}/sst_test

# ${BUILD_DIR}/sst_test

