# FAT



## Creating a virtual disk

```shell
dd if=/dev/zero of=ramdisk.img count=30000
```

```shell
fdisk ramdisk.img
```
Press [n, ENTER, ENTER, ENTER, t, 6, w]


```
mkfs.fat --offset 2048 ramdisk.img
```


## File Allocation Table (FAT)


- If the file size is larger than the sector size, file data is spanning over multiple sectors in the cluster.
- If the file size is larger than the cluster size, file data is spanning over multiple clusters in the cluster chain.

On entries in the FAT
- Directory contents (data) are a series of 32 byte directory entries.
- Value of the entry is the cluster number of the next cluster following this corresponding cluster.

## Inode
The inode is a data structure in a Unix-style file system that describes a file-system object such as a file or a directory. Each inode stores the attributes and disk block locations of the object's data.


## References
- [https://wiki.osdev.org/FAT](https://wiki.osdev.org/FAT)
- [http://elm-chan.org/docs/fat_e.html](http://elm-chan.org/docs/fat_e.html)
