#!/bin/sh
set -e
XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ../.xinit_test -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1280x720

