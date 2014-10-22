# Memory Cap

###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


## Property
```c#
MemoryCap MemoryCap;
```


## Get
###### Examples
```c#
using Fscc;
...

var input = p.MemoryCap.Input;
var output = p.MemoryCap.Output;
```


## Set
###### Examples
```c#
using Fscc;
...

p.MemoryCap.Input = 1000000; // 1MB
p.MemoryCap.Output = 1000000; // 1MB
```


### Additional Resources
- Complete example: [`examples/memory-cap.cs`](../examples/memory-cap.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
