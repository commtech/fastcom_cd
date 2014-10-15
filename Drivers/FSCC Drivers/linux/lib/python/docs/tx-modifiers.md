# TX Modifiers

| Modifiers | Value | Description
| -------- | ------ | -----------
| `XF`     | `0`    | Normal transmit (disable modifiers)
| `XREP`   | `1`    | Transmit frame repeatedly
| `TXT`    | `2`    | Transmit frame on timer
| `TXEXT`  | `4`    | Transmit frame on external signal

###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0`
| `fscc-linux`   | `v2.0.0`
| `pyfscc`       | `v1.0.0`


## Property
```python
tx_modifiers = property(...)
```


## Get
###### Examples
```python
import fscc
...

modifiers = p.tx_modifiers
```


## Set
###### Examples
```python
import fscc
...

p.tx_modifiers = TXT | XREP
```


### Additional Resources
- Complete example: [`examples\tx-modifiers.py`](https://github.com/commtech/pyfscc/blob/master/examples/tx-modifiers.py)
- Implemenation details: [`fscc.py`](https://github.com/commtech/pyfscc/blob/master/fscc.py)
