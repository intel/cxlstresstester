cat /proc/iomem | grep dax

daxctl reconfigure-device --human --mode=system-ram --force dax0.0
daxctl reconfigure-device --human --mode=system-ram --force dax1.0


echo 4 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 4 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
echo 4 > /sys/devices/system/node/node2/hugepages/hugepages-2048kB/nr_hugepages
echo 4 > /sys/devices/system/node/node3/hugepages/hugepages-2048kB/nr_hugepages
echo 4 > /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages
echo 4 > /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages
echo 4 > /sys/devices/system/node/node2/hugepages/hugepages-1048576kB/nr_hugepages
echo 4 > /sys/devices/system/node/node3/hugepages/hugepages-1048576kB/nr_hugepages

cat /proc/meminfo

