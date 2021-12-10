#!/bin/bash

save(){
    IMG_DIR=img/`arch`/`date +"%Y-%m-%d-%H-%M-%S"`
    echo "make dir: $IMG_DIR"
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

out/wasm_vm_node rclwasm/nodes/$1/out/$1.wasm


