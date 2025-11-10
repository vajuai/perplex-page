# Minimal set of Lavandula sources required for this project
SRCS_LAVANDULA = \
	lavandula/server.c \
	lavandula/router.c \
	lavandula/http.c \
	lavandula/request_context.c \
	lavandula/middleware.c \
	lavandula/json.c \
	lavandula/cors.c \
	lavandula/utils.c \
	lavandula/lavandula.c \
	lavandula/dotenv.c \
	lavandula/sql.c \
	lavandula/auth.c \
	lavandula/base64.c \
	lavandula/api_response.c

SRCS = app/app.c app/routes.c $(wildcard app/controllers/*.c) $(wildcard app/middleware/*.c)

# Hardened compile flags
CFLAGS = -Wall -Wextra -Werror -fstack-protector-strong -Wstrict-overflow -Wformat-security -Wno-unused-parameter -D_FORTIFY_SOURCE=2 -O2 -lsqlite3 -Isrc -Ilavandula/include

all:
	mkdir -p build
	gcc $(SRCS) $(SRCS_LAVANDULA) $(CFLAGS) -o build/a
