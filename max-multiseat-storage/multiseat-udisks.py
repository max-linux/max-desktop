#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##########################################################################
#
# Multiseat UDisk inhibit daemon
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

import os, sys
import atexit

import gobject
gobject.threads_init()

from subprocess import Popen, PIPE, STDOUT
import threading

import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
    import dbus.glib

PID_FILE="/var/run/multiseat-udisks.pid"


class DBusException(Exception):
    def __init__(self, *args, **kwargs):
        pass
    def __str__(self):
        pass
        
    def get_dbus_name(self):
        return self._dbus_error_name

#dbus.exceptions.DBusException=DBusException

class MultiSeatDeviceManager:
    def __init__(self):
        self.pid=os.getpid()
        self.mainloop = gobject.MainLoop()
        self.all_devices=[]
        self.bus = dbus.SystemBus()
        self.proxy = self.bus.get_object("org.freedesktop.UDisks", 
                                    "/org/freedesktop/UDisks")
        self.iface = dbus.Interface(self.proxy, 'org.freedesktop.UDisks')
        self.iface.connect_to_signal('DeviceAdded', self.device_added_callback)
        self.iface.connect_to_signal('DeviceRemoved', self.device_removed_callback)

    def device_added_callback(self, dev):
        self.init_load()

    def device_removed_callback(self, dev):
        # search in self.all_devices for a dev.device present and umount it
        for i  in range(len(self.all_devices)):
            if not os.path.exists(self.all_devices[i]['device']):
                print "device_removed_callback() NO EXISTS dev=%s"%self.all_devices[i]
                if self.all_devices[i]['ismounted']:
                    self.all_devices[i]['ismounted']=False
                    self.remove_device(self.all_devices[i])
                    self.umount_same_disk(self.all_devices[i]['device'])
        self.init_load()

    def umount_same_disk(self, devname):
        # removed /dev/sdc3 but not removed /dev/sdc1 and /dev/sdc2
        # force manual umount
        if len(devname) < 9:
            # disk not partition
            return
        dev_disk=devname[0:8]
        for dev in self.all_devices:
            if dev_disk in dev['device']:
                print "umount_same_disk() dev_disk=%s dev=%s"%(dev_disk, dev)
                self.remove_device(dev)

    def remove_device(self, dev):
        if self.UmountDevice(dev):
            # remove launcher, mountpoint is deleted in UmountDevice()
            if os.path.isfile(dev['desktopfile']):
                os.unlink(dev['desktopfile'])
                print "remove_device() deleted %s"%dev['desktopfile']

    def get(self, obj, prop):
        return obj.Get("org.freedesktop.DBus.Properties", prop)

    def init_load(self):
        self.all_devices=[]
        for sto in self.iface.EnumerateDevices():
            sto_obj = self.bus.get_object ('org.freedesktop.UDisks', sto)
            storage = dbus.Interface (sto_obj, 'org.freedesktop.DBus.Properties')
            try:
                storage.Get("org.freedesktop.DBus.Properties", 'DeviceIsRemovable')
            except Exception, err:
                print "Exception, perhaps removing device... err=%s"%err
                continue
            if bool(self.get(storage, 'DeviceIsPartition')) and \
               str(self.get(storage, 'DriveConnectionInterface')) == 'usb':
                fstype=str(self.get(storage, 'IdType'))
                if fstype not in ['vfat', 'iso9660', 'ext2', 'ext3', 'ext4', 'ntfs']:
                    continue
                path=str(self.get(storage, 'NativePath'))
                seat_id=self.getSeatID(path)
                serial=str(self.get(storage, 'DriveSerial'))
                partnumber=str(self.get(storage, 'PartitionNUmber'))
                label=str(self.get(storage, 'IdLabel'))
                if label == '':
                    label=serial
                username=self.getUserfromSeat(seat_id)
                desktopfile=self.getUserDesktop(username) + "/%s-%s.desktop"%(serial,partnumber)
                useruid=int(self.getUserUID(username))
                dev={
                    "device": str(self.get(storage, 'DeviceFile')),
                    "model": str(self.get(storage, 'DriveModel')),
                    "vendor": str(self.get(storage, 'DriveVendor')),
                    "serial": serial,
                    "label": label,
                    "fstype": fstype,
                    "ismounted": bool(self.get(storage, 'DeviceIsMounted')),
                    "partnumber":partnumber,
                    "mountpoint": "/media/%s-%s"%(serial,partnumber),
                    "desktopfile": desktopfile,
                    "path":path,
                    "seat_id":seat_id,
                    "username":username,
                    "useruid":useruid,
                  }
                print "init_load() device=%s"%dev
                if not dev['ismounted'] and self.MountDevice(dev):
                    dev['ismounted']=True
                    # FIXME create desktop launcher
                    try:
                        self.CreateLauncher(dev)
                    except Exception, err:
                        print "init_load() Exception creating launcher, err=%s"%err
                self.all_devices.append(dev)
        import pprint
        pprint.pprint(self.all_devices)

    def getSeatID(self, path):
        seat_id=0
        # /sys/devices/pci0000:00/0000:00:02.1/usb1/1-2/1-2.4/1-2.4:1.0/host9/target9:0:0/9:0:0:0/block/sdc/sdc1
        # /sys/devices/pci0000:00/0000:00:02.1/usb1/1-2/{devnum|busnum}
        buspath="/".join( path.split('/')[0:7] )
        print "getSeatID() buspath=%s"%buspath
        busnum=int(open(buspath+'/busnum', 'r').readline())
        devnum=int(open(buspath+'/devnum', 'r').readline())
        busnum="%03i"%busnum
        devnum="%03i"%devnum
        print "getSeatID() busnum=%s devnum=%s"%(busnum, devnum)
        # read /tmp/seat.db
        f=open('/tmp/seat.db', 'r')
        for line in f.readlines():
            if "%s %s "%(busnum, devnum) in line:
                print "getSeatID() found SEAT_ID in line=%s"%line.strip()
                seat_id=line.split()[2]
        f.close()
        #return 3rd column
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
            options="-o rw,nosuid,nodev,uhelper=multiseat-udisks,uid=%s,gid=0,iocharset=utf8,mode=0400,dmode=0500"%d['username']
        elif d['fstype'] == 'vfat':
            options="-o rw,nosuid,nodev,uhelper=multiseat-udisks,uid=%s,gid=0,shortname=mixed,dmask=0077,utf8=1,showexec,flush"%d['username']
        elif d['fstype'] in ['ext2', 'ext3', 'ext4']:
            options="-o rw,nosuid,nodev,uhelper=multiseat-udisks"
            # no uid,gid support, in ext* filesystems => chown mountpoint
        elif d['fstype'] == 'ntfs':
            # mount with ntfs-3g???
            options="-o rw,nosuid,nodev,uhelper=multiseat-udisks,uid=%s,gid=0,dmask=0077,default_permissions"%d['username']
        else:
            options="-o rw,nosuid,nodev,uhelper=multiseat-udisks"
        
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

