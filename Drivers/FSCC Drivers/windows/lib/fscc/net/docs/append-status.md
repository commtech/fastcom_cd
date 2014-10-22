# Append Status

It is a good idea to pay attention to the status of each frame. For example, you may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes to each frame you read from the card, if you opt-in to see this data. There are a few methods of enabling this additional data.

###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


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
- Complete example: [`examples/append-status.cs`](../examples/append-status.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
