#!/bin/sh
# Ensure /tmp/USERS_DB exists for userd
mkdir -p /tmp
if [ ! -f /tmp/USERS_DB ]; then
  touch /tmp/USERS_DB
fi
exec "$@"
