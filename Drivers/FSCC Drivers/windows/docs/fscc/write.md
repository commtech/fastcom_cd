# Write


###### Support
| Code           | Version
| -------------- | --------
| `fscc-windows` | `v2.0.0` 


## Write
The Windows [`WriteFile`](http://msdn.microsoft.com/en-us/library/windows/desktop/aa365747.aspx)
is used to write data to the port.

###### Examples
```c
#include <fscc.h>
...

char odata[] = "Hello world!";
unsigned bytes_written;

WriteFile(h, odata, sizeof(odata), (DWORD*)bytes_written, NULL);
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/fscc-windows/blob/master/examples/tutorial.c)
