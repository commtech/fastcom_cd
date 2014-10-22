# Ignore Timeout

###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


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
- Complete example: [`examples/ignore-timeout.cs`](../examples/ignore-timeout.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
