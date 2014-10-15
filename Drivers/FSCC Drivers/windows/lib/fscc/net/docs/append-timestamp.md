# Append Timestamp
###### Windows
[`KeQuerySystemTime`](http://msdn.microsoft.com/en-us/library/windows/hardware/ff553068.aspx)
is used to acquire the timestamp upon complete reception of a frame.

###### Linux
[`do_gettimeofday`](http://www.fsl.cs.sunysb.edu/kernel-api/re29.html)
is used to acquire the timestamp upon complete reception of a frame.

_We will be moving to 
[`getnstimeofday`](http://www.gnugeneration.com/books/linux/2.6.20/kernel-api/re32.html)
in the 3.0 driver series._


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.4.0` 
| `fscc-linux`   | `v2.4.0` 
| `netfscc`      | `v1.0.0`


###### Property
```c#
bool AppendTimestamp { get {...}, set{...} };
```


## Get
###### Examples
```c#
using Fscc;
...

var status = p.AppendTimestamp;
```


## Enable
###### Examples
```c#
using Fscc;
...

p.AppendTimestamp = true;
```


## Disable
###### Examples
```c#
using Fscc;
...

p.AppendTimestamp = false;
```


### Additional Resources
- Complete example: [`examples\append-timestamp.cs`](https://github.com/commtech/netfscc/blob/master/examples/append-timestamp.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
