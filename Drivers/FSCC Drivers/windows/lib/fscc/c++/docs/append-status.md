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
| `cppfscc`      | `v1.0.0`


## Get
```c++
bool GetAppendStatus(void) throw();
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
void EnableAppendStatus(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.EnableAppendStatus()
```


## Disable
```c++
void DisableAppendStatus(void) throw();
```


###### Examples
```c++
#include <fscc.hpp>
...

p.DisableAppendStatus()
```


### Additional Resources
- Complete example: [`examples\append-status.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/append-status.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
