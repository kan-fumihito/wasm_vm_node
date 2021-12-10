#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <map>

#include "wasm_export.h"

#include "rcl_common.hpp"
#include "msg_type.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "rcl/publisher.h"
#include "rcl/rcl.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

int publish_std_msgs_msg_String(wasm_exec_env_t exec_env,
                                int node_id,
                                int publisher_id,
                                const char *data)
{
    rcl_ret_t ret;
    std_msgs::msg::String msg;
    msg.data = std::string(data);

    wasm_node_t *wasm_node = get_node(node_id);

    Publisher_t *publisher;
    try
    {
        publisher = wasm_node->publisher.at(publisher_id);
    }
    catch (std::out_of_range &)
    {
        std::cout << "error B" << std::endl;
        exit(1);
    }

    ret = rcl_publish(publisher->publisher, &msg, nullptr);
    if (ret == RCL_RET_OK)
    {
        std::cout << msg.data << std::endl;
        return 0;
    }
    else
    {
        std::cout << "ret: " << ret << std::endl;
        return -1;
    }
}

int publish_std_msgs_msg_Float64(wasm_exec_env_t exec_env,
                                 int node_id,
                                 int publisher_id,
                                 double data)
{
    rcl_ret_t ret;
    std_msgs::msg::Float64 msg;
    msg.data = data;

    wasm_node_t *wasm_node = get_node(node_id);
    
    Publisher_t *publisher;
    try
    {
        publisher = wasm_node->publisher.at(publisher_id);
    }
    catch (std::out_of_range &)
    {
        std::cout << "error C" << std::endl;
        exit(1);
    }

    ret = rcl_publish(publisher->publisher, &msg, nullptr);
    if (ret == RCL_RET_OK)
    {
        std::cout << msg.data << std::endl;
        return 0;
    }
    else
    {
        std::cout << "ret: " << ret << std::endl;
        return -1;
    }
}