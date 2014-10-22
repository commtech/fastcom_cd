# RX Multiple

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.2.9 |
| fscc-linux | 2.2.4 |
| cppfscc | 1.0.0 |


## Get
```c++
bool GetRxMultiple(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

bool status = p.GetRxMultiple();
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
- Complete example: [`examples/rx-multiple.cpp`](../examples/rx-multiple.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)