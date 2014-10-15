# Write


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cfscc`        | `v1.0.0`


## Write (Overlapped)
```c
int fscc_write(fscc_handle h, char *buf, unsigned size, unsigned *bytes_written, OVERLAPPED *o)
```

| Parameter    | Type             | Description
| ------------ | ---------------- | -----------------------------------------------------------------------------------------------
| `h`          | `fscc_handle`    | The handle to your port
| `buf`        | `char *`         | The data buffer to transmit
| `size`       | `unsigned`       | The number of bytes to transmit
| `bytes_read` | `unsigned *`     | How many bytes were written to the read
| `o`          | `OVERLAPPED *`   | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx)

| Return Value            | Value   | Cause
| ----------------------- | ------- | ----------------------------------------------------
| `0`                     | `0`     | Success
| `FSCC_BUFFER_TOO_SMALL` | `16002` | The write size exceeds the output memory usage cap
| `FSCC_TIMEOUT`          | `16000` | Command timed out (missing clock)
| `FSCC_INCORRECT_MODE`   | `16001` | Using the synchronous port while in asynchronous mode

###### Examples
```c
#include <fscc.h>
...

char odata[] = "Hello world!";
unsigned bytes_written;

fscc_write(h, odata, sizeof(odata), &bytes_written, NULL);
```


## Write (Blocking)
```c
int fscc_write_with_blocking(fscc_handle h, char *buf, unsigned size, unsigned *bytes_written)
```

| Parameter    | Type             | Description
| ------------ | ---------------- | -----------------------------------------------------------------------------------------------
| `h`          | `fscc_handle`    | The handle to your port
| `buf`        | `char *`         | The data buffer to transmit
| `size`       | `unsigned`       | The number of bytes to transmit
| `bytes_read` | `unsigned *`     | How many bytes were written to the read

| Return Value            | Value   |  Cause
| ----------------------- | ------- |  --------------------------------------------------
| `0`                     | `0`     | Success
| `FSCC_BUFFER_TOO_SMALL` | `16002` | The write size exceeds the output memory usage cap
| `FSCC_TIMEOUT`          | `16000` | Command timed out (missing clock)
| `FSCC_INCORRECT_MODE`   | `16001` | Using the synchronous port while in asynchronous mode

###### Examples
```c
#include <fscc.h>
...

char odata[] = "Hello world!";
unsigned bytes_written;

fscc_write_with_blocking(h, odata, sizeof(odata), &bytes_written);
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/cfscc/blob/master/examples/tutorial.c)
- Implemenation details: [`src\fscc.c`](https://github.com/commtech/cfscc/blob/master/src/fscc.c)
