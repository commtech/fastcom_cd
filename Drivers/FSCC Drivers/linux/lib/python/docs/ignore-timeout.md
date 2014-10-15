# Ignore Timeout

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0`
| `fscc-linux`   | `v2.0.0`
| `pyfscc`       | `v1.0.0`


## Property
```python
ignore_timeout = property(...)
```


## Get
###### Examples
```python
import fscc
...

status = p.ignore_timeout
```


## Enable
###### Examples
```python
import fscc
...

p.ignore_timeout = True
```


## Disable
###### Examples
```python
import fscc
...

p.ignore_timeout = False
```


### Additional Resources
- Complete example: [`examples\ignore-timeout.py`](https://github.com/commtech/pyfscc/blob/master/examples/ignore-timeout.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
