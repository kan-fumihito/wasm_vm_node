#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <map>

#include "wasm_export.h"

#include "rcl_common.hpp"
#include "msg_type.hpp"
#include "create_publisher.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "rcl/publisher.h"
#include "rcl/rcl.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

const rosidl_message_type_support_t *string_ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();

const rosidl_message_type_support_t *float64_ts =
    rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::Float64>();

static int (*create_publisher_list[Msg_Type::Max])(wasm_exec_env_t exec_env,
                                                   int node_id,
                                                   const char *topic_name);

static int (*restore_publisher_list[Msg_Type::Max])(
    int node_id,
    int pub_id,
    const char *topic_name);


void set_create_publisher(void)
{
    create_publisher_list[Msg_Type::std_msgs_msg_String] = create_publisher_std_msgs_msg_String;
}

void set_restore_publisher(void)
{
    restore_publisher_list[Msg_Type::std_msgs_msg_String] = restore_publisher_std_msgs_msg_String;
}

int create_publisher_std_msgs_msg_String(wasm_exec_env_t exec_env,
                                         int node_id,
                                         const char *topic_name)
{
    rcl_ret_t ret;
    static rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();

    wasm_node_t *wasm_node = get_node(node_id);

    if (!rcl_node_is_valid(wasm_node->node))
    {
        std::cout << "node is invalid." << std::endl;
        return -1;
    }

    // templateのタイプ別: string_ts, float64_ts, ...
    ret = rcl_publisher_init(&publisher, wasm_node->node, string_ts, topic_name, &publisher_ops);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init publisher status: " << ret << std::endl;
        return -1;
    }

    int id = 0;
    while (wasm_node->publisher.find(id) != wasm_node->publisher.end())
    {
        id++;
    }
    wasm_node->publisher.insert(std::make_pair(id, new Publisher_t{
                                                       .type = std_msgs_msg_String,
                                                       .topic_name = std::string(topic_name),
                                                       .publisher = &publisher}));
    return id;
}

int create_publisher_std_msgs_msg_Float64(wasm_exec_env_t exec_env,
                                          int node_id,
                                          const char *topic_name)
{
    rcl_ret_t ret;
    static rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();

    wasm_node_t *wasm_node = get_node(node_id);

    if (!rcl_node_is_valid(wasm_node->node))
    {
        std::cout << "node is invalid." << std::endl;
        return -1;
    }

    // templateのタイプ別: string_ts, float64_ts, ...
    ret = rcl_publisher_init(&publisher, wasm_node->node, float64_ts, topic_name, &publisher_ops);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init publisher status: " << ret << std::endl;
        return -1;
    }

    int id = 0;
    while (wasm_node->publisher.find(id) != wasm_node->publisher.end())
    {
        id++;
    }
    wasm_node->publisher.insert(std::make_pair(id, new Publisher_t{
                                                       .type = std_msgs_msg_Float64,
                                                       .topic_name = std::string(topic_name),
                                                       .publisher = &publisher}));
    return id;
}

int restore_publisher_std_msgs_msg_String(int node_id,
                                           int pub_id,
                                           const char *topic_name)
{
    rcl_ret_t ret;
    static rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();

    wasm_node_t *wasm_node = get_node(node_id);

    if (!rcl_node_is_valid(wasm_node->node))
    {
        std::cout << "node is invalid." << std::endl;
        return -1;
    }

    // templateのタイプ別: string_ts, float64_ts, ...
    ret = rcl_publisher_init(&publisher, wasm_node->node, string_ts, topic_name, &publisher_ops);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init publisher status: " << ret << std::endl;
        return -1;
    }

    wasm_node->publisher.insert(std::make_pair(pub_id, new Publisher_t{
                                                           .type = std_msgs_msg_String,
                                                           .topic_name = std::string(topic_name),
                                                           .publisher = &publisher}));
    return pub_id;
}

int restore_publisher_std_msgs_msg_Float64(int node_id,
                                            int pub_id,
                                            const char *topic_name)
{
    rcl_ret_t ret;
    static rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rcl_publisher_options_t publisher_ops = rcl_publisher_get_default_options();

    wasm_node_t *wasm_node = get_node(node_id);

    if (!rcl_node_is_valid(wasm_node->node))
    {
        std::cout << "node is invalid." << std::endl;
        return -1;
    }

    // templateのタイプ別: string_ts, float64_ts, ...
    ret = rcl_publisher_init(&publisher, wasm_node->node, float64_ts, topic_name, &publisher_ops);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init publisher status: " << ret << std::endl;
        return -1;
    }

    wasm_node->publisher.insert(std::make_pair(pub_id, new Publisher_t{
                                                           .type = std_msgs_msg_Float64,
                                                           .topic_name = std::string(topic_name),
                                                           .publisher = &publisher}));
    return pub_id;
}

int create_publisher(wasm_exec_env_t exec_env,
                     int node_id,
                     const char *topic_name, int type)
{
    return (*create_publisher_list[type])(exec_env, node_id, topic_name);
}

int restore_publisher(int node_id, int pub_id,const char *topic_name, int type)
{
    return (*restore_publisher_list[type])(node_id, pub_id,topic_name);
}