#!/bin/bash

SCRIPT_NAME=$0
script_dir=`dirname $SCRIPT_NAME`
script_dir=`cd ${script_dir} && pwd`
ARCHIVE_PATH=${script_dir}/dep_pkgs
LLVM_TOP_SRC_DIR=llvm-3.1.src
LLVM_SRC_ARCHIVE=llvm-3.1.src.tar.gz
CLANG_SRC_ARCHIVE=clang-3.1.src.tar.gz
TMP_BUILD_DIR=${script_dir}/build
LLVM_BUILD_DIR=${TMP_BUILD_DIR}/${LLVM_TOP_SRC_DIR%%.src}.build
LLVM_INSTALL_DIR=${TMP_BUILD_DIR}/${LLVM_TOP_SRC_DIR%%.src}.install

# clean up old build files
rm -rf ${TMP_BUILD_DIR} 2>/dev/null
mkdir ${TMP_BUILD_DIR}
cd ${TMP_BUILD_DIR} || \
	( echo "Failed to cd ${TMP_BUILD_DIR}" && exit 1 )

# extract llvm source code
tar zxf ${ARCHIVE_PATH}/${LLVM_SRC_ARCHIVE} || \
	( echo "Failed to extract archive ${ARCHIVE_PATH}/${LLVM_SRC_ARCHIVE}" && \
	exit 1 )

# extract clang source code
mkdir ${LLVM_TOP_SRC_DIR}/tools/clang || \
	( echo "Failed to create directory for clang" && exit 1 )

cd ${LLVM_TOP_SRC_DIR}/tools/clang
tar zxf ${ARCHIVE_PATH}/${CLANG_SRC_ARCHIVE} --strip-components=1 || \
	( echo "Failed to extrat archive ${ARCHIVE_PATH}/${CLANG_SRC_ARCHIVE}" && \
	exit 1 )

mkdir -p ${LLVM_BUILD_DIR} || \
	( echo "Failed to create directory ${LLVM_BUILD_DIR}" && exit 1 )

mkdir -p ${LLVM_INSTALL_DIR} || \
	( echo "Failed to create directory ${LLVM_INSTALL_DIR}" && exit 1 )

cd ${LLVM_BUILD_DIR}
cmake -G "Unix Makefiles" ../llvm-3.1.src -DCMAKE_INSTALL_PREFIX=/mnt/disk2/charlie/aos7710_32x/EC_VOB/utility/llvm-3.1.install -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;CppBackend" || \
	( echo "cmake error" && exit 1 )

make || ( echo "make llvm error" && exit 1 )

cd ../../src
make || ( echo "make mget error" && exit 1 )

echo "Done"
exit 0
