## Create a small ext4 filesystem:

* ramfs type of filesystem is mounted on /mnt/ramdisk.
* dd : a null ext4 fs image (/sda/zero) is copied into /mnt/ramdisk/ext4.image and block size is specified as 1MB with 512 such blocks.
* mkfs.ext4: ext4 file system is created using the created ext4.image.
* finally the created file system is mounted on  /mnt/ext4ramdisk that is used as a loop device.
* loop device: a pseudo device used for accessing filesystem.
```
sudo mkdir /mnt/ramdisk
sudo mount -t ramfs ramfs /mnt/ramdisk
sudo dd if=/dev/zero of=/mnt/ramdisk/ext4.image bs=1M count=512
sudo mkfs.ext4 /mnt/ramdisk/ext4.image
sudo mkdir /mnt/ext4ramdisk
sudo mount -o loop /mnt/ramdisk/ext4.image /mnt/ext4ramdisk
```

```
//df -Th command shows this created ext4 filesystem among others.
Filesystem     Type      Size  Used Avail Use% Mounted on
/dev/loop16    ext4      488M  792K  452M   1% /mnt/ext4ramdisk
```

## Run ext4_checker
```
//To execute the ext4_checker.c

sudo gcc ext4_checker.c -o ext4_checker -lm
sudo ./ext4_checker
```

## Get some tests done
```
//segment is /dev/loop16
//mount point is /mnt/ext4ramdisk

// to list all files and directories:
sudo ls -al /mnt/ext4ramdisk/*

//to create directories:
sudo mkdir /mnt/ext4ramdisk/hello1

//to create txt file:
sudo nano /mnt/ext4ramdisk/hello1/hi1.txt

//to remove file or directories
sudo rm -rf /mnt/ext4ramdisk/hello1


//to set attributes:
sudo setfattr -n user.comment -v "hello" /mnt/ext4ramdisk/hello1/hi.txt
//to remove attributes:
sudo setfattr -x user.comment -v /mnt/ext4ramdisk/hello1/hi.txt
//to read attributes:
 sudo getfattr /mnt/ext4ramdisk/hello1/hi.txt 
```


