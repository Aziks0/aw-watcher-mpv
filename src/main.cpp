#include <thread>

#include "main.hpp"
#include "aw_client.hpp"
#include "logging.hpp"
#include "config.hpp"

using namespace std::chrono_literals;

#define MS_BEFORE_NEXT_STOP_REQUESTED_CHECK 200

/// @brief Logger for `loop` thread.
thread_local logging::Logger *logger = nullptr;

void cleanup() {
    if (logger) {
        delete logger;
        logger = nullptr;
    }
}

/**
 * @brief Verify that the given properties exist in mpv.
 *
 * @param mpv mpv handle.
 * @param properties List of properties.
 * @returns A list of the properties that exist.
 */
properties_t validate_properties(mpv_handle *mpv, properties_t properties) {
    mpv_node properties_node;
    int res = mpv_get_property(mpv, "property-list", MPV_FORMAT_NODE,
                               &properties_node);
    if (res != MPV_ERROR_SUCCESS) {
        // TODO: retry, because it might fail if called before mpv is ready
        logger->fatal("Could not get property-list: {}.",
                      mpv_error_string(res));
        return properties_t{};
    }

    properties_t properties_list;
    for (int i = 0; i < properties_node.u.list->num; i++) {
        properties_list.push_back(properties_node.u.list->values[i].u.string);
    }
    mpv_free_node_contents(&properties_node);

    properties_t ret;
    for (std::string property : properties) {
        if (std::find(properties_list.begin(), properties_list.end(),
                      property) == properties_list.end()) {
            logger->error("Property '{}' doesn't exist.", property);
            continue;
        }

        ret.push_back(property);
        logger->info("Property '{}' exist.", property);
    }

    return ret;
}

/**
 * @brief Main loop.
 *
 * @param stop_token The stop token of the jthead.
 * @param mpv mpv client handle for this plugin. It should only be used by
 * `mpv_xxx` functions because we cannot make it thread-safe without a lot of
 * overhead.
 */
void loop(std::stop_token stop_token, mpv_handle *mpv) {
    std::string client_name(mpv_client_name(mpv));

    // TODO: get msg-level property from mpv to setup logging level before
    // loading config (`--msg-level=aw_watcher_mpv=info` for example)
    logger = new logging::Logger(client_name);

    logger->info("Loading config.");

    config::Config config = config::get_config(client_name);

    logger->set_level(config.log_level.c_str());

    logger->info("Config loaded:");
    logger->info("\turl: {}", config.url);
    logger->info("\tpoll_time: {}", config.poll_time);
    logger->info("\tpulse_time: {}", config.pulse_time);
    logger->info("\tlog_level: {}", config.log_level);

    logger->debug("Validating properties.");

    properties_t properties = validate_properties(mpv, config.properties);
    if (properties.empty()) {
        logger->fatal("The list of properties is empty.");
        cleanup();
        return;
    }

    const int loops_needed_for_heartbeat =
        (config.poll_time * 1000) / MS_BEFORE_NEXT_STOP_REQUESTED_CHECK;
    logger->debug("Loops needed for heartbeat: {}.",
                  loops_needed_for_heartbeat);

    logger->debug("Creating bucket.");

    aw_client::Client client("aw-watcher-mpv", config.url);
    aw_client::result_t res_bucket =
        client.create_bucket(client.get_default_id(), "currently-playing");
    if (res_bucket.has_error()) {
        logger->fatal("Failed to create bucket: {}.", res_bucket.error());
        cleanup();
        return;
    }
    logger->info("Bucket created: {}.", client.get_default_id());

    // Mpv needs to wait for our plugin to stop before fully shutting down.
    // Since we want to abort the loop as soon as possible, the "stop
    // requested" polling needs to be independent from the heartbeat rate.
    // That's why we use `loops_since_last_heartbeat`, it counts the number of
    // times we slept since the last heartbeat.
    unsigned int loops_since_last_heartbeat = 0;
    for (; !stop_token.stop_requested(); loops_since_last_heartbeat++) {
        // If `mpv_get_property` fails, we try again the next loop. But it
        // shouldn't fail multiple times in a row.
        if (loops_since_last_heartbeat > (loops_needed_for_heartbeat * 2)) {
            logger->fatal("Max retries reached. Something is very wrong.");
            cleanup();
            return;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(MS_BEFORE_NEXT_STOP_REQUESTED_CHECK));
        if (loops_since_last_heartbeat < loops_needed_for_heartbeat)
            continue;

        // We use `core-idle` instead of `pause` because it's "more accurate".
        //
        // From the mpv docs:
        //
        // Whether the playback core is paused. This can differ from pause in
        // special situations, such as when the player pauses itself due to low
        // network cache. This also returns yes/true if playback is restarting
        // or if nothing is playing at all. In other words, it's only no/false
        // if there's actually no video playing.
        int paused;
        int res = mpv_get_property(mpv, "core-idle", MPV_FORMAT_FLAG, &paused);
        if (res != MPV_ERROR_SUCCESS) {
            logger->error("Could not get pause property: {}.",
                          mpv_error_string(res));
            continue;
        }

        // We only send heartbeats for "playing" state
        if (paused) {
            loops_since_last_heartbeat = 0;
            continue;
        }

        logger->debug("Preparing heartbeat.");

        json data{};
        for (const std::string property : properties) {
            char *value;
            int res = mpv_get_property(mpv, property.c_str(), MPV_FORMAT_STRING,
                                       &value);
            if (res != MPV_ERROR_SUCCESS) {
                logger->error("Could not get property: {}.",
                              mpv_error_string(res));
                continue;
            }
            data[property] = std::string(value);
            mpv_free(value);
        }

        if (data.empty()) {
            logger->error("Heartbeat data is empty.");
            continue;
        }

        logger->debug("Sending heartbeat.");

        aw_client::result_t res_heartbeat =
            client.heartbeat(client.get_default_id(), config.pulse_time, data);
        if (res_heartbeat.has_error()) {
            logger->error("Could not send heartbeat: {}.",
                          res_heartbeat.error());
            continue;
        }
        logger->info("Heartbeat sent: {}", data.dump());

        loops_since_last_heartbeat = 0;
    }
}

/**
 * @brief MPV cplugin entry point.
 *
 * @param mpv mpv client handle for this plugin.
 * @returns 0 on success or -1 on error.
 */
int mpv_open_cplugin(mpv_handle *mpv) {
    // Should never happen, but just in case
    if (!mpv)
        return -1;

    std::jthread thread(loop, mpv);

    while (true) {
        // We shouldn't use a timeout other than 0 because `mpv_wait_event`
        // locks onto the mpv handle (?) but we need it in `loop`
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_SHUTDOWN) {
            break;
        }
    }

    thread.request_stop();
    thread.join();

    return 0;
}
