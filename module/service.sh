#!/system/bin/sh
MODDIR=${0%/*}

until [ $(getprop init.svc.bootanim) = "stopped" ] ; do
    sleep 5
done

until [ -d "/sdcard/Android" ]; do
    sleep 1
done

if [ -f /data/adb/magisk/busybox ];then
    busybox_path="/data/adb/magisk/busybox"
else
    busybox_path="/data/adb/ksu/bin/busybox"
fi

chmod +x ${MODDIR}/GetmiHoYolink
nohup ${busybox_path} setuidgid "root:net_admin" ${Clash_user_group} ${MODDIR}/GetmiHoYolink 2>&1 &