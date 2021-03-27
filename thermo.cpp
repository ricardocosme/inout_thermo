#include <att85/ssd1306/display.hpp>
#include <att85/ssd1306/font/16x32/chars.hpp>
#include <att85/ssd1306/font/8x8/chars.hpp>
#include <avr/sleep/power_down.hpp>
#include <ds18b20.hpp>

using namespace att85::ssd1306;
using namespace att85::ssd1306::commands;
using namespace ds18b20;
using namespace avr;
using namespace avr::io;

inline auto thermo(auto rom)
{ return sensor{pb3, rom, internal_pullup, resolution::_9bits}; }

int main() {
    //avoids floating inputs
    set(pb1, pb4, pb5);
    
    display_128x64<> disp{contrast, of<1>{}};
    disp.clear();
    disp.on();

    auto inside = thermo(Rom<40, 251, 43, 31, 5, 0, 0, 139>{});
    auto outside = thermo(Rom<40, 198, 8, 141, 5, 0, 0, 124>{});

    disp.out<font::_8x8>(0, 0, ATT85_SSD1306_STR("inside"));
    disp.out<font::_8x8>(3, 40, ATT85_SSD1306_STR("o"));
    
    disp.out<font::_8x8>(0, 64, ATT85_SSD1306_STR("outside"));
    disp.out<font::_8x8>(3, 108, ATT85_SSD1306_STR("o"));
    
    while(true) {
        if(auto in_temp = inside.read())
            if(in_temp.has_value())
                disp.out<font::_16x32>(3, 4, in_temp.value());
        if(auto out_temp = outside.read())
            if(out_temp.has_value())
                disp.out<font::_16x32>(3, 74, out_temp.value());
        sleep::power_down::sleep<10min>();
    }
}
