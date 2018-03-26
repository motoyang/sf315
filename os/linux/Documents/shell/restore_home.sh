#!/bin/bash

# usage:
# restore_home.sh /mnt/sdb2/home homeinfo home_snapshot_20180321-190424.tar.xz home_diff_*.tar.xz
if [ $# -lt 3 ]; then
  echo -e "Usage: \n  restore_home.sh /mnt/sdb2/home homeinfo home_snapshot_20180321-190424.tar.xz home_diff_*.tar.xz"
  exit 1
fi

# 首先还原home的完整备份
sudo tar -C $1 -g $2 -xJvpf $3

# 逐个恢复增量备份的home文件
for ((i=4; i<=$#; i++)); do
  eval a=\$$i
  tar -C $1 -g $2 -xJvpf $a
done;
