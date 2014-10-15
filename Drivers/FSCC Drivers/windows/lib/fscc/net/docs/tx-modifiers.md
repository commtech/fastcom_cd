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
| `netfscc`      | `v1.0.0`


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
- Complete example: [`examples\tx-modifiers.cs`](https://github.com/commtech/netfscc/blob/master/examples/tx-modifiers.cs)
- Implemenation details: [`src\Fscc.cs`](https://github.com/commtech/netfscc/blob/master/src/Fscc.cs)
