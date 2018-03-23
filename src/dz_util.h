#include <Defs.h>
#include <value_classes/ValueID.h>

using OpenZWave::ValueID;
using std::string;

string dz_path(uint32 homeId);
string dz_path(uint32 homeId, uint8 nodeId);
string dz_path(uint32 homeId, uint8 nodeId, ValueID valueId);
