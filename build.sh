#
# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

#!/bin/bash

CURR_DIR=$PWD
WAMR_DIR=/opt/wasm-micro-runtime
OUT_DIR=${PWD}/out

rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}

echo "#####################build basic project"
cd ${CURR_DIR}
mkdir -p cmake_build
cd cmake_build
cmake ..
make
if [ $? != 0 ];then
    echo "BUILD_FAIL basic exit as $?\n"
    exit 2
fi

cp -a wasm_vm_node ${OUT_DIR}

echo -e "\n"
