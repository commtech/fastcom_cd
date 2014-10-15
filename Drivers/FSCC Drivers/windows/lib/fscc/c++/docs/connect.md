# Connect

Opening a handle using this API will only give you access to the
synchronous functionality of the card. You will need to use the COM ports
if you would like to use the asynchronous functionality.

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cppfscc`      | `v1.0.0`


## Connect
```c++
Port(unsigned port_num) throw(SystemException);
Port(unsigned port_num, bool overlapped) throw(SystemException);

Port(const Port &other) throw(SystemException);
Port& operator=(const Port &other) throw(SystemException);
```

| Exception                | Base Exception    | Cause
| ------------------------ | ----------------- | ------------------------
| `PortNotFoundException`  | `SystemException` | Port not found
| `InvalidAccessException` | `SystemException` | Insufficient permissions

###### Examples
Connect to port 0.
```c++
#include <fscc.hpp>
...

FSCC::Port p(0);
```


### Additional Resources
- Complete example: [`examples\tutorial.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/tutorial.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
