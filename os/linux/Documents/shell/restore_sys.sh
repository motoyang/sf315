#!/bin/bash

create_dir() {
  for ((i=4; i<=$#; i++)); do
    eval a=\$$i
    local DIR=$1/$a
#   echo $DIR $2 $3
    mkdir -p $DIR && chown $2 $DIR && chmod $3 $DIR
  done;
}

# usage:
# restore_sys.sh /mnt/sdb2 sysinfo sys_snapshot_20180125-235658.tar.xz sys_diff_*.tar.xz
if [ $# -lt 3 ]; then
  echo -e "Usage: \n  restore_sys.sh /mnt/sdb2 sysinfo sys_snapshot_20180125-235658.tar.xz sys_diff_*.tar.xz"
  exit 1
fi

# 首先还原系统的完整备份
sudo tar -C $1 -g $2 -xJvpf $3

# 恢复备份时不需要备份的目录
C_OWN=root:root
create_dir $1 $C_OWN 555 proc sys
create_dir $1 $C_OWN 755 run mnt media home var/cache var/tmp 
create_dir $1 $C_OWN 777 tmp 
create_dir $1 $C_OWN 700 boot/efi
# ln -s $1/run $1/var/run

# 逐个恢复增量备份的系统文件
for ((i=4; i<=$#; i++)); do
  eval a=\$$i
  tar -C $1 -g $2 -xJvpf $a
done;

