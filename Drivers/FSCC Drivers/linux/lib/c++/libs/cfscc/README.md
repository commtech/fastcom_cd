# FSCC C Library (cfscc)
This README file is best viewed [online](http://github.com/commtech/cfscc/).

## Installing Library

##### Downloading Library
- You can use the pre-built library files that are included with the driver
- Or, you can download the latest library version from
[Github releases](https://github.com/commtech/cfscc/releases).


## Quick Start Guide

Lets get started with a quick programming example for fun.

_This tutorial has already been set up for you at_ 
[`examples/tutorial.c`](https://github.com/commtech/cfscc/tree/master/examples/tutorial.c).

First, drop `cfscc.dll` and `cfscc.lib` into a test directory. Now that those files are 
copied over, create a new C file (named tutorial.c) with the following code.

```c
#include <stdio.h> /* fprintf */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <fscc.h> /* fscc_* */

int main(void)
{
    fscc_handle h;
    int e = 0;
    char odata[] = "Hello world!";
    char idata[20] = {0};
    unsigned tmp;

    /* Open port 0 in a blocking IO mode */
    e = fscc_connect(0, &h);
    if (e != 0) {
        fprintf(stderr, "fscc_connect failed with %d\n", e);
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    fscc_write_with_blocking(h, odata, sizeof(odata), &tmp, NULL);

    /* Read the data back in (with our loopback connector) */
    fscc_read_with_blocking(h, idata, sizeof(idata), &tmp, NULL);

    fprintf(stdout, "%s\n", idata);

    fscc_disconnect(h);

    return EXIT_SUCCESS;
}
```

For this example I will use the Visual Studio command line compiler, but
you can use your compiler of choice.

```
# cl tutorial.c cfscc.lib /I <CFSCC_DIR>\src\
```

Now attach the included loopback connector.

```
# tutorial.exe
Hello world!
```

You have now transmitted and received an HDLC frame! 


## API Reference

There are likely other configuration options you will need to set up for your 
own program. All of these options are described on their respective documentation page.

- [Connect](https://github.com/commtech/cfscc/blob/master/docs/connect.md)
- [Append Status](https://github.com/commtech/cfscc/blob/master/docs/append-status.md)
- [Append Timestamp](https://github.com/commtech/cfscc/blob/master/docs/append-timestamp.md)
- [Clock Frequency](https://github.com/commtech/cfscc/blob/master/docs/clock-frequency.md)
- [Ignore Timeout](https://github.com/commtech/cfscc/blob/master/docs/ignore-timeout.md)
- [RX Multiple](https://github.com/commtech/cfscc/blob/master/docs/rx-multiple.md)
- [Memory Cap](https://github.com/commtech/cfscc/blob/master/docs/memory-cap.md)
- [Purge](https://github.com/commtech/cfscc/blob/master/docs/purge.md)
- [Registers](https://github.com/commtech/cfscc/blob/master/docs/registers.md)
- [TX Modifiers](https://github.com/commtech/cfscc/blob/master/docs/tx-modifiers.md)
- [Track Interrupts](https://github.com/commtech/cfscc/blob/master/docs/track-interrupts.md)
- [Write](https://github.com/commtech/cfscc/blob/master/docs/write.md)
- [Read](https://github.com/commtech/cfscc/blob/master/docs/read.md)
- [Disconnect](https://github.com/commtech/cfscc/blob/master/docs/disconnect.md)


## Visual Studio Integration
The process for using the library will differ slighly depending on the version of Visual Studio.

1. Configuration Properties -> C/C++ -> General = fscc\bin\fscc\lib\fscc\c (path to .h file)
2. Configuration Properties -> Linker -> General = fscc\bin\fscc\lib\fscc\c (path to .lib file)
3. Configuration Properties -> Linker -> Input -> Additional Dependencies = cfscc.lib
4. Place cfscc.dll into the same folder as your executable


## API Compatibility
We follow [Semantic Versioning](http://semver.org/) when creating releases.


## License

Copyright (C) 2013 [Commtech, Inc.](http://commtech-fastcom.com)

Licensed under the [GNU General Public License v3](http://www.gnu.org/licenses/gpl.txt).
