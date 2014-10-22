# Append Timestamp

###### Windows
[`KeQuerySystemTime`](http://msdn.microsoft.com/en-us/library/windows/hardware/ff553068.aspx) is used to acquire the timestamp upon complete reception of a frame.

###### Linux
[`do_gettimeofday`](http://www.fsl.cs.sunysb.edu/kernel-api/re29.html) is used to acquire the timestamp upon complete reception of a frame.

_We will be moving to [`getnstimeofday`](http://www.gnugeneration.com/books/linux/2.6.20/kernel-api/re32.html) in the 3.0 driver series._

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.4.0 |
| fscc-linux | 2.4.0 |
| cppfscc | 1.0.0 |


## Get
```c++
bool GetAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

bool status = p.GetAppendTimestamp();
```


## Enable
```c++
void EnableAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.EnableAppendTimestamp()
```


## Disable
```c++
void EnableAppendTimestamp(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.DisableAppendTimestamp()
```


### Additional Resources
- Complete example: [`examples/append-timestamp.cpp`](../examples/append-timestamp.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)
