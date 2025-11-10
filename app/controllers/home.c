#include "../../lavandula/include/lavandula.h"
#include <errno.h>

static char *read_command_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        size_t sz = 256;
        char *err = malloc(sz);
        if (!err) return NULL;
        snprintf(err, sz, "Failed to run command: %s", strerror(errno));
        return err;
    }

    size_t cap = 8192; // start with 8KB
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    size_t n;
    char chunk[1024];
    while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (len + n + 1 > cap) {
            size_t new_cap = cap * 2;
            while (len + n + 1 > new_cap) new_cap *= 2;
            char *tmp = realloc(buf, new_cap);
            if (!tmp) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            cap = new_cap;
            buf = tmp;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }
    pclose(fp);

    buf[len] = '\0';
    return buf;
}

appRoute(home, ctx) {
    (void)ctx;

    // Default query to show something meaningful on the homepage
    const char *query = "latest technology news";

    // Prefer showing a helpful message when API key is missing
    const char *api = getenv("PERPLEXITY_API_KEY");
    if (!api || strlen(api) == 0) {
        const char *msg =
            "PERPLEXITY_API_KEY is not set.\n"
            "Set it in your environment and ensure './simpcurl' is executable.\n"
            "Example: export PERPLEXITY_API_KEY=your_key_here\n";
        return ok(strdup((char *)msg), TEXT_PLAIN);
    }

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "./simpcurl \"%s\"", query);

    char *output = read_command_output(cmd);
    if (!output) {
        const char *fallback =
            "Failed to retrieve results from Perplexity.\n"
            "Make sure './simpcurl' exists and is executable.\n";
        return ok(strdup((char *)fallback), TEXT_PLAIN);
    }

    return ok(output, TEXT_PLAIN);
}
