# Track Interrupts

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.7.0 |
| cppfscc | 1.0.0 |


## Track Interrupts (Overlapped)
```c++
int TrackInterrupts(unsigned interrupts, unsigned *matches, OVERLAPPED *o) throw(SystemException);

```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `interrupts` | `unsigned` | The interrupt(s) to track |
| `matches` | `unsigned *` | The interrupt(s) that happened |
| `o` | `OVERLAPPED *` | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx) |

| Return Value | Cause |
| ------------:| ----- |
| 0 | Success |
| ~ | Unknown system error |

###### Examples
```c++
#include <fscc.hpp>
...

unsigned matches;
OVERLAPPED o;

p.TrackInterrupts(0x00000400, &matches, &o);
```

## Track Interrupts (Blocking)
```c++
unsigned TrackInterrupts(unsigned interrupts) throw(SystemException);
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `interrupts` | `unsigned` | The interrupt(s) to track |

| Return |
| ------ |
| What interrupts where matched |


###### Examples
```c++
#include <fscc.hpp>
...

unsigned matches = p.TrackInterrupts(0x00000400);
```


## Track Interrupts (Timeout)
```c++
unsigned TrackInterrupts(unsigned interrupts, unsigned timeout) throw(SystemException);
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `interrupts` | `unsigned` | The interrupt(s) to track |
| `timeout` | `unsigned` | Number of milliseconds to wait for interrupts |

| Return |
| ------ |
| What interrupts where matched |


###### Examples
```c++
#include <fscc.hpp>
...

unsigned matches = p.TrackInterrupts(0x00000400, 100);
```


### Additional Resources
- Complete example: [`examples/track-interrupts.cpp`](../examples/track-interrupts.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)