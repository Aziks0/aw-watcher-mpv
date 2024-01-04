/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include "aw_client.hpp"

namespace aw_client {

inline std::string get_potential_cpr_error(cpr::Response response) {
    return response.status_code == 0 ? response.error.message
                                     : response.status_line;
}

Client::Client(std::string name, std::string url) : name(name), url(url) {
    this->hostname = utils::get_hostname();
    this->default_id = std::format("{}_{}", this->name, this->hostname);
}

result_t Client::create_bucket(std::string id, std::string type) {
    cpr::Response response =
        cpr::Post(cpr::Url{std::format("{}/buckets/{}", this->url, id)},
                  cpr::Header{{"Content-Type", "application/json"}},
                  cpr::Body{json{{"client", this->name},
                                 {"hostname", this->hostname},
                                 {"type", type}}
                                .dump()});

    // 304 means bucket already exists, which is fine.
    if (response.status_code == 200 || response.status_code == 304) {
        return outcome::success();
    }

    return get_potential_cpr_error(response);
}

result_t Client::heartbeat(std::string id, float pulsetime, json data) {
    // ISO 8601 format
    const std::string timestamp =
        std::format("{:%FT%TZ}", std::chrono::utc_clock::now());

    cpr::Response response = cpr::Post(
        cpr::Url{std::format("{}/buckets/{}/heartbeat?pulsetime={}", this->url,
                             id, pulsetime)},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{json{{"timestamp", timestamp}, {"data", data}}.dump()});

    if (response.status_code == 200) {
        return outcome::success();
    }

    return get_potential_cpr_error(response);
};

} // namespace aw_client
