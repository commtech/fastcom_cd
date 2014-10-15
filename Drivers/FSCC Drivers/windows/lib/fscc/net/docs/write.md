# Write


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `netfscc`      | `v1.0.0`


## Write
```c#
int Write(byte[] buf, uint size, out NativeOverlapped o);
```

| Parameter    | Type                   | Description
| ------------ | ---------------------- | -----------------------
| `buf`        | `byte[]`               | The data buffer to transmit
| `size`       | `uint`                 | The number of bytes to transmit
| `o`          | `out NativeOverlapped` | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx)

| Exception                 | Base Exception    | Cause
| ------------------------- | ----------------- | --------------------------------------------------
| `BufferTooSmallException` | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`        | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`  | `SystemException` | Using the synchronous port while in asynchronous mode


###### Examples
```c#
using Fscc;
...

var odata = new byte[20];
var bytes_written = 0;

bytes_written = p.Write(odata, odata.Length, o);
```

## Write
```c#
uint Write(byte[] buf, uint size);
```

| Parameter    | Type             | Description
| ------------ | ---------------- | -----------------------
| `buf`        | `byte[]`               | The data buffer to transmit
| `size`       | `uint`                 | The number of bytes to transmit

| Return
| ---------------------------
| Number of bytes transmitted

| Exception                 | Base Exception    | Cause
| ------------------------- | ----------------- | --------------------------------------------------
| `BufferTooSmallException` | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`        | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`  | `SystemException` | Using the synchronous port while in asynchronous mode

###### Examples
```c#
using Fscc;
...

var odata = new byte[20];
var bytes_written = 0;

bytes_written = p.Write(odata, odata.Length);
```

## Write
```c#
uint Write(string str);
```

| Parameter | Type     | Description
| --------- | -------- | -----------------------
| `str`     | `string` | The string to transmit

| Return
| ---------------------------
| Number of bytes transmitted

| Exception                 | Base Exception    | Cause
| ------------------------- | ----------------- | --------------------------------------------------
| `BufferTooSmallException` | `SystemException` | The write size exceeds the output memory usage cap
| `TimeoutException`        | `SystemException` | Command timed out (missing clock)
| `IncorrectModeException`  | `SystemException` | Using the synchronous port while in asynchronous mode

###### Examples
```c#
using Fscc;
...

string odata = "Hello world!";
var bytes_written = 0;

bytes_written = p.Write(odata);
```


### Additional Resources
- Complete example: [`examples\tutorial.cs`](https://github.com/commtech/netfscc/blob/master/examples/tutorial.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
