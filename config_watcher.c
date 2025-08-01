#include "x-watcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For sleep

#define SIGNAL_FILE "/data/adb/ktaify_config_updated"

void config_changed_callback(XWATCHER_FILE_EVENT event, const char *path, int context, void *additional_data) {
    if (event == XWATCHER_FILE_MODIFIED || event == XWATCHER_FILE_CREATED) {
        FILE *fp = fopen(SIGNAL_FILE, "w");
        if (fp) {
            fprintf(fp, "1");
            fclose(fp);
            system("chmod 644 " SIGNAL_FILE);
            system("sync");
            printf("Config file %s modified, signaled shell script\n", path);
        } else {
            fprintf(stderr, "Failed to write to signal file: %s\n", strerror(errno));
        }
    }
}

int main() {
    // Initialize x-watcher
    x_watcher *watcher = xWatcher_create();
    if (!watcher) {
        fprintf(stderr, "Failed to create watcher\n");
        return 1;
    }

    // Define the file to watch
    xWatcher_reference config_ref = {
        .path = "/data/adb/modules/KTAify/ify.json",
        .callback_func = config_changed_callback,
        .context = 1,
        .additional_data = NULL
    };

    // Add the file watch
    if (!xWatcher_appendFile(watcher, &config_ref)) {
        fprintf(stderr, "Failed to add watcher for ify.json\n");
        xWatcher_destroy(watcher);
        return 1;
    }

    // Start monitoring
    printf("Monitoring %s for changes...\n", config_ref.path);
    if (!xWatcher_start(watcher)) {
        fprintf(stderr, "Failed to start watcher\n");
        xWatcher_destroy(watcher);
        return 1;
    }

    // Keep the program running
    while (1) {
        sleep(1); // Prevent CPU overuse
    }

    // Cleanup (unreachable unless interrupted)
    xWatcher_destroy(watcher);
    return 0;
}
