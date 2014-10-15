# Write


###### Driver Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `pyfscc`       | `v1.0.0`


## Write
```python
def write(self, data):
```

| Parameter    | Type    | Description
| ------------ | ------- | ---------------------------
| `buf`        | `bytes` | The data buffer to transmit

| Return
| ---------------------------
| Number of bytes transmitted

| Exception             | Base Exception | Cause
| --------------------- | -------------- | ----------------------------------------------------
| `BufferTooSmallError` | `OSError`      | The write size exceeds the output memory usage cap
| `TimeoutError`        | `OSError`      | Command timed out (missing clock)
| `IncorrectModeError`  | `OSError`      | Using the synchronous port while in asynchronous mode

###### Examples
```c
import fscc
...

p.write(b'Hello world!')
```


### Additional Resources
- Complete example: [`examples\tutorial.py`](https://github.com/commtech/pyfscc/blob/master/examples/tutorial.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
