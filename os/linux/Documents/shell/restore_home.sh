#!/bin/bash

# 脚本需要root用户权限

# 首先还原home的完整备份
sudo tar -g homeinfo -xJvPf $1 -C /home

# 恢复增量备份的home文件，可以使用通配符，按照文件名的顺序恢复
sudo ls $2 | sort | xargs -n 1 -t tar -g homeinfo -xJvPf -C /home
