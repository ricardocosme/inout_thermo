#include <att85/ssd1306/display.hpp>
#include <att85/ssd1306/font/16x32/chars.hpp>
#include <att85/ssd1306/font/8x8/chars.hpp>
#include <avrcxx/sleep/power_down.hpp>
#include <ds18b20.hpp>

using namespace att85::ssd1306;
using namespace att85::ssd1306::commands;
using namespace ds18b20;
using namespace avrcxx;

inline void disable_unused_pins()
{ PORTB = PORTB | (1 << PB1) | (1 << PB4) | (1 << PB5); }

template<typename Rom_>
using thermo = sensor<PB3, Rom_, InternalPullup>;

int main() {
    display_128x64<> disp{contrast, of<1>{}};
    disp.clear();
    disp.on();
    
    thermo<Rom<40, 251, 43, 31, 5, 0, 0, 139>> inside;
    thermo<Rom<40, 198, 8, 141, 5, 0, 0, 124>> outside;
    
    using temperature = decltype(inside)::value_type;
    
    disp.out<font::_8x8>(0, 0, ATT85_SSD1306_STR("inside"));
    disp.out<font::_8x8>(3, 40, ATT85_SSD1306_STR("o"));
    
    disp.out<font::_8x8>(0, 64, ATT85_SSD1306_STR("outside"));
    disp.out<font::_8x8>(3, 108, ATT85_SSD1306_STR("o"));

    disable_unused_pins();

    temperature in_temp;
    temperature out_temp;
    while(true) {
        if(in_temp = inside.read(); in_temp)
            if(in_temp.has_value())
                disp.out<font::_16x32>(3, 4, in_temp.value());
        if(out_temp = outside.read(), out_temp)
            if(out_temp.has_value())
                disp.out<font::_16x32>(3, 74, out_temp.value());
        if(in_temp && out_temp)
            sleep::power_down::sleep<10min>();
    }
}
