# Ignore Timeout

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cppfscc`      | `v1.0.0`


## Get
```c++
bool GetIgnoreTimeout(void) throw();
```


###### Examples
```c++
#include <fscc.hpp>
...

bool status;

status = p.GetIgnoreTimeout();
```


## Enable
```c++
void EnableIgnoreTimeout(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.EnableIgnoreTimeout()
```


## Disable
```c++
void DisableIgnoreTimeout(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

p.DisableIgnoreTimeout()
```


### Additional Resources
- Complete example: [`examples\ignore-timeout.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/ignore-timeout.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
