#define F_CPU 1.0e6
#include <util/delay.h>
#include <avr/sleep.hpp>
#include <power_switch.hpp>
#include <ssd1306.hpp>
#include <ds18b20.hpp>

using namespace ds18b20;
using namespace avr::io;
using namespace avr::sleep;
using namespace ssd1306;

static auto& load{pb4};
static auto& sw{pb1};

AVRINT_PCINT0(){ power::turnon(sw, load); }

AVRINT_WDT(){ inc_wdt_cnt(); }

inline auto thermo(auto rom)
{ return sensor{pb3, rom, internal_pullup, resolution::_9bits}; }

const uint8_t degree[] = {0xff, 0x81, 0x81, 0x81, 0x81, 0xff};

int main() {
    //avoids floating inputs
    set(pb5);
    
    /** A naive implementation to debounces a pressed push button that
        uses a pull-up resistor. */
    auto btn_pressed = [](auto pin){
        if(pin.is_low()) {
            _delay_ms(40);
            return pin.is_low();
        } 
        return false;
    };
    
    /** A naive implementation to debounces a released push button. */
    auto btn_released = [](auto pin){
        if(pin.is_high()) {
            _delay_ms(40);
            return pin.is_high();
        } 
        return false;
    };
    
    /** The system is halted at the startup and it turns on when the
        switch is pressed. */
    power::sw power(sw, load, btn_pressed, btn_released);

    auto disp = display{pb0, pb2, contrast<1>{}, turn_on{}};

    //line between the labels and the temperature values
    disp.out(page{2,2}, 0x50, repeat{uint8_t(128)});

    disp.out<12, 16>(page{0,1}, column{3}, segments::i, segments::n);
    disp.out<12, 16>(column{75}, segments::o, segments::u, segments::t);

    disp.out(page{4,4}, column{46}, degree);
    disp.out(column{116}, degree);
    
    auto inside = thermo(Rom<40, 251, 43, 31, 5, 0, 0, 139>{});
    auto outside = thermo(Rom<40, 198, 8, 141, 5, 0, 0, 124>{});
    
    auto turnoff = []{
        out(pb0, pb2, pb3);
        clear(pb0, pb2, pb3);
    };

    bool in_temp_ready{false}, out_temp_ready{false};
    while(true) {
        power.poll(turnoff);

        if(auto in_temp = inside.async_read()) 
            if(in_temp.has_value()) {
                in_temp_ready = true;
                disp.out<18, 32>(page{4,7}, column{0}, in_temp.value());
            }
        if(auto out_temp = outside.async_read()) 
            if(out_temp.has_value()) {
                out_temp_ready = true;
                disp.out<18, 32>(page{4,7}, column{70}, out_temp.value());
            }

        if(in_temp_ready && out_temp_ready) {
            in_temp_ready = out_temp_ready = false;
            power_down::sleep_for<8_s>();
            power.off(turnoff);
        }
    }
}
