# Append Status

It is a good idea to pay attention to the status of each frame. For example, you
may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes
to each frame you read from the card, if you opt-in to see this data. There are
a few methods of enabling this additional data.

###### Driver Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Get
```c
int fscc_get_append_status(fscc_handle h, unsigned *status)
```

| Parameter | Type             | Description
| --------- | ---------------- | ---------------------------------
| `h`       | `fscc_handle`    | The handle to your port
| `status`  | `unsigned int *` | The current `append status` value

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned status;

fscc_get_append_status(h, &status);
```


## Enable
```c
int fscc_enable_append_status(fscc_handle h)
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

fscc_enable_append_status(h);
```


## Disable
```c
int fscc_disable_append_status(fscc_handle h)
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

fscc_disable_append_status(h);
```


### Additional Resources
- Complete example: [`examples\append-status.c`](https://github.com/commtech/cfscc/blob/master/examples/append-status.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
