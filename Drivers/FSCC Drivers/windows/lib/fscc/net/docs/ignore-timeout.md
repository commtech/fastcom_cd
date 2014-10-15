# Ignore Timeout


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `netfscc`      | `v1.0.0`


## Property
```c#
bool IgnoreTimeout { get {...}, set{...} };
```


## Get
###### Examples
```c#
using Fscc;
...

var status = p.IgnoreTimeout;
```


## Enable
###### Examples
```c#
using Fscc;
...

p.IgnoreTimeout = true;
```


## Disable
###### Examples
```c#
using Fscc;
...

p.AppendStatus = false;
```


### Additional Resources
- Complete example: [`examples\ignore-timeout.cs`](https://github.com/commtech/netfscc/blob/master/examples/ignore-timeout.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
