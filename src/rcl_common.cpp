#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <map>
#include <filesystem>

#include "wasm_export.h"
#include "rcl_common.hpp"
#include "msg_type.hpp"
#include "create_publisher.hpp"
#include "publish.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "rcl/publisher.h"
#include "rcl/rcl.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

namespace fs = std::filesystem;

static rcl_context_t context;
static std::map<int, wasm_node_t *> node_pool;

int init_context(wasm_exec_env_t exec_env)
{
    // init options
    rcl_ret_t ret;
    rcl_init_options_t init_ops = rcl_get_zero_initialized_init_options();
    static rcl_allocator_t allocator = rcl_get_default_allocator();
    ret = rcl_init_options_init(&init_ops, allocator);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init options status: " << ret << std::endl;
        return -1;
    }

    // init context
    context = rcl_get_zero_initialized_context();
    ret = rcl_init(0, nullptr, &init_ops, &context);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init context status: " << ret << std::endl;
        return -1;
    }

    // create_publisher_list[Msg_Type::std_msgs_msg_String] = create_publisher_std_msgs_msg_String;
    set_create_publisher();

    return 0;
}

void checkpoint(void)
{
    FILE *fp;
    fp = fopen("native.img", "w");

    // [int, wasm_node_t*]
    for (const auto &[node_id, node] : node_pool)
    {
        fprintf(fp, "node_id:%d\n", node_id);
        fprintf(fp, "node_name:%s\n", node->node_name.c_str());
        fprintf(fp, "namespace:%s\n", node->namespace_.c_str());

        int size = node->publisher.size();
        fprintf(fp, "size:%d\n", size);

        // [int, Publisher_t]
        for (const auto &[pub_id, pub] : node->publisher)
        {
            fprintf(fp, "pub_id:%d\n", pub_id);
            fprintf(fp, "type:%d\n", pub->type);
            fprintf(fp, "topic_name:%s\n", pub->topic_name.c_str());
        }
    }
    fclose(fp);
}

void restore(const char *dir)
{
    int node_id, pub_id, size, type;
    char buf[64] = {}, node_name[64] = {}, namespace_[64] = {}, topic_name[64] = {};
    FILE *fp;
    fs::path p = fs::path(dir) / "native.img";
    fp = fopen(p.c_str(), "r");

    init_context(nullptr);
    set_restore_publisher();

    // [int, wasm_node_t*]
    while (!feof(fp))
    {
        fscanf(fp, "%s", buf);
        if (strncmp("node_id:",buf,8))
        {
            break;
        }
        node_id = atoi(buf + strlen("node_id:"));

        fscanf(fp, "%s", node_name);
        fscanf(fp, "%s", namespace_);

        restore_node(node_id, node_name + strlen("node_name:"), namespace_ + strlen("namespace:"));

        // publisher size
        fscanf(fp, "%s", buf);
        size = atoi(buf + strlen("size:"));

        // [int, Publisher_t]
        for (int i = 0; i < size; i++)
        {
            fscanf(fp, "%s", buf);
            pub_id = atoi(buf + strlen("pub_id:"));
            fscanf(fp, "%s", buf);
            type = atoi(buf + strlen("type:"));

            fscanf(fp, "%s", topic_name);

            restore_publisher(node_id, pub_id, topic_name + strlen("topic_name:"), type);
        }
    }

    fclose(fp);
}

wasm_node_t *get_node(int id)
{
    try
    {
        return node_pool.at(id);
    }
    catch (std::out_of_range &)
    {
        std::cout << "error A" << std::endl;
        exit(1);
    }
}

int create_node(wasm_exec_env_t exec_env,
                const char *node_name,
                const char *namespace_)
{
    rcl_ret_t ret;
    static rcl_node_t node = rcl_get_zero_initialized_node();
    static rcl_node_options_t node_ops = rcl_node_get_default_options();

    ret = rcl_node_init(&node, node_name, namespace_, &context, &node_ops);
    if (ret != RCL_RET_OK)
    {
        std::cout << "init node status: " << ret << std::endl;
        return -1;
    }
    wasm_node_t *wasm_node = new wasm_node_t();
    wasm_node->node = &node;
    wasm_node->node_name = std::string(node_name);
    wasm_node->namespace_ = std::string(namespace_);

    int id = 0;
    while (node_pool.find(id) != node_pool.end())
    {
        id++;
    }

    if (!rcl_node_is_valid(&node))
    {
        std::cout << "node is invalid." << std::endl;
        return -1;
    }

    node_pool.insert(std::make_pair(id, wasm_node));
    return id;
}

static wasm_node_t *restore_node(int node_id,
                                 const char *node_name,
                                 const char *namespace_)
{
    rcl_ret_t ret;
    static rcl_node_t node = rcl_get_zero_initialized_node();
    static rcl_node_options_t node_ops = rcl_node_get_default_options();

    ret = rcl_node_init(&node, node_name, namespace_, &context, &node_ops);
    
    if (ret != RCL_RET_OK)
    {
        std::cout << "init node status: " << ret << std::endl;
        return nullptr;
    }
    wasm_node_t *wasm_node = new wasm_node_t();
    wasm_node->node = &node;
    wasm_node->node_name = std::string(node_name);
    wasm_node->namespace_ = std::string(namespace_);

    if (!rcl_node_is_valid(&node))
    {
        std::cout << "node is invalid." << std::endl;
        return nullptr;
    }

    node_pool.insert(std::make_pair(node_id, wasm_node));
    return wasm_node;
}