#include "ssd1306.hpp"
#include <avr/pgmspace.h>
#include <avr/sleep.hpp>
#define F_CPU 1.2e6
#include <ds18b20.hpp>

using namespace ds18b20;
using namespace avr::io;
using namespace avr::sleep;
using namespace ssd1306;

AVRINT_WDT(){ inc_wdt_cnt(); }

inline auto thermo(auto rom)
{ return sensor{pb3, rom, internal_pullup, resolution::_9bits}; }

static const uint8_t degree[] = {0xff, 0x81, 0x81, 0x81, 0x81, 0xff};

int main() {
    ssd1306::i2c dev{pb0, pb2};

    send_commands(dev, cmds);

    /** clear the whole screen */
    out(dev, 0x00, repeat{128 * 8});
    
    //avoids floating inputs
    set(pb1, pb4, pb5);
    
    auto inside = thermo(Rom<40, 251, 43, 31, 5, 0, 0, 139>{});
    auto outside = thermo(Rom<40, 198, 8, 141, 5, 0, 0, 124>{});

    //line between the labels and the temperature values
    out(dev, page{2,2}, 0x50, repeat{uint8_t(128)});

    out<12, 16>(dev, page{0,1}, column{3}, seg_i, seg_n);
    out<12, 16>(dev, column{75}, seg_o, seg_u, seg_t);

    out(dev, page{4,4}, column{46}, degree);
    out(dev, column{116}, degree);
    
    while(true) {
        if(auto in_temp = inside.read()) 
            if(in_temp.has_value())
                out<18, 32>(dev, page{4,7}, column{0}, in_temp.value());
        if(auto out_temp = outside.read()) 
            if(out_temp.has_value())
                out<18, 32>(dev, column{70}, out_temp.value());
        power_down::sleep_for<500_ms>();
    }
}
