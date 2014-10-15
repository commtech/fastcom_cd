# Append Timestamp
###### Windows
[`KeQuerySystemTime`](http://msdn.microsoft.com/en-us/library/windows/hardware/ff553068.aspx)
is used to acquire the timestamp upon complete reception of a frame.

###### Linux
[`do_gettimeofday`](http://www.fsl.cs.sunysb.edu/kernel-api/re29.html)
is used to acquire the timestamp upon complete reception of a frame.

_We will be moving to 
[`getnstimeofday`](http://www.gnugeneration.com/books/linux/2.6.20/kernel-api/re32.html)
in the 3.0 driver series._


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.4.0`
| `fscc-linux`   | `v2.4.0`
| `pyfscc`       | `v2.0.0`


## Property
```python
append_timestamp = property(...)
```


## Get
###### Examples
```python
import fscc
...

status = p.append_timestamp
```


## Enable
###### Examples
```python
import fscc
...

p.append_timestamp = True
```


## Disable
###### Examples
```python
import fscc
...

p.append_timestamp = False
```


### Additional Resources
- Complete example: [`examples\append-timestamp.py`](https://github.com/commtech/pyfscc/blob/master/examples/append-timestamp.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
