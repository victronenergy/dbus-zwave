#ifndef DZ_GRIDMETER_H
#define DZ_GRIDMETER_H

#include <map>
#include <string>
#include <tuple>

#include <value_classes/ValueID.h>

#include "dz_value.hpp"

using OpenZWave::ValueID;
using std::map;
using std::string;
using std::tuple;

class DZGridMeter : public DZValue
{
  public:
    static bool handles(ValueID zwaveValueId);

    explicit DZGridMeter(ValueID zwaveValueId);

    virtual void publish() override;
    virtual string getServiceName() override;
    virtual string getPath() override;

    virtual void onZwaveNotification(const Notification* _notification) override;

  protected:
    static map<tuple<uint8, uint8, uint8>, string> valueMapping;
    static tuple<uint8, uint8, uint8> zwaveValueIdToTuple(ValueID zwaveValueId);

    string path;

    template<typename T> void addConstAux(string path, T value);
    void updateTotals();
};

#endif
