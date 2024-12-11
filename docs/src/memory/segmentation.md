# Segmentation

There are some special combinations of segment registers and general registers that point to important addresses:

|Register Pair|Full Name|Description
|-|-|-|
|`CS:IP`| Code Segment : Instruction Pointer | Points to the address where the processor will fetch the next byte of code.|
|`SS:SP`| Stack Segment : Stack Pointer| Points to the address of the top of the stack, i.e., the most recently pushed byte.|
|`SS:BP`| Stack Segment : Base Pointer | Points to the address of the top of the stack frame, i.e., the base of the data area in the call stack for the currently active subprogram.|
|`DS:SI`| Data Segment : Source Index |Often used to point to string data that is about to be copied to ES:DI.|
|`ES:DI`| Extra Segment : Destination Index | Typically used to point to the destination for a string copy, as mentioned above.|