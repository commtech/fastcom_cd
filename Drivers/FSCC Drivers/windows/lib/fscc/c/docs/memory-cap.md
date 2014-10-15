# Memory Cap
If your system has limited memory available, there are safety checks in place to 
prevent spurious incoming data from overrunning your system. Each port has an 
option for setting it's input and output memory cap.


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Structure
```c
struct fscc_memory_cap {
    int input;
    int output;
};
```


## Macros
```c
FSCC_MEMORY_CAP_INIT(memcap)
```

| Parameter   | Type                       | Description
| ----------- | -------------------------- | --------------------------------------
| `memcap`    | `struct fscc_memory_cap *` | The memory cap structure to initialize

The `FSCC_MEMORY_CAP_INIT` macro should be called each time you use the 
`struct fscc_memory_cap` structure. An initialized structure will allow you to 
only set/receive the memory cap you need.


## Get
```c
int fscc_get_memory_cap(fscc_handle h, const struct fscc_memory_cap *memcap)
```

| Parameter | Type                             | Description
| --------- | -------------------------------- | -------------------------------
| `h`       | `fscc_handle`                    | The handle to your port
| `memcap`  | `const struct fscc_memory_cap *` | The current `memory cap` values

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```
#include <fscc.h>
...

struct fscc_memory_cap memcap;

FSCC_MEMORY_CAP_INIT(memcap);

fscc_get_memory_cap(h, &memcap);
```

At this point `memcap.input` and `memcap.output` would be set to their respective
values.


## Set
```c
int fscc_set_memory_cap(fscc_handle h, struct fscc_memory_cap *memcap)
```

| Parameter | Type                       | Description
| --------- | -------------------------- | -----------------------------
| `h`       | `fscc_handle`              | The handle to your port
| `memcap`  | `struct fscc_memory_cap *` | The new `memory cap` value(s)

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```
#include <fscc.h>
...

struct fscc_memory_cap memcap;

FSCC_MEMORY_CAP_INIT(memcap);

memcap.input = 1000000; /* 1 MB */
memcap.output = 2000000; /* 2 MB */

fscc_set_memory_cap(h, &memcap);
```


### Additional Resources
- Complete example: [`examples\memory-cap.c`](https://github.com/commtech/cfscc/blob/master/examples/memory-cap.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
