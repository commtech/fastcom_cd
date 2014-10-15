# Connect

Opening a handle using this API will only give you access to the
synchronous functionality of the card. You will need to use the COM ports
if you would like to use the asynchronous functionality.

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Connect
```c
int fscc_connect(unsigned port_num, fscc_handle *h)
```

| Parameter     | Type           | Description
| ------------ | --------------- | --------------------------------------------
| `port_num    | `unsigned`      | The port number you would like to connect to
| `h`          | `fscc_handle *` | The handle to your port

| Return Value          | Value   | Cause
| --------------------- | ------- | -----------------------
| `0`                   | `0`     | Success
| `FSCC_PORT_NOT_FOUND` | `16003` | Port not found
| `FSCC_INVALID_ACCESS` | `16004` | Insufficient permisions

###### Examples
Connect to port 0.

```c
#include <fscc.h>
...

HANDLE h;

fscc_connect(0, &h);
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/cfscc/blob/master/examples/tutorial.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
