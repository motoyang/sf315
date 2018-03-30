#~/bin/bash

# define some functions

# do_tar srcdir exclude_string infoname errout tarname
function do_tar() {
	sudo tar -C $1 $2 -g $3 -cvpf - . 2>$4 | parallel --no-notice --pipe --recend "" -k -j4 xz -zc > $5
}

# do_sys infoname errout tarname date tempname
function do_sys() {
	local SYSDIR=/

	cat >> $5 <<-EOF
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

	local EXCLUDE_STRING="-X $5"
  $(do_tar $SYSDIR "$EXCLUDE_STRING" "$1" "$2" "$3")
	tar -cJvf ${1}_${4}.tar.xz ${1}
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
	
	$(do_tar "$HOMEDIR" "$EXCLUDE_STRING" "$1" "$2" "$3")
	tar -cJvf ${1}_${4}.tar.xz ${1}
}

function do_other() {
	$(do_tar $1 "" "$2" "$3" "$4")
	tar -cJvf ${2}_${5}.tar.xz ${2}
}

# start the program

# get option and parameters
while getopts o:s:t: OPT
do
	case "$OPT" in
		o) ONAME=$OPTARG
			echo "Found the -g option, with value $OPTARG" ;;
		s) SNAME=$OPTARG
			echo "Found the -s option, with value $OPTARG" ;;
		*)	echo "Unknown option: $opt"
				exit
				;;
	esac
done

if [ -z $ONAME ] || [ -z $SNAME ]; then
	echo "Usage: $0 -s SNAME -o ONAME"
	exit
fi

TEMPNAME=$(mktemp t.XXXXXX)
DATE=`date +%Y%m%d-%H%M%S`
INFONAME="${ONAME}_info"
ERROUT="${ONAME}_err.txt"

TARNAME=snapshot
if [ -f "$INFONAME" ]; then 
	TARNAME=diff
fi
TARNAME="${ONAME}_${TARNAME}_${DATE}.tar.xz"

# clean the temp files
trap "rm ${TEMPNAME}" EXIT

case $SNAME in
	sys)		echo "do sys"
					do_sys "$INFONAME" "$ERROUT" "$TARNAME" "$DATE" "$TEMPNAME"
					;;

	home)		echo "do home"
					do_home "$INFONAME" "$ERROUT" "$TARNAME" "$DATE"
					;;
					
	all)		echo "all"
					$(do_sys "$INFONAME" "$ERROUT" "$TARNAME" "$DATE" "$TEMPNAME")
					$(do_home "$INFONAME" "$ERROUT" "$TARNAME" "$DATE")
					;;

	*)			echo "do other"
					$(do_other "$SNAME" "$INFONAME" "$ERROUT" "$TARNAME" "$DATE");;

esac

sleep 3
echo "rm $TEMPNAME"
# rm $TEMPNAME
