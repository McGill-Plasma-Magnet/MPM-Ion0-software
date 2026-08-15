#!/bin/bash
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enp7s0f4u2 -j ACCEPT
sudo iptables --append FORWARD -m state --state RELATED,ESTABLISHED -j ACCEPT
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
sudo systemctl restart dnsmasq
