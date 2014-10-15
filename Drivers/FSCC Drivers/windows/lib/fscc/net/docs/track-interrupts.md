# Track Interrupts


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.7.0` 
| `netfscc`      | `v1.0.0`


## Track Interrupts (Overlapped)
```c#
int TrackInterrupts(uint interrupts, out *matches, out NativeOverlapped o);
```

| Parameter    | Type                   | Description
| ------------ | ---------------------- | -----------------------
| `interrupts` | `uint`                 | The interrupt(s) to track
| `matches`    | `out uint`             | The interrupt(s) that happened
| `o`          | `out NativeOverlapped` | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx)

| Return Value | Cause
| ------------ | ------------------------------------------------------------------
| 0            | Success
| ~            | Unknown system error

###### Examples
```c#
using Fscc;
...

var NativeOverlapped o;
uint matches;

int e = p.TrackInterrupts(0x00000400, out matches, out o);
```


## Track Interrupts (Blocking)
```c#
uint TrackInterrupts(uint interrupts)
```

| Parameter    | Type   | Description
| ------------ | ------ | -----------------------
| `interrupts` | `uint` | The interrupt(s) to track

| Return
| ---------------------------
| What interrupts where matched

###### Examples
```c#
using Fscc;
...

var matches = 0;

matches = p.TrackInterrupts(0x00000400);
```


## Track Interrupts (Timeout)
```c#
uint TrackInterrupts(uint interrupts, uint timeout)
```

| Parameter    | Type   | Description
| ------------ | ------ | -----------------------
| `interrupts` | `uint` | The interrupt(s) to track
| `timeout`    | `uint` | Number of milliseconds to wait for interrupts

| Return
| ---------------------------
| What interrupts where matched

###### Examples
```c#
using Fscc;
...

var matches = 0;

matches = p.TrackInterrupts(0x00000400, 100);
```


### Additional Resources
- Complete example: [`examples\track-interrupts.cs`](https://github.com/commtech/netfscc/blob/master/examples/track-interrupts.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
