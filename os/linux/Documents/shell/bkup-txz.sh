#!/bin/bash

# define some functions

function do_clean() {
	[ -f $1 ] && rm $1 &>/dev/null
}

# do_tar srcdir exclude_string infoname errout tarname
function do_tar() {
	tar -C $1 $2 -g $3 -cvpf - . 2>$4 | parallel --no-notice --pipe --recend "" -k -j4 xz -zc > $5

	[ $? -ne 0 ] && exit 1
}

# do_sys infoname errout tarname date tempname
function do_sys() {
	local SYSDIR=/

	TEMPNAME=$(mktemp t.XXXXXX)

	# clean the temp files
	trap "do_clean ${TEMPNAME}" EXIT

	cat >> $TEMPNAME <<-EOF
		proc
		run
		sys
		tmp
		media
		mnt
		home
		boot/efi
		var/cache
		var/run
		var/tmp
		lost+found
	EOF

	local EXCLUDE_STRING="-X $TEMPNAME"
  do_tar $SYSDIR "$EXCLUDE_STRING" "$1" "$2" "$3"
	rm ${TEMPNAME}

	tar -cJf ${1}_${4}.tar.xz ${1}
}

# getusers /home
function getusers() {
	local R=""
	cd "$1"
	for D in *; do
		if [ -z $R ]; then
			R=$D
		else
			R="$R $D"
		fi
	done
	echo "$R"
}

# do_home infoname errout tarname date
function do_home() {
	local HOMEDIR=/home
	local U=$(getusers $HOMEDIR)

	local EXCLUDE_STRING=""
	local EXCLUDE_LIST=".cache"
	for E in ${EXCLUDE_LIST}; do
		for D in $U; do
			EXCLUDE_STRING="${EXCLUDE_STRING} --exclude=$D/$E"
		done
	done
	
	do_tar "$HOMEDIR" "$EXCLUDE_STRING" "$1" "$2" "$3"
	tar -cJf ${1}_${4}.tar.xz ${1}
}

function do_other() {
	do_tar $1 "" "$2" "$3" "$4"
	tar -cJf ${2}_${5}.tar.xz ${2}
}

function usage() {
	local N=$(basename $0)
	echo "Usage: $N -s SNAME -o ONAME"
}

# start the program

# get option and parameters
while getopts o:s:t: OPT
do
	case "$OPT" in
		o)	ONAME=$OPTARG
				;;

		s)	SNAME=$OPTARG
				;;

		*)	echo "Unknown option: $opt"
				usage $0
				exit
				;;

	esac
done

if [ -z $ONAME ] || [ -z $SNAME ]; then
	usage $0
	exit
fi

DATE=`date +%Y%m%d-%H%M%S`
INFONAME="${ONAME}_info"
ERROUT="${ONAME}_err.txt"

TARNAME=snapshot
if [ -f "$INFONAME" ]; then 
	TARNAME=diff
fi
TARNAME="${ONAME}_${TARNAME}_${DATE}.tar.xz"

case $SNAME in
	sys)		read -n 1 -p "Backup the system to a tar file named ${TARNAME}. [y/n]?"
					echo ""
					if [ $REPLY = "y" ]; then
						do_sys "$INFONAME" "$ERROUT" "$TARNAME" "$DATE"
					fi
					;;

	home)		read -n 1 -p "Backup the home directory to a tar file named ${TARNAME}. [y/n]?"
					echo ""
					if [ $REPLY = "y" ]; then
						do_home "$INFONAME" "$ERROUT" "$TARNAME" "$DATE"
					fi
					;;
					
	all)		read -n 1 -p "Backup the system and home. [y/n]?"
					echo ""
					if [ $REPLY = "y" ]; then
						do_sys "$INFONAME" "$ERROUT" "$TARNAME" "$DATE" "$TEMPNAME"
						do_home "$INFONAME" "$ERROUT" "$TARNAME" "$DATE"
					fi
					;;

	*)			read -n 1 -p "Backup the $SNAME directory to ${TARNAME}. [y/n]?"
					echo ""
					if [ $REPLY = "y" ]; then
						do_other "$SNAME" "$INFONAME" "$ERROUT" "$TARNAME" "$DATE"
					fi
					;;

esac

echo "Done. Everything is OK."

