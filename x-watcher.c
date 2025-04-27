#include "x-watcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIGNAL_FILE "/data/adb/copg_config_updated"

void config_changed_callback(XWATCHER_FILE_EVENT event, const char *path, int context, void *data) {
    if (event == XWATCHER_FILE_MODIFIED || event == XWATCHER_FILE_CREATED) {
        FILE *fp = fopen(SIGNAL_FILE, "w");
        if (fp) {
            fprintf(fp, "1");
            fclose(fp);
            system("chmod 644 " SIGNAL_FILE);
            system("sync");
            printf("Config file %s modified, signaled shell script\n", path);
        } else {
            fprintf(stderr, "Failed to write to signal file\n");
        }
    }
}

int main() {
    x_watcher *watcher = xWatcher_create();
    if (!watcher) {
        fprintf(stderr, "Failed to create watcher\n");
        return 1;
    }

    xWatcher_reference config_ref = {
        .path = "/data/adb/modules/COPG/config.json",
        .callback_func = config_changed_callback,
        .context = 1,
        .data = NULL
    };

    if (xWatcher_add(watcher, &config_ref) != 0) {
        fprintf(stderr, "Failed to add watcher for config.json\n");
        xWatcher_destroy(watcher);
        return 1;
    }

    printf("Monitoring %s for changes...\n", config_ref.path);
    xWatcher_run(watcher);

    xWatcher_destroy(watcher);
    return 0;
}
