# Purge

Between the hardware FIFO and the driver's software buffers there are multiple places data could  be stored, excluding your application code. If you ever need to clear this data and start fresh, there are a couple of methods you can use.

_A `Purge()` (receive side) is required after changing the `MODE` bits in the `CCR0` register. If you need to change the `MODE` bits but don't have a clock present, change the `CM` bits to `0x7` temporarily. This will give you an internal clock to switch modes. You can then switch to your desired `CM` now that your `MODE` is locked in._

###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


## Execute
```c
void Purge(bool tx, bool rx);
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `tx` | `bool` | Whether to purge the transmit data |
| `rx` | `bool` | Whether to purge the receive data |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `TimeoutException` | `SystemException` | Command timed out (missing clock) |


```c
void Purge();
```

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `TimeoutException` | `SystemException` | You are executing a command that requires a transmit clock present |
| `BufferTooSmallException` | `SystemException` | The read size is smaller than the next frame (in a frame based mode) |


This version purges both the transmit and receive sides.


###### Examples
Purge both the transmit and receive data.
```c#
using Fscc;
...

p.Purge(true, true)
```

Purge only the transmit data.
```c#
using Fscc;
...

p.Purge(true, false)
```

Purge only the receive data.
```c#
using Fscc;
...

p.Purge(false, true)
```


### Additional Resources
- Complete example: [`examples/purge.cs`](../examples/purge.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
