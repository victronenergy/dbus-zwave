# vic-dbus-zwave
**C++ bridge for publishing Z-Wave devices on DBus**

The bridge uses the [OpenZWave library](https://github.com/OpenZWave/open-zwave)
to handle Z-Wave connectivity and [velib](https://github.com/victronenergy/velib)
for the DBus interface. It expects OpenZWave to be installed as a system
library (so that it might be updated separately to support new Z-Wave devices).

## Building and running
 - Install OpenZWave: `cd ext/open-zwave && make && make install`.
 - Build with either `./configure && make` or `qmake && make`.
 - Run with `LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pkg-config --libs-only-L libopenzwave | sed 's/^-L//') ./dbus-zwave`

For now, the serial port used for connecting is hardcoded in `task.cpp`, change
`defaultDriver = "/dev/ttyS8"` to whatever you need and recompile.

## Code overview
 - `src/task.cpp`: The velib task. Connects OpenZWave and DBus and creates
   objects for each new Z-Wave item.
 - `src/dz_driver.cpp`: Driver item. Upon receiving a value change over DBus,
   will set the Z-Wave controller in inclusion mode to add a new device.
 - `src/dz_node.cpp`: Publishes info about a node. Doesn't do much else currently.
 - `src/dz_value.cpp`: Publishes a Z-Wave node value on the DBus. This can be
   a measurement, setting or something else.

All items are currently published under a path determined by their ID's:
```sh
/Zwave/$HOMEID/$NODEID/$COMMANDCLASS/$INSTANCE/$INDEX
```
