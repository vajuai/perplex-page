APP_SRCS := \
	app/app.c \
	app/routes.c \
	$(wildcard app/controllers/*.c)

LIB_DIR := lavandula/build
LIB_NAME := liblavandula.a
LIB := $(LIB_DIR)/$(LIB_NAME)

APP_CFLAGS := -Wall -Wextra -Werror -fstack-protector-strong -Wstrict-overflow -Wformat-security -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 -Ilavandula/include
APP_LDFLAGS := -L$(LIB_DIR) -llavandula -lsqlite3 -lpthread

.PHONY: all app lib simpcurl clean

all: app simpcurl

lib:
	$(MAKE) -C lavandula

app: lib
	mkdir -p build
	$(CC) $(APP_SRCS) $(APP_CFLAGS) -o build/a $(APP_LDFLAGS)

simpcurl:
	$(CC) simpcurl.c -lcurl -o simpcurl

clean:
	rm -rf build simpcurl
	$(MAKE) -C lavandula clean
