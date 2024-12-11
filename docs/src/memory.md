# Memory

## Higher Half Kernel
Linux (among other unix-like kernels) reside at virtual addresses `0xC000000 - 0xFFFFFFFF`, leaving `0x00000000 – 0xBFFFFFFF` for user code, data, stacks, libraries and so on. Kernels with this design are said to be in the **higher half**.