# Append Status

It is a good idea to pay attention to the status of each frame. For example, you may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes to each frame you read from the card, if you opt-in to see this data. There are a few methods of enabling this additional data.

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| cppfscc | 1.0.0 |


## Get
```c++
bool GetAppendStatus(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

bool status = p.GetAppendStatus();
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
- Complete example: [`examples/append-status.cpp`](../examples/append-status.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)
