#!/bin/sh

# 替换opkg的安装源，发布版本的安装源文件有问题
cp distfeeds.conf /etc/opkg

while true; do
	break

	while true; do 
		opkg install libustream-mbedtls || continue;
 		break;
	done
	sed -i s/http:/https:/g /etc/opkg/distfeeds.conf
	
	while true; do
		opkg update || continue;

	 	opkg install coreutils-base64 curl ca-certificates ca-bundle || continue;
	
		opkg install shadowsocks-libev-ss-local || continue;
		opkg install shadowsocks-libev-ss-redir || continue;
		opkg install shadowsocks-libev-ss-tunnel || continue; 
		opkg install shadowsocks-libev-ss-rules  || continue;
		opkg install shadowsocks-libev-config || continue;
		opkg install luci-app-shadowsocks-libev || continue;

		break;
	done
	
	ss-redir -h > /dev/null && ss-tunnel -h > /dev/null && ss-local -h > /dev/null && break;
	
	opkg remove --force-removal-of-dependent-packages   shadowsocks-libev-ss-local \ 
			shadowsocks-libev-ss-rules  hadowsocks-libev-config \
			shadowsocks-libev-ss-redir shadowsocks-libev-ss-tunnel luci-app-shadowsocks-libev
done

chmod -x /usr/bin/ss-rules
mv /usr/bin/ss-rules /usr/bin/ss-rules.origin
cp ss-rules /usr/bin/
chmod +x /usr/bin/ss-rules

mkdir /etc/dnsmasq.d
echo  "conf-dir=/etc/dnsmasq.d" > /etc/dnsmasq.conf
cp dnsmasq_gfwlist.conf /etc/dnsmasq.d/

read -p "Input SS server address: " ss_server

read -p "Input SS server port: " ss_port

read -p "Input SS server encrypt method: " ss_enc

read -p "Input SS server password: " ss_pass

cat << EOF > /etc/config/shadowsocks-libev
config ss_local
	option server 'sss0'
	option local_address '0.0.0.0'
	option local_port '1080'
	option timeout '30'
	option mode 'tcp_and_udp'
	option fast_open '1'
	option disabled 'false'

config ss_tunnel
	option server 'sss0'
	option local_address '0.0.0.0'
	option mode 'tcp_and_udp'
	option timeout '60'
	option local_port '5353'
	option tunnel_address '8.8.8.8:53'

config ss_redir 'hi'
	option server 'sss0'
	option local_address '0.0.0.0'
	option local_port '1100'
	option mode 'tcp_and_udp'
	option timeout '60'
	option fast_open '1'
	option verbose '1'
	option reuse_port '1'
	option disabled '0'

config ss_redir 'hj'
	option disabled '1'
	option server 'sss0'
	option local_address '0.0.0.0'
	option local_port '1100'
	option mode 'tcp_and_udp'
	option timeout '60'
	option fast_open '1'
	option verbose '1'
	option reuse_port '1'

config ss_rules 'ss_rules'
	option redir_tcp 'hi'
	option redir_udp 'hi'
	option src_default 'checkdst'
	option local_default 'checkdst'
	list src_ips_forward '192.168.1.4'
	list dst_ips_forward '8.8.8.8'
	option dst_forward_recentrst '0'
	option dst_default 'bypass'

config server 'sss0'
	option server '$ss_server'
	option server_port '$ss_port'
	option method '$ss_enc'
	option password '$ss_pass'
EOF

cat << EOF > /etc/firewall.user
ipset -N gfw_set iphash
iptables -t nat -A PREROUTING -p tcp -m set --match-set gfw_set dst -j REDIRECT --to-port 1100
iptables -t nat -A OUTPUT -p tcp -m set --match-set gfw_set dst -j REDIRECT --to-port 1100
EOF

# 定时任务，更新dnsmasq_gfwlist.conf
cp gfwlist2dnsmasq.sh /usr/bin/
chmod +x /usr/bin/gfwlist2dnsmasq.sh
echo "0 5 * * 0  cd /tmp && /usr/bin/gfwlist2dnsmasq.sh -s gfw_set -o /etc/dnsmasq.d/gfwlist.conf > /dev/null"  > /tmp/crontab.root

crontab /tmp/crontab.root
rm /tmp/crontab.root

/etc/init.d/shadowsocks-libev restart
/etc/init.d/dnsmasq restart
/etc/init.d/network restart
/etc/init.d/firewall restart
