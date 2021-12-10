#pragma once

#include "wasm_export.h"

int publish_std_msgs_msg_String(wasm_exec_env_t exec_env,
                                int node_id,
                                int publisher_id,
                                const char *data);

int publish_std_msgs_msg_Float64(wasm_exec_env_t exec_env,
                                 int node_id,
                                 int publisher_id,
                                 double data);

