# Memory Cap

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 
| `fscc-linux`   | `v2.0.0` 
| `pyfscc`       | `v1.0.0`


## Property
```python
memory_cap = property(...)
```


## Get
###### Examples
```python
import fscc
...

input_cap = p.memory_cap.input
output_cap = p.memory_cap.output
```


## Set
###### Examples
```python
import fscc
...

p.memory_cap.input = 1000000
p.memory_cap.output = 2000000
```


### Additional Resources
- Complete example: [`examples\memory-cap.py`](https://github.com/commtech/pyfscc/blob/master/examples/memory-cap.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
