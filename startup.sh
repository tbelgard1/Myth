#!/bin/bash
set -e

# Path for DB files
db_users="/tmp/USERS_DB"
db_orders="/tmp/ORDERS_DB"

# Only initialize DBs if this is the userd service
if [[ "$1" == "bin/userd_new" && ( ! -s "$db_users" || ! -s "$db_orders" ) ]]; then
    echo "[startup.sh] Initializing USERS_DB and ORDERS_DB..."
    /bin/userd_new -b
fi

# Pass all arguments to the daemon
echo "PWD: $(pwd)"
ls -al /
# ls -al /src/ (removed: /src/ does not exist at runtime)
DAEMON="/bin/$(basename "$1")"
echo "Attempting to exec: $DAEMON ${@:2}"
if [ ! -x "$DAEMON" ]; then
    echo "ERROR: $DAEMON not found or not executable!"
    ls -al /bin/
    exit 2
fi
exec $DAEMON ${@:2}
