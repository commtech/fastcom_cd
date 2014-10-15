# Write


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `cppfscc`      | `v1.0.0`


## Write
```c++
int Write(const char *buf, unsigned size, OVERLAPPED *o) throw(SystemException);
```

| Parameter    | Type             | Description
| ------------ | ---------------- | -----------------------
| `buf`        | `const char *`   | The data buffer to transmit
| `size`       | `unsigned`       | The number of bytes to transmit
| `o`          | `OVERLAPPED *`   | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx)

| Exception                   | Base Exception  | Cause
| --------------------------- | --------------- | --------------------------------------------------
| `BufferTooSmallException`   | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`          | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`    | `SystemException` | Using the synchronous port while in asynchronous mode


###### Examples
```c++
#include <fscc.hpp>
...

char odata[] = "Hello world!";
OVERLAPPED o;

p.Write(odata, sizeof(odata), &o);
```

## Write
```c++
unsigned Write(const char *buf, unsigned size) throw(SystemException);
```

| Parameter    | Type             | Description
| ------------ | ---------------- | -----------------------
| `buf`        | `const char *`   | The data buffer to transmit
| `size`       | `unsigned`       | The number of bytes to transmit

| Return
| ---------------------------
| Number of bytes transmitted

| Exception                   | Base Exception    | Cause
| --------------------------- | ----------------- | --------------------------------------------------
| `BufferTooSmallException`   | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`          | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`    | `SystemException` | Using the synchronous port while in asynchronous mode

###### Examples
```c++
#include <fscc.hpp>
...

char odata[] = "Hello world!";
unsigned bytes_written = 0;

p.Write(odata, sizeof(odata));
```

## Write
```c++
unsigned Write(const std::string &str) throw(SystemException);
```

| Parameter | Type                | Description
| --------- | ------------------- | -----------------------
| `str`     | `const std::string` | The string to transmit

| Return
| ---------------------------
| Number of bytes transmitted

| Exception                   | Base Exception    | Cause
| --------------------------- | ----------------- | --------------------------------------------------
| `BufferTooSmallException`   | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`          | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`    | `SystemException` | Using the synchronous port while in asynchronous mode

###### Examples
```c++
#include <fscc.hpp>
...

std::string odata("Hello world!");
unsigned bytes_written = 0;

bytes_written = p.Write(odata);
```


### Additional Resources
- Complete example: [`examples\tutorial.cpp`](https://github.com/commtech/cppfscc/blob/master/examples/tutorial.cpp)
- Implemenation details: [`src\fscc.cpp`](https://github.com/commtech/cppfscc/blob/master/src/fscc.cpp)
