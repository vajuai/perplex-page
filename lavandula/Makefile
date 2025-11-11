LIB_NAME := liblavandula.a
BUILD_DIR := build
INCLUDE_DIR := include

SRCS := \
	api_response.c \
	auth.c \
	base64.c \
	cors.c \
	dotenv.c \
	http.c \
	json.c \
	lavandula.c \
	middleware.c \
	request_context.c \
	router.c \
	server.c \
	sql.c \
	utils.c

OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

CFLAGS ?= -Wall -Wextra -Werror -fstack-protector-strong -Wstrict-overflow -Wformat-security -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 -fPIC -I$(INCLUDE_DIR)
LDFLAGS ?=

.PHONY: all clean

all: $(BUILD_DIR)/$(LIB_NAME)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	$(AR) rcs $@ $^

clean:
	rm -rf $(BUILD_DIR)
