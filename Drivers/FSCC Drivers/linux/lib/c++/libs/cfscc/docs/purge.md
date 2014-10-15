# Purge
Between the hardware FIFO and the driver's software buffers there are multiple places data could 
be stored, excluding your application code. If you ever need to clear this data and start fresh, 
there are a couple of methods you can use.

_An `fscc_purge()` (receive side)
is required after changing the `MODE` bits in the `CCR0` register. If you need to change
the `MODE` bits but don't have a clock present, change the `CM` bits to `0x7` temporarily. This will give 
you an internal clock to switch modes. You can then switch to your desired `CM` now that your `MODE` is 
locked in._

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Execute
```c
fscc_purge(fscc_handle h, unsigned tx, unsigned rx)
```

| Parameter | Type           | Description
| --------- | -------------- | ----------------------------------
| `h`       | `fscc_handle`  | The handle to your port
| `tx`      | `unsigned int` | Whether to purge the transmit data
| `rx`      | `unsigned int` | Whether to purge the receive data

| Return Value   | Value | Cause
| -------------- | ----- | ------------------------------------------------------------------
| `0`            | `0`   | Success
| `FSCC_TIMEOUT` | 16000 | Command timed out (missing clock)

###### Examples
Purge both the transmit and receive data.
```c
#include <fscc.h>
...

fscc_purge(h, 1, 1);
```

Purge only the transmit data.
```c
#include <fscc.h>
...

fscc_purge(h, 1, 0);
```

Purge only the receive data.
```c
#include <fscc.h>
...

fscc_purge(h, 0, 1);
```


### Additional Resources
- Complete example: [`examples\purge.c`](https://github.com/commtech/cfscc/blob/master/examples/purge.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
