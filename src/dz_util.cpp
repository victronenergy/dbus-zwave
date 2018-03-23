#include <stdio.h>
#include <stdlib.h>

#include <Defs.h>
#include <value_classes/ValueID.h>

using OpenZWave::ValueID;
using std::ostringstream;
using std::string;

string dz_path(uint32 zwaveHomeId)
{
    ostringstream path;
    path << "Zwave/" << +zwaveHomeId;
    return path.str();
}

string dz_path(uint32 zwaveHomeId, uint8 zwaveNodeId)
{
    ostringstream path;
    path << dz_path(zwaveHomeId) << "/" << +zwaveNodeId;
    return path.str();
}

string dz_path(uint32 zwaveHomeId, uint8 zwaveNodeId, ValueID zwaveValueId)
{
    ostringstream path;
    path << dz_path(zwaveHomeId, zwaveNodeId) << "/" << +zwaveValueId.GetCommandClassId() << "/" << +zwaveValueId.GetInstance() << "/" << +zwaveValueId.GetIndex();
    return path.str();
}
