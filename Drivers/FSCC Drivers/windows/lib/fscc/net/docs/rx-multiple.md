# RX Multiple

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `netfscc`      | `v1.0.0`


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
- Complete example: [`examples\rx-multiple.cs`](https://github.com/commtech/netfscc/blob/master/examples/rx-multiple.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
