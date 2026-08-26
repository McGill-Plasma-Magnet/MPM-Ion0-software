#!/bin/bash
sd=$1

dd if=/dev/zero of=/dev/$sd bs=1M count=8
fdisk /dev/$sd
mkfs.ext4 /dev/"$sd"1
mkdir -p /mnt
mount /dev/"$sd"1 /mnt
rm -rf ArchLinuxARM-am33x-latest.tar.gz*
wget http://os.archlinuxarm.org/os/ArchLinuxARM-am33x-latest.tar.gz
bsdtar -xpf ArchLinuxARM-am33x-latest.tar.gz -C /mnt
sync
dd if=/mnt/boot/MLO of=/dev/$sd count=1 seek=1 conv=notrunc bs=128k
dd if=/mnt/boot/u-boot.img of=/dev/$sd count=2 seek=1 conv=notrunc bs=384k
umount /mnt
sync
