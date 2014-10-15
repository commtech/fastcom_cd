# FSCC C++ Library (cppfscc)
This README file is best viewed [online](http://github.com/commtech/cppfscc/).

## Installing Library

##### Downloading Library
- You can use the pre-built library files that are included with the driver
- Or, you can download the latest library version from
[Github releases](https://github.com/commtech/cppfscc/releases).


## Quick Start Guide

Lets get started with a quick programming example for fun.

_This tutorial has already been set up for you at_ 
[`examples/tutorial.cpp`](https://github.com/commtech/cppfscc/tree/master/examples/tutorial.cpp).

First, drop `cppfscc.dll`, `cppfscc.lib` and `cfscc.dll` into a test directory. Now that those 
files are copied over, create a new C++ file (named tutorial.cpp) with the following code.

```c++
#include <iostream> /* cout, endl */
#include <cstdlib> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <fscc.hpp> /* fscc_connect, fscc_disconnect, fscc_handle
                       fscc_write, fscc_read */

int main(void)
{
    Fscc::Port p(0);

    // Send "Hello world!" text
    p.Write("Hello world!");

    // Read the data back in (with our loopback connector)
    std::cout << p.Read() << std::endl;

    return EXIT_SUCCESS;
}
```

For this example I will use the Visual Studio command line compiler, but
you can use your compiler of choice.

```
# cl /EHsc tutorial.cpp cppfscc.lib /I <CPPFSCC_DIR>\src
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

- [Connect](https://github.com/commtech/cppfscc/blob/master/docs/connect.md)
- [Append Status](https://github.com/commtech/cppfscc/blob/master/docs/append-status.md)
- [Append Timestamp](https://github.com/commtech/cppfscc/blob/master/docs/append-timestamp.md)
- [Clock Frequency](https://github.com/commtech/cppfscc/blob/master/docs/clock-frequency.md)
- [Ignore Timeout](https://github.com/commtech/cppfscc/blob/master/docs/ignore-timeout.md)
- [RX Multiple](https://github.com/commtech/cppfscc/blob/master/docs/rx-multiple.md)
- [Memory Cap](https://github.com/commtech/cppfscc/blob/master/docs/memory-cap.md)
- [Purge](https://github.com/commtech/cppfscc/blob/master/docs/purge.md)
- [Registers](https://github.com/commtech/cppfscc/blob/master/docs/registers.md)
- [TX Modifiers](https://github.com/commtech/cppfscc/blob/master/docs/tx-modifiers.md)
- [Track Interrupts](https://github.com/commtech/cppfscc/blob/master/docs/track-interrupts.md)
- [Write](https://github.com/commtech/cppfscc/blob/master/docs/write.md)
- [Read](https://github.com/commtech/cppfscc/blob/master/docs/read.md)


## Dependencies
- [FSCC C Library](https://github.com/commtech/cfscc/)


## API Compatibility
We follow [Semantic Versioning](http://semver.org/) when creating releases.


## License

Copyright (C) 2013 [Commtech, Inc.](http://commtech-fastcom.com)

Licensed under the [GNU General Public License v3](http://www.gnu.org/licenses/gpl.txt).
