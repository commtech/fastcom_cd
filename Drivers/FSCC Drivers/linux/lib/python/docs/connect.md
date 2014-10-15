# Connect

Opening a handle using this API will only give you access to the
synchronous functionality of the card. You will need to use the COM ports
if you would like to use the asynchronous functionality.

###### Driver Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0`
| `fscc-linux`   | `v2.0.0`
| `pyfscc`       | `v1.0.0`


## Connect
```python
def __init__(self, port_num, append_status=True, append_timestamp=True)
```

| Exception            | Base Exception | Cause
| -------------------- | -------------- |-------------------------
| `PortNotFoundError`  | `OSError`      | Port not found
| `InvalidAccessError` | `OSError`      | Insufficient permissions

###### Examples
```python
import fscc
...

p = fscc.Port(0)
```


### Additional Resources
- Complete example: [`examples\tutorial.py`](https://github.com/commtech/pyfscc/blob/master/examples/tutorial.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
