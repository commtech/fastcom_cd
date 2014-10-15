# Append Timestamp

###### Windows
[`KeQuerySystemTime`](http://msdn.microsoft.com/en-us/library/windows/hardware/ff553068.aspx)
is used to acquire the timestamp upon complete reception of a frame.

###### Linux
[`do_gettimeofday`](http://www.fsl.cs.sunysb.edu/kernel-api/re29.html)
is used to acquire the timestamp upon complete reception of a frame.

_We will be moving to 
[`getnstimeofday`](http://www.gnugeneration.com/books/linux/2.6.20/kernel-api/re32.html)
in the 3.0 driver series._

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.4.0` 
| `fscc-linux`   | `v2.4.0` 
| `cfscc`        | `v1.0.0`


## Get
```c
int fscc_get_append_timestamp(fscc_handle h, unsigned *status)
```

| Parameter | Type             | Description
| --------- | ---------------- | ------------------------------------
| `h`       | `fscc_handle`    | The handle to your port
| `status`  | `unsigned int *` | The current `append timestamp` value

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned status;

fscc_get_append_timestamp(h, &status);
```


## Enable
```c
int fscc_enable_append_timestamp(fscc_handle h)
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

fscc_enable_append_timestamp(h);
```


## Disable
```c
int fscc_disable_append_timestamp(fscc_handle h)
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

fscc_disable_append_timestamp(h);
```


### Additional Resources
- Complete example: [`examples\append-timestamp.c`](https://github.com/commtech/cfscc/blob/master/examples/append-timestamp.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
