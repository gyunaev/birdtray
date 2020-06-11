#!/bin/sh

# This is an example of unread hook invoking notify-send to notify you about unread emails.
# It should be configured as:
# /path/to/birdtray/contrib/unreadhook/examplehook.sh %NEW% %OLD%

NOTIFYSEND="/usr/bin/notify-send"

# If the second argument is zero, this means the command-line specified was not correct
if [ -z "$2" ]; then
    $NOTIFYSEND "You didn't set up the hook correctly, must pass %NUM%"
    exit 1
fi

# Check the new counter, which must be greater than zero and same or greater than the old one
if [ "$1" -gt 0 -a "$1" -ge "$2" ]; then
    $NOTIFYSEND "$1 new emails received"
fi
