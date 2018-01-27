#!/bin/bash

# 脚本需要root用户权限
sudo su

# 首先还原系统的完整备份
tar -g sysinfo -xJvPf $1 -C /

# 恢复备份时不需要备份的目录
mkdir /proc && chownd root:root /proc && chmod 555 /proc
mkdir /lost+found && chownd root:root /proc && chmod 700 /lost+found
mkdir /mnt && chownd root:root /proc && chmod 755 /mnt
mkdir /sys && chownd root:root /proc && chmod 555 /sys
mkdir /tmp && chownd root:root /proc && chmod 777 /tmp
mkdir /media && chownd root:root /proc && chmod 755 /mnt
mkdir /home && chownd root:root /proc && chmod 755 /home

# 恢复增量备份的系统文件，可以使用通配符，按照文件名的顺序恢复
ls $2 | sort | xargs -n 1 -t tar -g sysinfo -xJvPf -C /

# 退出root用户
exit
