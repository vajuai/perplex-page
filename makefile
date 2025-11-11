APP_SRCS := \
	app/app.c \
	app/routes.c \
	$(wildcard app/controllers/*.c)

LAV_SRC_DIR := lavandula/src
LAV_BUILD_DIR := lavandula/build
LIB_NAME := liblavandula.a
LIB := $(LAV_BUILD_DIR)/$(LIB_NAME)
LAV_SRCS := $(shell find $(LAV_SRC_DIR) -name "*.c" ! -name "main.c" ! -name "server.c")
LAV_OVERRIDE_SRC := lavandula_override/server.c
LAV_OBJS := $(patsubst $(LAV_SRC_DIR)/%.c,$(LAV_BUILD_DIR)/%.o,$(LAV_SRCS)) $(LAV_BUILD_DIR)/server.o

APP_CFLAGS := -Wall -Wextra -Werror -fstack-protector-strong -Wstrict-overflow -Wformat-security -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 -I$(LAV_SRC_DIR)/include
APP_LDFLAGS := -L$(LAV_BUILD_DIR) -llavandula -lsqlite3 -lpthread
LAV_CFLAGS := -Wall -Wextra -Werror -fstack-protector-strong -Wstrict-overflow -Wformat-security -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 -fPIC -I$(LAV_SRC_DIR)/include -I$(LAV_SRC_DIR)

.PHONY: all app lib simpcurl clean

all: app simpcurl

lib: $(LIB)

$(LAV_BUILD_DIR):
	mkdir -p $(LAV_BUILD_DIR)

$(LAV_BUILD_DIR)/%.o: $(LAV_SRC_DIR)/%.c | $(LAV_BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(LAV_CFLAGS) -c $< -o $@

$(LAV_BUILD_DIR)/server.o: $(LAV_OVERRIDE_SRC) | $(LAV_BUILD_DIR)
	$(CC) $(LAV_CFLAGS) -c $< -o $@

$(LIB): $(LAV_OBJS)
	ar rcs $@ $^

app: lib
	mkdir -p build
	$(CC) $(APP_SRCS) $(APP_CFLAGS) -o build/a $(APP_LDFLAGS)

simpcurl:
	$(CC) simpcurl.c -lcurl -o simpcurl

clean:
	rm -rf build simpcurl
	$(MAKE) -C lavandula clean
