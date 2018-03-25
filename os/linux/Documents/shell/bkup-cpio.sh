#!/bin/sh -e
# Copyright (C) 2007-2008 Osamu Aoki <osamu@debian.org>, Public Domain
# BUUID=1000; USER=osamu # UID and name of a user who accesses backup files

if [ "x$1" = "x" ]; then
	BUDIR=.
else
	BUDIR=$1
fi

if [ "x$2" = "x" ]; then
	OWNER=$USER:$USER
else
	OWNER=$2
fi

XDIR0=".+/Mail|.+/Desktop"
XDIR1=".+/\.thumbnails|.+/\.?Trash|.+/\.?[cC]ache|.+/\.gvfs|.+/sessions"
XDIR2=".+/CVS|.+/\.git|.+/\.svn|.+/Qt5.9.2|.+/Archive|.+/Checkout|.+/tmp"
XSFX=".+\.iso|.+\.tgz|.+\.tar\.gz|.+\.tar\.bz2|.+\.cpio|.+\.tmp|.+\.swp|.+~"
SIZE="+99M"
DATE=$(date --utc +"%Y%m%d-%H%M%S")
XDIR3=".+/\.[cC]ache|.+/xQt5.9.2"

[ -d "$BUDIR" ] || mkdir -p "$BUDIR"
umask 077
dpkg --get-selections > /tmp/dpkg-selections.list
debconf-get-selections > /tmp/debconf-selections

{
find /tmp/dpkg-selections.list /tmp/debconf-selections /etc /usr/local /opt /srv /root -xdev -print0
find /home -xdev -regextype posix-extended -type d -regex "$XDIR3" -prune -o -print0
# find /home/$USER/Mail/Inbox /home/$USER/Mail/Outbox -print0
#find /home/$USER/Desktop  -xdev -regextype posix-extended \
#  -type d -regex "$XDIR2" -prune -o -type f -regex "$XSFX" -prune -o \
#  -type f -size  "$SIZE" -prune -o -print0
} | cpio -ov --null -O $BUDIR/BU$DATE.cpio
# } | cpio -ov --null | parallel --pipe --recend "" -k -j4 xz -zc > $BUDIR/BU$DATE.cpio.xz
chown $OWNER $BUDIR/BU$DATE.cpio
touch $BUDIR/backup.stamp
