#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##########################################################################
#
# Multiseat HAL inhibit daemon
# Copyright 2011, Mario Izquierdo, mariodebian at gmail
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
###########################################################################

"""
Changelog
  20110129 - First usable version

"""

import os
import sys
import pprint
import commands
import multiprocessing

import gobject
gobject.threads_init()

from subprocess import Popen, PIPE, STDOUT

import dbus
import dbus.service
if getattr(dbus, 'version', (0, 0, 0)) >= (0, 41, 0):
    import dbus.glib

PID_FILE="/var/run/multiseat-udisks.pid"

class MultiSeatDeviceManager:
    def __init__(self):
        self.pid=os.getpid()
        self.mainloop = gobject.MainLoop()
        self.all_devices=[]
        self.bus = dbus.SystemBus()
        self.proxy = self.bus.get_object("org.freedesktop.Hal", 
                                    "/org/freedesktop/Hal/Manager")
        self.iface = dbus.Interface(self.proxy, 'org.freedesktop.Hal.Manager')
        self.iface.connect_to_signal('DeviceAdded', self.device_added_callback)
        self.iface.connect_to_signal('DeviceRemoved', self.device_removed_callback)
        print "hal loaded"
        self.init_load()

    def device_added_callback(self, dev):
        #print "device_added_callback=%s"%dev
        self.init_load(dev)

    def device_removed_callback(self, dev):
        print "FIXME device_removed_callback=%s"%dev
        # search in self.all_devices for a dev.device present and umount it
        for i  in range(len(self.all_devices)):
            if not os.path.isfile(self.all_devices[i]['device']):
                if self.all_devices[i]['ismounted']:
                    self.all_devices[i]['ismounted']=False
                    self.remove_device(self.all_devices[i])
                    self.umount_same_disk(self.all_devices[i]['device'])
        #self.init_load()

    def umount_same_disk(self, devname):
        # removed /dev/sdc3 but not removed /dev/sdc1 and /dev/sdc2
        # force manual umount
        if len(devname) < 9:
            print "umount_same_disk() devname seems to be disk %s, no umounting..."%devname
            # disk not partition
            return
        dev_disk=devname[0:8]
        for dev in self.all_devices:
            if dev_disk in dev['device']:
                print "umount_same_disk() dev_disk=%s dev=%s"%(dev_disk, dev)
                self.remove_device(dev)

    def __print_properties(self, properties):
        print
        print '-----------------------------------------------'
        print "Device" + ":", properties['info.udi']
        print
        keys = properties.keys()
        keys.sort()
        for key in keys:
            print " " + key + '=' + str(properties[key])


    def remove_device(self, dev):
        if self.UmountDevice(dev):
            # remove launcher, mountpoint is deleted in UmountDevice()
            if os.path.isfile(dev['desktopfile']):
                os.unlink(dev['desktopfile'])
                print "remove_device() deleted %s"%dev['desktopfile']

    def get(self, obj, prop):
        return obj.Get("org.freedesktop.DBus.Properties", prop)

    def init_load(self, devname=None):
        print "init_load() devname=%s"%devname
        self.all_devices=[]
        for sto in self.iface.GetAllDevices():
            sto_obj=self.bus.get_object ('org.freedesktop.Hal', sto)
            storage = dbus.Interface (sto_obj, 'org.freedesktop.Hal.Device')
            props=storage.GetAllProperties()
            #print "devname=%s info.udi=%s"%(devname, props['info.udi'])
            if devname and devname != str(props['info.udi']):
                print "devname=%s passed but != info.udi=%s"%(devname, props['info.udi'])
                return
            if props.has_key('info.category') and \
               props['info.category'] == 'volume':
                if props.has_key('volume.fstype') and \
                   props['volume.fstype'] in ['vfat', 'iso9660', 'ext2', 'ext3', 'ext4', 'ntfs']:
                   
                    # get storage device
                    parent_obj=self.bus.get_object ('org.freedesktop.Hal', str(props['block.storage_device']) )
                    parent = dbus.Interface (parent_obj, 'org.freedesktop.Hal.Device')
                    parent_props=parent.GetAllProperties()
                    #self.__print_properties(parent_props)
                    if str(parent_props['storage.bus']) != "usb":
                        print "device is not USB storage %s"%(str(parent_props['block.device']))
                        continue
                    
                    #get serial
                    serial=commands.getoutput("udevadm info --query=env --name=%s| awk -F\"=\" '/^ID_SERIAL_SHORT=/ {print $2}'"%props['block.device'])
                    #self.__print_properties(props)
                    partnumber=str(props['volume.partition.number'])
                    seat_id=self.getSeatID(str(props['linux.sysfs_path']))
                    username=self.getUserfromSeat(seat_id)
                    if username != '':
                        desktopfile=self.getUserDesktop(username) + "/%s-%s.desktop"%(serial,partnumber)
                        useruid=int(self.getUserUID(username))
                    else:
                        desktopfile='/dev/null'
                        useruid=0
                    dev={
                        "device": str(props['block.device']),
                        "serial": serial,
                        "label": str(props['volume.label']),
                        "fstype": str(props['volume.fstype']),
                        "ismounted": bool(props['volume.is_mounted']),
                        "partnumber":partnumber,
                        "mountpoint": "/media/%s-%s"%(serial,partnumber),
                        "desktopfile": desktopfile,
                        "path":str(props['linux.sysfs_path']),
                        "seat_id":seat_id,
                        "username":username,
                        "useruid":useruid,
                        "udi":str(props['info.udi']),
                        "parent_udi":str(parent_props['info.udi']),
                        }
                    pprint.pprint(dev)
                    if not dev['ismounted'] and username != '' and self.MountDevice(dev):
                        dev['ismounted']=True
                        try:
                            self.CreateLauncher(dev)
                        except Exception, err:
                            print "init_load() Exception creating launcher, err=%s"%err
                    self.all_devices.append(dev)
        
        pprint.pprint(self.all_devices)

    def getSeatID(self, path):
        seat_id=0
        # read /dev/seat.db
        if not os.path.isfile('/dev/seat.db'):
            return seat_id
        # /sys/devices/pci0000:00/0000:00:02.1/usb1/1-2/1-2.4/1-2.4:1.0/host9/target9:0:0/9:0:0:0/block/sdc/sdc1
        # /sys/devices/pci0000:00/0000:00:02.1/usb1/1-2/{devnum|busnum}
        #
        # /sys/devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.5/1-1.5.4/1-1.5.4:1.0/host7/target7:0:0/7:0:0:0/block/sdf/sdf1
        # /sys/devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.5/{devnum|busnum}
        #
        #       basename(path) == sdf  => pendrive without partitions (don't work, busnum in wrong path)
        #       basename(path) == sdf1 => first pendrive partition
        #
        parent_folder=8
        if len(os.path.basename(path)) == 3:
            # sdf
            parent_folder=7
        buspath="/".join(path.split('/')[0:len(path.split('/'))-parent_folder])
        print "getSeatID() buspath=%s"%buspath
        if not os.path.isfile(buspath+'/busnum'):
            print "getSeatID() no busnum file in path=%s, return seat_id=0"%path
            return seat_id
        busnum=int(open(buspath+'/busnum', 'r').readline())
        devnum=int(open(buspath+'/devnum', 'r').readline())
        busnum="%03i"%busnum
        devnum="%03i"%devnum
        print "getSeatID() busnum=%s devnum=%s"%(busnum, devnum)
        f=open('/dev/seat.db', 'r')
        for line in f.readlines():
            if "%s %s "%(busnum, devnum) in line:
                print "getSeatID() found SEAT_ID in line=%s"%line.strip()
                #return 3rd column
                seat_id=line.split()[2]
        f.close()
        return seat_id

    def getUserfromSeat(self, seat_id):
        username=''
        cmd="pidof gdm && gdmdynamic -l"
        p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        for line in p.stdout.readlines():
            print "getUserfromSeat() line=%s"%line.strip()
            if ":" in line:
                # :0,pc01,8;    :2,pc04,8;    :3,pc03,9
                seats=line.strip().split(';')
                for seat in seats:
                    if ":%s"%seat_id in seat:
                        username=seat.split(',')[1]
        return username

    def MountDevice(self, d):
        if d['username'] == '':
            # don't mount if no username is logged in SEAT_ID
            return False
        if not self.CreateMountPoint(d):
            # can't create mountpoint
            return False
        # udisks is inhibited, we have to mount it
        # iso9660 (rw,nosuid,nodev,uhelper=udisks,uid=0,gid=0,iocharset=utf8,mode=0400,dmode=0500)
        #
        # vfat: rw,nosuid,nodev,uhelper=udisks,uid=1000,gid=1000,shortname=mixed,dmask=0077,utf8=1,showexec,flush
        #
        # any_allow "exec", "noexec", "nodev", "nosuid", "atime", "noatime", "nodiratime", "ro", "rw", "sync", "dirsync"
        if d['fstype'] == 'iso9660':
            options="-o rw,nosuid,nodev,uhelper=multiseat-hal,uid=%s,gid=0,iocharset=utf8,mode=0400,dmode=0500"%d['username']
        elif d['fstype'] == 'vfat':
            options="-o rw,nosuid,nodev,uhelper=multiseat-hal,uid=%s,gid=0,shortname=mixed,dmask=0077,utf8=1,showexec,flush"%d['username']
        elif d['fstype'] in ['ext2', 'ext3', 'ext4']:
            options="-o rw,nosuid,nodev,uhelper=multiseat-hal"
            # no uid,gid support, in ext* filesystems => chown mountpoint
        elif d['fstype'] == 'ntfs':
            # mount with ntfs-3g???
            options="-o rw,nosuid,nodev,uhelper=multiseat-hal,uid=%s,gid=0,dmask=0077,default_permissions"%d['username']
        else:
            options="-o rw,nosuid,nodev,uhelper=multiseat-hal"
        
        cmd="mount -t %s %s '%s' %s"%(d['fstype'], d['device'], d['mountpoint'], options)
        print "MountDevice() cmd=%s"%cmd
        p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        for line in p.stdout.readlines():
            print line
        p.communicate()
        if p.returncode == 0:
            os.chown(d['mountpoint'], d['useruid'], 0)
            os.chmod(d['mountpoint'], 0700)
            return True
        return False

    def UmountDevice(self, d):
        cmd="umount %s"%(d['device'])
        print "UmountDevice() cmd=%s"%cmd
        p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        for line in p.stdout.readlines():
            print line
        p.communicate()
        if p.returncode == 0:
            self.RemoveMountPoint(d)
            return True
        return False

    def CreateMountPoint(self, d):
        if os.path.isdir(d['mountpoint']):
            return True
        try:
            os.mkdir(d['mountpoint'])
            os.chmod(d['mountpoint'], 0700)
            return True
        except Exception, err:
            print "Exception creating mountpoint %s, err=%s"%(d['mountpoint'], err)
        return False

    def RemoveMountPoint(self, d):
        try:
            os.rmdir(d['mountpoint'])
            return True
        except Exception, err:
            print "Exception removing mountpoint %s, err=%s"%(d['mountpoint'], err)
        return False

    def CreateLauncher(self, d):
        txt="""#!/usr/bin/env xdg-open

[Desktop Entry]
Version=1.0
Type=Link
Name=%s
Icon=drive-removable-media
Dev=%s
FSType=%s
MountPoint=%s
URL=%s
X-multiseat-desktop=%s
"""%(d['label'], d['device'], d['fstype'], d['mountpoint'], d['mountpoint'], d['seat_id'])
        # create /home/user/Escritorio/XXXXXXXXXXX-x.desktop (serial,partnumber)
        print txt
        f=open(d['desktopfile'], 'w')
        f.write(txt)
        f.close()
        os.chown(d['desktopfile'], d['useruid'], 0)
        os.chmod(d['desktopfile'], 0700)

    def getUserDesktop(self, username):
        home=""
        cmd="getent passwd %s"%username
        p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        for line in p.stdout.readlines():
            if "%s:"%username in line:
                home=line.split(':')[5]
        if os.path.isdir(home + "/Escritorio"):
            return home + "/Escritorio"
        elif os.path.isdir(home + "/Desktop"):
            return home + "/Desktop"
        # FIXME return ERROR???
        return home

    def getUserUID(self, username):
        uid=0
        cmd="getent passwd %s"%username
        p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        for line in p.stdout.readlines():
            if "%s:"%username in line:
                uid=line.split(':')[2]
        return uid

    def run (self):
        file(PID_FILE, 'w').write('%d'%self.pid)
        try:
            self.mainloop.run()
        except KeyboardInterrupt:
            self.quit()
        
    def quit(self, *args):
        self.mainloop.quit()
        sys.exit(0)



class DevKitInhibitor(multiprocessing.Process):
    def __init__(self):
        multiprocessing.Process.__init__(self)

    def run(self):
        try:
            os.system("devkit-disks --inhibit")
        except KeyboardInterrupt:
            os.system("killall devkit-disks")


if __name__ == '__main__':
    if "--daemon" in sys.argv:
        # run inhibitor daemon
        if os.path.isfile('/usr/bin/devkit-disks'):
            inhibit=DevKitInhibitor()
            inhibit.start()
        # run MultiSeat detector
        app = MultiSeatDeviceManager()
        app.run()
        sys.exit(0)
    
    print """no options???? 

try:
    to run as inhibitor daemon:
        multiseat-hal --daemon

"""