def umount(dev, uid):
    #FIXME if /dev/sdc2 is umounted try to search /dev/sdc? in /proc/mounts and umount it too
    #print "umount() dev=%s uid=%s"%(dev, uid)
    # read /proc/mounts searching device
    found=False
    mountline=''
    f=open('/proc/mounts', 'r')
    for line in f.readlines():
        if line.startswith(dev):
            mountline=line
            found=True
    f.close()
    if not found:
        return "no-mounted"
    # exec "getent passwd uid" and read username and home
    username=''
    home=''
    cmd="getent passwd %s"%uid
    p=Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
    for line in p.stdout.readlines():
        if ":%s:"%uid in line:
            username=line.split(':')[0]
            home=line.split(':')[5]
    if username == '' or home == '':
        return "invalid-user"
    
    #if not "uid=%s"%uid in mountline:
    #    return "not-yours"
    # ntfs-3g don't put uid in mountline stat mountpoint
    allowed=False
    try:
        if os.stat(mountline.split()[1]).st_uid == int(uid):
            allowed=True
    except:
        pass
    
    if not allowed:
        return "not-yours"
    
    # get serial from mount point
    #/media/serial-partnumber
    serial_partnumber=mountline.split()[1].replace("/media/", '')
    if serial_partnumber == "":
        return "no-serial"
    
    # umount it
    os.system("sync ; umount %s"%dev)
    
    # remove mount dir
    try:
        os.rmdir(mountline.split()[1])
    except:
        pass
    
    # remove Desktop launcher
    desktop=''
    if os.path.isdir(home + "/Escritorio"):
        desktop=home + "/Escritorio"
    elif os.path.isdir(home + "/Desktop"):
        desktop=home + "/Desktop"
    
    try:
        os.unlink(desktop + "/%s.desktop"%serial_partnumber)
    except:
        pass
    
    return "ok"



if __name__ == '__main__':
    if len(sys.argv) == 3:
        #                              dev      user_uid
        #/usr/sbin/multiseat-udisks /dev/sdc1    1000
        print umount(sys.argv[1], sys.argv[2])
        sys.exit(0)
    
    elif "--daemon" in sys.argv:
        # run inhibitor daemon
        app = MultiSeatDeviceManager()
        app.run()
        sys.exit(0)
    
    print """no options???? 

try:
    to run as inhibitor daemon:
        multiseat-udisks --daemon

    to umount a device (this is called by /sbin/umount.multiseat)
        multiseat-udisks /dev/sdc1 1000

"""
