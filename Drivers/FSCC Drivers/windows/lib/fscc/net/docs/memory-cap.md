# Memory Cap

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `netfscc`      | `v1.0.0`


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

p.MemoryCap.Input = 1000000;
p.MemoryCap.Output = 2000000;
```


### Additional Resources
- Complete example: [`examples\memory-cap.cs`](https://github.com/commtech/netfscc/blob/master/examples/memory-cap.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
