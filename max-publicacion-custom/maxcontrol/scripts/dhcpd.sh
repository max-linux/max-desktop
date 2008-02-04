#!/bin/sh

echo "" > /tmp/dhcpd.conf
a=0
for i in `cat ./dhcpd.conf`; 
do 
  a=$(($a+1))
  if [ $a == "1" ];
    then echo -e "option domain-name-servers $i;
default-lease-time 86400;
max-lease-time 604800;

authoritative; " >> /tmp/dhcpd.conf
  elif [ $a == "2" ]; then
    subnet=$i
    echo -e "subnet $i netmask 255.255.255.0 {" >> /tmp/dhcpd.conf
  elif [ $a == "3" ]; then
    echo -e "range $i " >> /tmp/dhcpd.conf
  elif [ $a == "4" ]; then
    echo -e "${i};
        filename "pxelinux.0";
        option subnet-mask 255.255.255.0; " >> /tmp/dhcpd.conf
        NET=`echo $subnet|cut -d '.' -f1,2,3`
	echo -e "option broadcast-address $NET.255;" >> /tmp/dhcpd.conf
  elif [ $a == "5" ]; then
    a=0; 
    echo -e "option routers $i;
}" >> /tmp/dhcpd.conf
  fi
done
sudo cp /tmp/dhcpd.conf /etc/dhcp3/dhcpd.conf
sudo /etc/init.d/dhcp3-server restart
exit 0
