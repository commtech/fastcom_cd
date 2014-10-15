# Purge
Between the hardware FIFO and the driver's software buffers there are multiple
places data could be stored, excluding your application code. If you ever need
to clear this data and start fresh, there are a couple of methods you can use.

_A `purge()` (receive side)
is required after changing the `MODE` bits in the `CCR0` register. If you need to change
the `MODE` bits but don't have a clock present, change the `CM` bits to `0x7` temporarily. This will give 
you an internal clock to switch modes. You can then switch to your desired `CM` now that your `MODE` is 
locked in._

###### Driver Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0`
| `fscc-linux`   | `v2.0.0`
| `pyfscc`       | `v1.0.0`


## Execute
```python
def purge(self, tx=True, rx=True)
```

| Parameter | Type      | Default | Description
| --------- | --------- | ------- | ----------------------------------
| `tx`      | `Boolean` | True    | Whether to purge the transmit data
| `rx`      | `Boolean` | True    | Whether to purge the receive data

| Exception      | Base Exception | Cause
| -------------- | -------------- | ---------------------------------
| `TimeoutError` | `OSError`      | Command timed out (missing clock)

###### Examples
Purge both the transmit and receive data.
```python
using Fscc;
...

p.Purge(True, True)
```

Purge only the transmit data.
```python
using Fscc;
...

p.Purge(True, False)
```

Purge only the receive data.
```python
using Fscc;
...

p.Purge(False, True)
```


### Additional Resources
- Complete example: [`examples\purge.py`](https://github.com/commtech/pyfscc/blob/master/examples/purge.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
