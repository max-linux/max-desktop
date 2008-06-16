#!/bin/sh

echo "" > /tmp/smb2.conf
a=0
for i in `cat ./recursos.conf`; 
do 
  a=$(($a+1))
  if [ $a == "1" ];
    then echo -e "[$i]" >> /tmp/smb2.conf
  elif [ $a == "2" ]; then
    echo -e "path = $i
available = yes" >> /tmp/smb2.conf
  elif [ $a == "4" ]; then
    echo -e "valid users = @$i" >> /tmp/smb2.conf
    a=0; 
  else
    c=$i
    if [ $c == "w" ]; then 
      b="yes" 
    else
      b="no"
    fi
    echo -e "writable = $b" >> /tmp/smb2.conf
  fi
done
cat ./copias/smb.conf  /tmp/smb2.conf > /tmp/smb.conf
sudo cp /tmp/smb.conf /etc/samba
sudo /etc/init.d/samba restart
exit 0
