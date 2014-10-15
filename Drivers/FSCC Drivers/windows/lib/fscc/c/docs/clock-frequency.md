# Clock Frequency
The FSCC device has a programmable clock that can be set anywhere from
20 KHz to 200 MHz. However, this is not the full operational range of an
FSCC port, only the range that the onboard clock can be set to.

Using one of the synchronous modes you can only receive data consistently
up to 30 MHz (when you are using an external clock). If you are transmitting
data using an internal clock, you can safely receive data consistently up to 50 MHz.

Lower clock rates (less than 1 MHz for example) can take a long time for 
the frequency generator to finish. If you run into this situation we 
recommend using a larger frequency and then dividing it down to your 
desired baud rate using the BGR register.

_If you are receiving timeout errors when using slow data rates you can bypass
the safety checks by using the 
[`fscc_enable_ignore_timeout()`](https://github.com/commtech/cfscc/blob/master/docs/ignore-timeout.md)
option._

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Set
```c
int fscc_set_clock_frequency(fscc_handle h, unsigned frequency)
```

| Parameter    | Type          | Description
| ------------ | ------------- | ----------------------------
| `h`          | `fscc_handle` | The handle to your port
| `frequency`  | `unsigned`    | The new clock frequency (hz)

| Return Value             | Value   | Cause
| ------------------------ | ------- | --------------------------------
| `0`                      | `0`     |
| `FSCC_INVALID_PARAMETER` | `16005` | Clock frequency is out of range (15,000 to 270,000,000)


###### Examples
Set the port's clock frequency to 18.432 MHz.
```c
#include <fscc.h>
...

fscc_set_clock_frequency(h, 18432000);
```


### Additional Resources
- Complete example: [`examples\clock-frequency.c`](https://github.com/commtech/cfscc/blob/master/examples/clock-frequency.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
