# Block device

In simple terms, a *block device* can be though of as a device that reads and writes one block at a time as opposed to a character device that reads and writes one byte at a time. A block here being a fixed set of bytes.

A block device is a special file that provides buffered access to a hardware device.

```c
typedef struct block_t {
  block_read_t read;
  block_write_t write;
  void *device;
} block_t;
```