#include <thread>

#include "main.hpp"
#include "aw_client.hpp"
#include "logging.hpp"
#include "config.hpp"

using namespace std::chrono_literals;

#define MS_BEFORE_NEXT_STOP_REQUESTED_CHECK 200

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
    logging::Logger logger = logging::Logger(client_name);

    // logger.debug("Loading config...");
    config::Config config = config::get_config(client_name);

    logger.set_level(config.log_level.c_str());

    logger.info("Config loaded:");
    logger.info("\turl: {}", config.url);
    logger.info("\tpoll_time: {}", config.poll_time);
    logger.info("\tpulse_time: {}", config.pulse_time);
    logger.info("\tlog_level: {}", config.log_level);

    const int loops_needed_for_heartbeat =
        (config.poll_time * 1000) / MS_BEFORE_NEXT_STOP_REQUESTED_CHECK;
    logger.debug("Loops needed for heartbeat: {}.", loops_needed_for_heartbeat);

    logger.debug("Creating bucket...");

    aw_client::Client client("aw-watcher-mpv", config.url);
    if (!client.create_bucket(client.get_default_id(), "currently-playing")) {
        logger.fatal("Failed to create bucket: {}.", client.get_last_error());
        return;
    }
    logger.info("Bucket created: {}.", client.get_default_id());

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
            logger.fatal("Max retries reached. Something is very wrong.");
            return;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(MS_BEFORE_NEXT_STOP_REQUESTED_CHECK));
        if (loops_since_last_heartbeat < loops_needed_for_heartbeat)
            continue;

        int res;

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
        res = mpv_get_property(mpv, "core-idle", MPV_FORMAT_FLAG, &paused);
        if ((res != MPV_ERROR_SUCCESS)) {
            logger.error("Could not get pause property: {}.",
                         mpv_error_string(res));
            continue;
        }

        // We only send heartbeats for "playing" state
        if (paused) {
            loops_since_last_heartbeat = 0;
            continue;
        }

        char *ptr_filename;
        res =
            mpv_get_property(mpv, "filename", MPV_FORMAT_STRING, &ptr_filename);
        if ((res != MPV_ERROR_SUCCESS)) {
            logger.error("Could not get filename property: {}.",
                         mpv_error_string(res));
            continue;
        }
        const std::string filename(ptr_filename);
        mpv_free(ptr_filename);

        char *ptr_title;
        res =
            mpv_get_property(mpv, "media-title", MPV_FORMAT_STRING, &ptr_title);
        if ((res != MPV_ERROR_SUCCESS)) {
            logger.error("Could not get media-title property: {}.",
                         mpv_error_string(res));
            continue;
        }
        const std::string title(ptr_title);
        mpv_free(ptr_title);

        logger.debug("Sending heartbeat.");

        json data{{"filename", filename}, {"title", title}};
        if (!client.heartbeat(client.get_default_id(), config.pulse_time,
                              data)) {
            logger.error("Could not send heartbeat: {}.",
                         client.get_last_error());
            continue;
        }
        logger.info("Heartbeat sent: {}", data.dump());

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
