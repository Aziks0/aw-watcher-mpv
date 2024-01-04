#pragma once

#include "common.hpp"
#include "utils.hpp"
#include <cpr/cpr.h>

namespace aw_client {

typedef outcome::result<void, std::string> result_t;

class Client {
  private:
    std::string name;
    std::string url;
    std::string default_id;
    std::string hostname;

    bool testing = false;

  public:
    Client(std::string name, std::string url);

    std::string get_default_id() { return this->default_id; };

    result_t create_bucket(std::string id, std::string type);

    result_t heartbeat(std::string id, float pulsetime, json data);
};

} // namespace aw_client
