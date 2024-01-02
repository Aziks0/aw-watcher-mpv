#pragma once

#include "common.hpp"
#include "utils.hpp"
#include <cpr/cpr.h>

namespace aw_client {

class Client {
  private:
    std::string name;
    std::string url;
    std::string default_id;
    std::string last_error;
    std::string hostname;

    bool testing = false;

    void set_last_error(cpr::Response response);

  public:
    Client(std::string name, std::string url);

    std::string get_default_id() { return this->default_id; };

    std::string get_last_error() { return this->last_error; }

    bool create_bucket(std::string id, std::string type);

    bool heartbeat(std::string id, float pulsetime, json data);
};

} // namespace aw_client
