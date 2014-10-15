# Ignore Timeout

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Get
```c
int fscc_get_ignore_timeout(fscc_handle h, unsigned *status)
```

| Parameter | Type             | Description
| --------- | ---------------- | ----------------------------------
| `h`       | `fscc_handle`    | The handle to your port
| `status`  | `unsigned int *` | The current `ignore timeout` value

| Return Value | Cause
| ------------ | -------
| `0`            | Success


###### Examples
```c
#include <fscc.h>
...

unsigned status;

fscc_get_ignore_timeout(h, &status);
```


## Enable
```c
int fscc_enable_ignore_timeout(fscc_handle h)
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

fscc_enable_ignore_timeout(h);
```


## Disable
```c
int fscc_disable_ignore_timeout(fscc_handle h)
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

fscc_disable_ignore_timeout(h);
```


### Additional Resources
- Complete example: [`examples\ignore-timeout.c`](https://github.com/commtech/cfscc/blob/master/examples/ignore-timeout.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
