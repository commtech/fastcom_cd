# Memory Cap
If your system has limited memory available, there are safety checks in place to 
prevent spurious incoming data from overrunning your system. Each port has an 
option for setting it's input and output memory cap.


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cppfscc`      | `v1.0.0`


## Structure
```c++
struct MemoryCap {
    int input;
    int output;
};
```


## Helpers
```c
void Reset(void);
```

The `Reset()` helper function should be called any time you want to reuse a
`MemoryCap` structure. It will reset the structure to it's initial state.


## Get
```c++
MemoryCap GetMemoryCap(void) throw();
```

###### Examples
```c++
#include <fscc.hpp>
...

memcap = p.GetMemoryCap();
```

At this point `memcap.input` and `memcap.output` would be set to their respective
values.


## Set
```c++
void SetMemoryCap(const MemoryCap &memcap) throw();
```

| Parameter | Type        | Description
| ----------| ----------- | ----------------------------
| `memcap`  | `MemoryCap` | The memory cap values to set

###### Examples
```c++
#include <fscc.hpp>
...

MemoryCap memcap;

memcap.input = 1000000; // 1 MB
memcap.output = 2000000; // 2 MB

p.SetMemoryCap(memcap);
```


### Additional Resources
- Complete example: [`examples\memory-cap.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/memory-cap.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
