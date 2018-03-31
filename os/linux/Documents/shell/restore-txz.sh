#!/bin/bash

# usage:

# 首先恢复系统的snapshot备份，-c参数表示要创建没有备份的目录并设置权限
# restore-txz.sh -c -o /mnt/sdb2 -g sys_info -f sys_snapshot_20180323-080808.tar.xz

# 其次依次恢复diff备份的文件
# restore-txz.sh -o /mnt/sdb2 -g sys_info -f sys_diff_20180328-060606.tar.xz
# restore-txz.sh -o /mnt/sdb2 -g sys_info -f sys_diff_20180329-060909.tar.xz

# 恢复备份的home文件
# restore-txz.sh -o /mnt/sdb2/home -g home_info -f home_snapshot_20180323-012345.tar.xz
# restore-txz.sh -o /mnt/sdb2/home -g home_info -f home_diff_20180328-112345.tar.xz

# end of usage


# define some functions

create_dir() {
  for ((i=4; i<=$#; i++)); do
    eval a=\$$i
    local DIR=$1/$a
    sudo mkdir -p $DIR && sudo chown $2 $DIR && sudo chmod $3 $DIR
  done;
}

function usage() {
	local N=$(basename $0)
	echo "Usage: $N -o ODIR -g GNAME -f FNAME"
}

# start the program

# default value is no
CREATE_DIRS=no

# get option and parameters
while getopts cf:g:o: OPT; do
	case "$OPT" in
		c)	CREATE_DIRS=yes
				;;

		o)	ODIR=$OPTARG
				;;

		g)	GNAME=$OPTARG
				;;

		f)	FNAME=$OPTARG
				;;

		*)	echo "Unknown option: $opt"
				usage $0
				exit
				;;

	esac
done

# check the parameters
if [ -z $ODIR ] || [ -z $GNAME ] || [ -z $FNAME ]; then
	usage $0
	exit
fi

# 首先还原系统的完整备份
sudo tar -C ${ODIR} -g ${GNAME} -xJvpf ${FNAME}

if [ $CREATE_DIRS = "yes" ]; then
	# 恢复备份时不需要备份的目录
	C_OWN=root:root
	create_dir $ODIR $C_OWN 555 proc sys
	create_dir $ODIR $C_OWN 755 run mnt media home var/cache var/tmp 
	create_dir $ODIR $C_OWN 777 tmp 
	create_dir $ODIR $C_OWN 700 boot/efi
	# ln -s $1/run $1/var/run
fi

