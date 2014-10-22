# TX Modifiers

| Modifiers | Value | Description |
| --------- | -----:| ----------- |
| `XF` | 0 | Normal transmit (disable modifiers) |
| `XREP` | 1 | Transmit frame repeatedly |
| `TXT` | 2 | Transmit frame on timer |
| `TXEXT` | 4 | Transmit frame on external signal |


###### Support
| Code  | Version |
| ----- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| netfscc | 1.0.0 |


## Property
```c#
TxModifiers TxModifiers { get {...}, set{...} };
```


## Get
###### Examples
```c#
using Fscc;
...

var modifiers = p.TxModifiers;
```


## Set
###### Examples
```c#
using Fscc;
...

p.TxModifiers = Fscc.TxModifiers.TXT | Fscc.TxModifiers.XREP;
```


### Additional Resources
- Complete example: [`examples/tx-modifiers.cs`](../examples/tx-modifiers.cs)
- Implementation details: [`src/Fscc.cs`](../src/Fscc.cs)
