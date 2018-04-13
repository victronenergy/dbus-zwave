# vic-dbus-zwave
**C++ bridge for publishing Z-Wave devices on DBus**

The bridge uses the [OpenZWave library](https://github.com/OpenZWave/open-zwave)
to handle Z-Wave connectivity and [velib](https://github.com/victronenergy/velib)
for the D-Bus interface. It expects OpenZWave to be installed as a system
library (so that it might be updated separately to support new Z-Wave devices).

## Building and running
 - Install OpenZWave: `cd ext/open-zwave && make && make install`.
 - Build with either `./configure && make` or `qmake && make`.
 - Run with `LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pkg-config --libs-only-L libopenzwave | sed 's/^-L//') ./dbus-zwave`

Alternatively, the provided Dockerfile is set up to cross-compile for the CCGX.

## Code overview
 - `src/task.cpp`: The velib task. Connects OpenZWave and creates objects for
   each new Z-Wave item.
 - `src/dz_item.cpp`: Abstract base for all items, contains logic for setting
   up the D-Bus connection.
 - `src/dz_driver.cpp`: Driver item. Upon receiving a value change over D-Bus,
   will set the Z-Wave controller in inclusion mode to add a new device.
 - `src/dz_node.cpp`: Publishes info about a node. Doesn't do much else currently.
 - `src/dz_value.cpp`: Publishes a Z-Wave node value on the D-Bus. This can be
   a measurement, setting or something else. They are published under a path
   with the form:
   `/Interfaces/$HOMEID/Devices/$NODEID/CommandClasses/$COMMANDCLASS/Instances/$INSTANCE/Indexes/$INDEX`.
 - `src/dz_namedvalue.cpp`: Publishes supported values to a predefined path.
   Used for publishing known Z-Wave values to common paths used by the CCGX.
 - `src/dz_constvalue.cpp`: Publishes a constant value to the D-Bus, not linked
   to any Z-Wave value.
 - `src/dz_setting.cpp`: Publishes a setting to the CCGX settings service on
   D-Bus.

For now, the serial port used for connecting is hardcoded in `task.cpp`, change
`defaultDriver = "/dev/ttyACM0"` to whatever you need and recompile.
