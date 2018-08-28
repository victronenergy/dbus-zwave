#ifndef DZ_UTIL_H
#define DZ_UTIL_H

#include <set>
#include <string>
#include <tuple>

extern "C" {
#include <velib/types/variant.h>
#include <velib/utils/ve_logger.h>
}

#include <value_classes/ValueID.h>

using OpenZWave::ValueID;
using std::set;
using std::string;

class DZUtil
{
  public:
    struct Variant : public VeVariant {
      Variant(uint8  value) : VeVariant() { veVariantUn8(        this, +value        ); }
      Variant(int16  value) : VeVariant() { veVariantSn16(       this, +value        ); }
      Variant(int32  value) : VeVariant() { veVariantSn32(       this, +value        ); }
      Variant(float  value) : VeVariant() { veVariantFloat(      this,  value        ); }
      Variant(string value) : VeVariant() { veVariantHeapStr(    this,  value.c_str()); }
      Variant(            ) : VeVariant() { veVariantInvalidType(this,  VE_UNKNOWN   ); }
    };

    struct MatchSpec {
      set<string> manufacturerIds;
      set<string> productTypes;
      set<string> productIds;
      set<uint8> commandClassIds;
      set<uint8> instances;
      set<uint8> indexes;
    };

    static bool match(ValueID valueId, MatchSpec spec);

    static string path(uint32 homeId);
    static string path(uint32 homeId, uint8 nodeId);
    static string path(uint32 homeId, uint8 nodeId, uint8 commandClassId);
    static string path(uint32 homeId, uint8 nodeId, uint8 commandClassId, uint8 instance);
    static string path(uint32 homeId, uint8 nodeId, uint8 commandClassId, uint8 instance, uint8 index);
    static string path(ValueID valueId);

    static bool setZwaveValueByVariant(ValueID zwaveValueId, Variant variant) { return DZUtil::setZwaveValueByVariant(zwaveValueId, static_cast<VeVariant>(variant)); }
    static bool setZwaveValueByVariant(ValueID zwaveValueId, VeVariant veVariant);
};

namespace std
{
  template<> struct less<DZUtil::MatchSpec>
  {
    bool operator() (const DZUtil::MatchSpec& lhs, const DZUtil::MatchSpec& rhs) const
    {
      return
        tie(lhs.manufacturerIds, lhs.productTypes, lhs.productIds, lhs.commandClassIds, lhs.instances, lhs.indexes)
        <
        tie(rhs.manufacturerIds, rhs.productTypes, rhs.productIds, rhs.commandClassIds, rhs.instances, rhs.indexes);
    }
  };
};

#endif
