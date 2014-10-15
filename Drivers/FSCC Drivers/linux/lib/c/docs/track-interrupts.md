# Track Interrupts


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.7.0` 
| `cfscc`        | `v1.0.0`


## Track Interrupts (Overlapped)
```c
int fscc_track_interrupts(fscc_handle h, unsigned interrupts, unsigned *matches, OVERLAPPED *o)
```

| Parameter    | Type           | Description
| ------------ | -------------- | ------------------------------------
| `h`          | `fscc_handle`  | The handle to your port
| `interrupts` | `unsigned`     | The interrupt(s) to track
| `matches`    | `unsigned *`   | The interrupt(s) that happened
| `o`          | `OVERLAPPED *` | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx)

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned matches;

/* TIN interrupt */
fscc_track_interrupts(h, 0x00000100, &matches, NULL);
```


## Track Interrupts (Blocking)
```c
int fscc_track_interrupts_with_blocking(fscc_handle h, unsigned interrupts, unsigned *matches)
```

| Parameter    | Type           | Description
| ------------ | -------------- | -----------------------
| `h`          | `fscc_handle`  | The handle to your port
| `interrupts` | `unsigned`     | The interrupt(s) to track
| `matches`    | `unsigned *`   | The interrupt(s) that happened

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned matches;

/* TIN interrupt */
fscc_track_interrupts_with_blocking(h, 0x00000100, &matches);
```


## Track Interrupts (Timeout)
```c
int fscc_track_interrupts_with_timeout(fscc_handle h, unsigned interrupts, unsigned *matches, unsigned timeout)
```

| Parameter    | Type          | Description
| ------------ | ------------- | -----------------------
| `h`          | `fscc_handle` | The handle to your port
| `interrupts` | `unsigned`    | The interrupt(s) to track
| `matches`    | `unsigned *`  | The interrupt(s) that happened
| `timeout`    | `unsigned`    | Number of milliseconds to wait for interrupts

| Return Value | Cause
| ------------ | -------
| `0`          | Success

###### Examples
```c
#include <fscc.h>
...

unsigned matches;

/* TIN interrupt */
fscc_track_interrupts_with_timeout(h, 0x00000100, &matches, 100);
```

### Additional Resources
- Complete example: [`examples\track-interrupts.c`](https://github.com/commtech/cfscc/blob/master/examples/track-interrupts.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
