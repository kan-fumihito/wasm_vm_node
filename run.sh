#!/bin/bash

save(){
    IMG_DIR=img/`arch`/`date +"%Y-%m-%d-%H-%M-%S"`
    mkdir -p $IMG_DIR

    mv *.img $IMG_DIR/
    exit 0
}

NODE_NAME=$1

if [ "$NODE_NAME" = "" ];then
        echo "Not specified node name."
        exit 1
fi

rm *.img -f

trap 'save' 2

if [ "$2" = "" ];then
    out/wasm_vm_node -f rclwasm/nodes/$1/out/$1.wasm
else
    out/wasm_vm_node -f rclwasm/nodes/$1/out/$1.wasm -r $2
fi



