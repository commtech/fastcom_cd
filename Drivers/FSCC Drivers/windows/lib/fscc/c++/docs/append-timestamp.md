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
| `cppfscc`      | `v1.0.0`


## Get
```c++
bool GetAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

bool status;

status = p.GetAppendStatus();
```


## Enable
```c++
void EnableAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.EnableAppendStatus()
```


## Disable
```c++
void EnableAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.DisableAppendStatus()
```


### Additional Resources
- Complete example: [`examples\append-timestamp.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/append-timestamp.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
