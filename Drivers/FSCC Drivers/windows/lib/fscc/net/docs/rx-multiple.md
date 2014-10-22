# RX Multiple

###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


## Property
```c#
bool RxMultiple { get {...}, set{...} };
```


## Get
###### Examples
```c#
using Fscc;
...

var status = p.RxMultiple;
```


## Enable
```c#
using Fscc;
...

p.RxMultiple = true;
```


## Disable
###### Examples
```c#
using Fscc;
...

p.RxMultiple = false;
```


### Additional Resources
- Complete example: [`examples/rx-multiple.cs`](../examples/rx-multiple.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
