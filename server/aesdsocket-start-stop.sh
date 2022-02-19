#!/bin/sh
#Init script to start, stop the aesd sppliocation in daemon mode
#Reference:https://www.coursera.org/learn/advanced-embedded-software-development/lecture/SSRzk/linux-system-initialization

case "$1" in
	start)
		echo "Socket application starting"
		start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
		;;
	stop)
		echo "Socket application stopping"
		start-stop-daemon -K -n aesdsocket
		;;
	*)
		echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0

