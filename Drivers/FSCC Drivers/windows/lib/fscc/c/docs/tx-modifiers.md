# TX Modifiers

| Modifiers | Value | Description
| -------- | ------ | -----------
| `XF`     | `0`    | Normal transmit (disable modifiers)
| `XREP`   | `1`    | Transmit frame repeatedly
| `TXT`    | `2`    | Transmit frame on timer
| `TXEXT`  | `4`    | Transmit frame on external signal

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`

## Get
```c
int fscc_get_tx_modifiers(fscc_handle h, unsigned *modifiers)
```

| Parameter    | Type             | Description
| ------------ | ---------------- | --------------------------------
| `h`          | `fscc_handle`    | The handle to your port
| `modifiers`  | `unsigned int *` | The current `tx modifiers` value

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```
#include <fscc.h>
...

unsigned modifiers;

fscc_get_tx_modifiers(h, &modifiers);
```


## Set
```c
int fscc_set_tx_modifiers(fscc_handle h, unsigned modifiers)
```

| Parameter   | Type           | Description
| ----------- | -------------- | -----------------------------------------
| `h`         | `fscc_handle`  | The handle to your port
| `modifiers` | `unsigned int` | What `tx modifiers` you would like to set

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```
#include <fscc.h>
...

fscc_set_tx_modifiers(h, TXT | XREP);
```


### Additional Resources
- Complete example: [`examples\tx-modifiers.c`](https://github.com/commtech/cfscc/blob/master/examples/tx-modifiers.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
