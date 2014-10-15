# RX Multiple

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.2.9` 
| `fscc-linux`   | `v2.2.4` 
| `cfscc`        | `v1.0.0`


## Get
```c
int fscc_get_rx_multiple(fscc_handle h, unsigned *status)
```

| Parameter | Type             | Description
| --------- | ---------------- | -----------------------
| `h`       | `fscc_handle`    | The handle to your port
| `status`  | `unsigned int *` | The current `rx multiple` value

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned status;

fscc_get_rx_multiple(h, &status);
```


## Enable
```c
int fscc_enable_rx_multiple(fscc_handle h)
```

| Parameter | Type             | Description
| --------- | ---------------- | -----------------------
| `h`       | `fscc_handle`    | The handle to your port

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

fscc_enable_rx_multiple(h);
```


## Disable
```c
int fscc_disable_rx_multiple(fscc_handle h)
```

| Parameter | Type             | Description
| --------- | ---------------- | -----------------------
| `h`       | `fscc_handle`    | The handle to your port

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

fscc_disable_rx_multiple(h);
```


### Additional Resources
- Complete example: [`examples\rx-multiple.c`](https://github.com/commtech/cfscc/blob/master/examples/rx-multiple.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
