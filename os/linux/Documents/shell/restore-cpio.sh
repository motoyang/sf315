#!/bin/sh -e
# Copyright (C) 2007-2008 Osamu Aoki <osamu@debian.org>, Public Domain
# BUUID=1000; USER=osamu # UID and name of a user who accesses backup files

cpio -idvu "/etc/apt/sources.list|/tmp/dpkg-selections.list|/tmp/debconf-selections" < $1

apt update
dselect update
debconf-set-selections < /tmp/debconf-selections
dpkg --set-selections < /tmp/dpkg-selections.list
apt-get -u dselect-upgrade 

cpio -idv < $1

