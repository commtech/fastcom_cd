# Read

The `size` argument of the various read functions means different things depending on the mode you are using.

In a frame based mode the `size` argument specifies the maximum frame `size` to return. If the next queued frame is larger than the size you specified the error `FSCC_BUFFER_TOO_SMALL` is returned and the data will remain waiting for a read of a larger value. If a `size` is specified that is larger than the length of multiple frames in queue, you will still only receive one frame per read call.

In streaming mode (no frame termination) the `size` argument specifies the maximum amount of data to return. If there are 100 bytes of streaming data in the card and you read with a `size` of 50, you will receive 50 bytes. If you do a read of 200 bytes, you will receive the 100 bytes available.

Frame based data and streaming data are kept separate within the driver. To understand what this means, first imagine the following scenario. You are in a frame based mode and receive a couple of frames. You then switch to streaming mode and receive a stream of data. When calling read you will receive the the streaming data until you switch back into a frame based mode and do a read.

Most users will want the advanced I/O capabilities included by using the Windows [OVERLAPPED I/O API](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx). We won't duplicate the documentation here, but for your reference, here is an [article](http://blogs.msdn.com/b/oldnewthing/archive/2011/02/02/10123392.aspx) on a common bug developers introduce while trying to cancel I/O operations when using OVERLAPPED I/O.

###### Support
| Code | Version |
| ---- | ------- |
| fscc-windows | 2.0.0 |
| fscc-linux | 2.0.0 |
| cppfscc | 1.0.0 |


## Read (Overlapped)
```c++
int Read(char *buf, unsigned size, OVERLAPPED *o) throw(SystemException);
```

| Parameter | Type  | Description |
| --------- | ----- | ----------- |
| `buf` | `char *` | The data buffer to hold incoming data |
| `size` | `unsigned` | The data buffer size |
| `o` | `OVERLAPPED *` | [Overlapped IO structure](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx) |

| Return Value | Cause |
| ------------:| ----- |
| 0 | Success |
| ~ | Unknown system error |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `BufferTooSmallException` | `SystemException` | The buffer size is smaller than the next frame |
| `IncorrectModeException` | `SystemException` | Using the synchronous port while in asynchronous mode |

###### Examples
```c++
#include <fscc.hpp>
...

char idata[20] = {0};
unsigned bytes_read;
OVERLAPPED o;

p.Read(idata, sizeof(idata), &o);
```

## Read (Blocking)
```c++
unsigned Read(char *buf, unsigned size) throw(SystemException);
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `buf` | `char *` | The data buffer to hold incoming data |
| `size` | `unsigned` | The data buffer size |

| Return |
| ------ |
| Number of bytes read |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `BufferTooSmallException` | `SystemException` | The buffer size is smaller than the next frame |
| `IncorrectModeException` | `SystemException` | Using the synchronous port while in asynchronous mode |

###### Examples
```c++
#include <fscc.hpp>
...

char idata[20] = {0};
unsigned bytes_read;

bytes_read = p.Read(idata, sizeof(idata));
```


## Read (Timeout)
```c++
unsigned Read(char *buf, unsigned size, unsigned timeout) throw(SystemException);
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `buf` | `char *` | The data buffer to hold incoming data |
| `size` | `unsigned` | The data buffer size |
| `timeout` | `unsigned` | Number of milliseconds to wait for data |

| Return |
| ------ |
| Number of bytes read |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `BufferTooSmallException` | `SystemException` | The buffer size is smaller than the next frame |
| `IncorrectModeException` | `SystemException` | Using the synchronous port while in asynchronous mode |

###### Examples
```c++
#include <fscc.hpp>
...

char idata[20] = {0};
unsigned bytes_read;

bytes_read = p.Read(idata, sizeof(idata), 100);
```


## Read (Blocking)
```c++
public std::string Read(unsigned size=4096);
```

| Parameter | Type | Default | Description |
| --------- | ---- | ------- | ----------- |
| `size` | `unsigned int` | 4096 | The max frame size |

| Return
| ---------------------------
| The latest frame

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `BufferTooSmallException` | `SystemException` | The buffer size is smaller than the next frame |
| `IncorrectModeException` | `SystemException` | Using the synchronous port while in asynchronous mode |

###### Examples
```c++
#include <fscc.hpp>
...

std::string str = p.Read();
```

## Read (Timeout)
```c++
public std::string Read(unsigned size, unsigned timeout);
```

| Parameter | Type | Description |
| --------- | ---- | ------------
| `size` | `unsigned int` | The max frame size |
| `timeout` | `unsigned int` | Number of milliseconds to wait for data |

| Return |
| ------ |
| The latest frame |

| Exception | Base Exception | Cause |
| --------- | -------------- | ----- |
| `BufferTooSmallException` | `SystemException` | The buffer size is smaller than the next frame |
| `IncorrectModeException` | `SystemException` | Using the synchronous port while in asynchronous mode |

###### Examples
```c++
#include <fscc.hpp>
...

std::string str = p.Read(4096, 100);
```


### Additional Resources
- Complete example: [`examples/tutorial.cpp`](../examples/tutorial.cpp)
- Implementation details: [`src/fscc.cpp`](../src/fscc.cpp)