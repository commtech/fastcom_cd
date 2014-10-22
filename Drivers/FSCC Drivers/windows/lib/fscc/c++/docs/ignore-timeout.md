# Ignore Timeout

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| cppfscc | 1.0.0 |


## Get
```c++
bool GetIgnoreTimeout(void) throw();
```


###### Examples
```c++
#include <fscc.hpp>
...

bool status = p.GetIgnoreTimeout();
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
- Complete example: [`examples/ignore-timeout.cpp`](../examples/ignore-timeout.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)