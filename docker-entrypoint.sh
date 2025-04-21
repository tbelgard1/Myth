#!/bin/bash
set -e

DAEMON="$1"

case "$DAEMON" in
    userd_new)
        echo "Starting Myth II user daemon (userd_new) on port 6226..."
        exec ./bin/userd_new
        ;;
    roomd_new)
        echo "Starting Myth II room/metaserver (roomd_new) on port 3453..."
        exec ./bin/roomd_new
        ;;
    game_search_server_new)
        echo "Starting Myth II game search server (game_search_server_new) on port 6113..."
        exec ./bin/game_search_server_new
        ;;
    webd_new)
        echo "Starting Myth II web daemon (webd_new) on port 8080..."
        exec ./bin/webd_new
        ;;
    *)
        echo "Unknown daemon: $DAEMON"
        echo "Usage: $0 [userd_new|roomd_new|game_search_server_new|webd_new]"
        exit 1
        ;;
esac
