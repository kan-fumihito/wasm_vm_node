#ifndef NATIVE_IMPL_HPP_
#define NATIVE_IMPL_HPP_

#include <map>

#include "wasm_export.h"

#include "msg_type.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "rcl/publisher.h"
#include "rcl/rcl.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

typedef struct Publisher_t Publisher_t;

typedef struct wasm_node_t
{
    rcl_node_t *node;
    std::string node_name;
    std::string namespace_;
    std::map<int, Publisher_t *> publisher;
} wasm_node_t;

struct Publisher_t
{
    Msg_Type type;
    std::string topic_name;
    rcl_publisher_t *publisher;
};

int init_context(wasm_exec_env_t exec_env);

void checkpoint(void);
void restore(const char *img_dir);

wasm_node_t *get_node(int id);
int create_node(wasm_exec_env_t exec_env,
                const char *node_name,
                const char *namespace_);

static wasm_node_t *restore_node(int node_id,
             const char *node_name,
             const char *namespace_);

#endif // NATIVE_IMPL_HPP_