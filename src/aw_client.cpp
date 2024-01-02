#include "aw_client.hpp"

// TODO: better logging

namespace aw_client {

Client::Client(std::string name, std::string url) : name(name), url(url) {
    this->hostname = utils::get_hostname();
    this->default_id = std::format("{}_{}", this->name, this->hostname);
}

void Client::set_last_error(cpr::Response response) {
    if (response.status_code == 0) {
        this->last_error = response.error.message;
        return;
    }

    this->last_error = response.status_line;
}

bool Client::create_bucket(std::string id, std::string type) {
    cpr::Response response =
        cpr::Post(cpr::Url{std::format("{}/buckets/{}", this->url, id)},
                  cpr::Header{{"Content-Type", "application/json"}},
                  cpr::Body{json{{"client", this->name},
                                 {"hostname", this->hostname},
                                 {"type", type}}
                                .dump()});

    this->set_last_error(response);

    // 304 means bucket already exists, which is fine.
    return response.status_code == 200 || response.status_code == 304;
}

bool Client::heartbeat(std::string id, float pulsetime, json data) {
    // ISO 8601 format
    const std::string timestamp =
        std::format("{:%FT%TZ}", std::chrono::utc_clock::now());

    cpr::Response response = cpr::Post(
        cpr::Url{std::format("{}/buckets/{}/heartbeat?pulsetime={}", this->url,
                             id, pulsetime)},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{json{{"timestamp", timestamp}, {"data", data}}.dump()});

    this->set_last_error(response);

    return response.status_code == 200;
};

} // namespace aw_client
