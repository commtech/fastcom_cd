# RX Multiple

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.2.9` 
| `fscc-linux`   | `v2.2.4` 
| `cppfscc`      | `v1.0.0`


## Get
```c++
bool GetRxMultiple(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

bool status;

status = p.GetRxMultiple();
```


## Enable
```c++
void EnableRxMultiple(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.EnableRxMultiple();
```


## Disable
```c++
void DisableRxMultiple(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.DisableRxMultiple();
```


### Additional Resources
- Complete example: [`examples\rx-multiple.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/rx-multiple.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
