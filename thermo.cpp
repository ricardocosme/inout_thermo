#include <avr/sleep.hpp>
#include <ssd1306.hpp>
#define F_CPU 1.2e6
#include <ds18b20.hpp>

using namespace ds18b20;
using namespace avr::io;
using namespace avr::sleep;
using namespace ssd1306;

AVRINT_WDT(){ inc_wdt_cnt(); }

inline auto thermo(auto rom)
{ return sensor{pb3, rom, internal_pullup, resolution::_9bits}; }

const uint8_t degree[] = {0xff, 0x81, 0x81, 0x81, 0x81, 0xff};

int main() {
    display disp{pb0, pb2, contrast<1>{}, turn_on{}};
    
    //avoids floating inputs
    set(pb1, pb4, pb5);
    
    auto inside = thermo(Rom<40, 251, 43, 31, 5, 0, 0, 139>{});
    auto outside = thermo(Rom<40, 198, 8, 141, 5, 0, 0, 124>{});

    //line between the labels and the temperature values
    disp.out(page{2,2}, 0x50, repeat{uint8_t(128)});

    disp.out<12, 16>(page{0,1}, column{3}, segments::i, segments::n);
    disp.out<12, 16>(column{75}, segments::o, segments::u, segments::t);

    disp.out(page{4,4}, column{46}, degree);
    disp.out(column{116}, degree);
    
    while(true) {
        if(auto in_temp = inside.read()) 
            if(in_temp.has_value())
                disp.out<18, 32>(page{4,7}, column{0}, in_temp.value());
        if(auto out_temp = outside.read()) 
            if(out_temp.has_value())
                disp.out<18, 32>(column{70}, out_temp.value());
        power_down::sleep_for<10_min>();
    }
}
