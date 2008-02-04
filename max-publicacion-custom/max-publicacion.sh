#!/bin/sh

cd /var/www/
tar xzf /usr/share/max/maxcontrol.tgz
cd /var/www/maxcontrol
chown www-data.www-data * -R
cp /etc/rc3.d/S99apache2 /etc/rc2.d/
rm -fr /etc/apache2/sites-enabled/000-default
cat /usr/share/max/000-default > /etc/apache2/sites-enabled/000-default
/etc/init.d/apache2 restart
echo "www-data ALL = NOPASSWD: ALL" >> /etc/sudoers
chown madrid.madrid /home/madrid/backharddi_1.0_i386_r56.deb
