#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unistd.h>

#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"
#include "rcl_common.hpp"
#include "create_publisher.hpp"
#include "publish.hpp"

int _sleep(wasm_exec_env_t exec_env, int milisec)
{
    // std::cout << "native sleep: " << milisec << std::endl;
    return usleep(milisec * 1000);
}

int main(int argc, char *argv_main[])
{
    static char global_heap_buf[512 * 1024 * 8];
    uint8_t *buffer;
    char error_buf[128];
    int opt;
    char *wasm_path = NULL, *img_path = NULL;

    wasm_module_t module = NULL;
    wasm_module_inst_t module_inst = NULL;
    wasm_exec_env_t exec_env = NULL;
    uint32_t buf_size, stack_size = 8092, heap_size = 8092;
    wasm_function_inst_t func = NULL;
    char *native_buffer = NULL;
    uint32_t wasm_buffer = 0;

    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    while ((opt = getopt(argc, argv_main, "f:r")) != -1)
    {
        switch (opt)
        {
        case 'f':
            wasm_path = optarg;
            break;
        case 'r':
            img_path = optarg;
            break;
        }
    }
    if (optind == 1)
    {
        printf("optind error.\n");
        return 0;
    }

    static NativeSymbol native_symbols[] =
        {
            {"_init_context",
             (void *)init_context,
             "()i",
             nullptr},
            {"_create_node",
             (void *)create_node,
             "($$)i",
             nullptr},
            {"_create_publisher",
             (void *)create_publisher,
             "(i$i)i",
             nullptr},
            {"_sleep",
             (void *)_sleep,
             "(i)i",
             nullptr},
            {"_publish_std_msgs_msg_String",
             (void *)publish_std_msgs_msg_String,
             "(ii$)i",
             nullptr}};

    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    // Native symbols need below registration phase
    init_args.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    init_args.native_module_name = "env";
    init_args.native_symbols = native_symbols;

    if (!wasm_runtime_full_init(&init_args))
    {
        printf("Init runtime environment failed.\n");
        return -1;
    }

    buffer = (uint8_t *)bh_read_file_to_buffer(wasm_path, &buf_size);

    if (!buffer)
    {
        printf("Open wasm app file [%s] failed. size: %d\n", wasm_path, buf_size);
        goto fail;
    }

    module = wasm_runtime_load(buffer, buf_size, error_buf, sizeof(error_buf));
    if (!module)
    {
        printf("Load wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    module_inst = wasm_runtime_instantiate(module,
                                           stack_size,
                                           heap_size,
                                           error_buf,
                                           sizeof(error_buf));

    if (!module_inst)
    {
        printf("Instantiate wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
    if (!exec_env)
    {
        printf("Create wasm execution environment failed.\n");
        goto fail;
    }

    if (!(func = wasm_runtime_lookup_function(module_inst, "_start", NULL)))
    {
        printf("The _start wasm function is not found.\n");
        goto fail;
    }
    wasm_runtime_set_native_handler(checkpoint);

    if (img_path == NULL)
    {
        uint32_t argv[1];
        // pass 4 elements for function arguments
        if (!wasm_runtime_call_wasm(exec_env, func, 0, argv))
        {
            printf("call wasm function main failed. %s\n", wasm_runtime_get_exception(module_inst));
            goto fail;
        }
    }
    else
    {
        restore(img_path);
        uint32_t argv[1];
        if (!wasm_runtime_restore_wasm(exec_env, func, 0, argv))
        {
            printf("restore wasm function main failed. %s\n",
                   wasm_runtime_get_exception(module_inst));
            goto fail;
        }
    }

fail:
    if (exec_env)
        wasm_runtime_destroy_exec_env(exec_env);
    if (module_inst)
    {
        if (wasm_buffer)
            wasm_runtime_module_free(module_inst, wasm_buffer);
        wasm_runtime_deinstantiate(module_inst);
    }
    if (module)
        wasm_runtime_unload(module);
    if (buffer)
        BH_FREE(buffer);
    wasm_runtime_destroy();
    return 0;
}