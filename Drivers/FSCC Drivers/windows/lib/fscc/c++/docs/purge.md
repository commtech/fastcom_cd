# Purge

Between the hardware FIFO and the driver's software buffers there are multiple places data could  be stored, excluding your application code. If you ever need to clear this data and start fresh, there are a couple of methods you can use.

_A `Purge()` (receive side) is required after changing the `MODE` bits in the `CCR0` register. If you need to change the `MODE` bits but don't have a clock present, change the `CM` bits to `0x7` temporarily. This will give you an internal clock to switch modes. You can then switch to your desired `CM` now that your `MODE` is locked in._

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| cppfscc | 1.0.0 |


## Execute
```c++
void Purge(bool tx=true, bool rx=true) throw(SystemException);
```

| Parameter | Type | Default | Description |
| --------- | ---- | ------- | ----------- |
| `tx` | `bool` | `true` | Whether to purge the transmit data |
| `rx` | `bool` | `true` | Whether to purge the receive data |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `TimeoutException` | `SystemException` | Command timed out (missing clock) |

###### Examples
Purge both the transmit and receive data.
```c++
#include <fscc.hpp>
...

p.Purge(true, true);
```

Purge only the transmit data.
```c++
#include <fscc.hpp>
...

p.Purge(true, false);
```

Purge only the receive data.
```c++
#include <fscc.hpp>
...

p.Purge(false, true);
```


### Additional Resources
- Complete example: [`examples/purge.cpp`](../examples/purge.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)