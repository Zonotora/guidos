# Block device

In simple terms, a *block device* can be though of as a device that reads and writes one block at a time as opposed to a character device that reads and writes one byte at a time. A block here being a fixed set of bytes.


```c
typedef struct block_device_t {
    char name[16];
} block_device_t;
```