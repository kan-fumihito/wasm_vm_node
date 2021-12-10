#pragma once

#include "wasm_export.h"
#include "msg_type.hpp"

void set_create_publisher(void);
void set_restore_publisher(void);

int call_restore_publisher(int node_id, int pub_id, const char *topic_name, int type);

int create_publisher_std_msgs_msg_String(wasm_exec_env_t exec_env,
                                         int node_id,
                                         const char *topic_name);

int create_publisher_std_msgs_msg_Float64(wasm_exec_env_t exec_env,
                                          int node_id,
                                          const char *topic_name);
int restore_publisher_std_msgs_msg_String(int node_id,
                                          int pub_id,
                                          const char *topic_name);
int restore_publisher_std_msgs_msg_Float64(int node_id,
                                           int pub_id,
                                           const char *topic_name);

int create_publisher(wasm_exec_env_t exec_env,
                     int node_id,
                     const char *topic_name, int type);

int restore_publisher(int node_id, int pub_id,const char *topic_name, int type);
