# Append Status

It is a good idea to pay attention to the status of each frame. For example, you
may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes
to each frame you read from the card, if you opt-in to see this data. There are
a few methods of enabling this additional data.

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `netfscc`      | `v1.0.0`


## Property
```c#
bool AppendStatus { get {...}, set{...} };
```


## Get
###### Examples
```c#
using Fscc;
...

var status = p.AppendStatus;
```


## Enable
###### Examples
```c#
using Fscc;
...

p.AppendStatus = true;
```


## Disable
###### Examples
```c#
using Fscc;
...

p.AppendStatus = false;
```


### Additional Resources
- Complete example: [`examples\append-status.cs`](https://github.com/commtech/netfscc/blob/master/examples/append-status.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
