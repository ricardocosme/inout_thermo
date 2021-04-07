#pragma once

#include <avr/pgmspace.h>
#include <ssd1306.hpp>

/** This demo setups a display with 128x64 dots with some basic
    commands and after that erases the content of the whole screen to
    print a square of 8x8 pixels.
*/

static const uint8_t cmds[]= {
    /** Commands to inform the driver what is the physical
        configuration of the display. Note that your display can be
        different, take a look at the secton 10.1.18 of the datasheet
        with the result on the screen is weird. 
    */
    0xC8, /** COM Output Scan Direction*/ 
    0xA1, /** Segment Re-map */
    
    0x20, 1, /** Vertical Addressing Mode*/ 
    0x22, 0, 7, /** Set page address: 0 to 7*/  
    0x21, 0, 127, /** Set column address: 0 to 127*/
    
    0x8D, 0x14, /** Enable Charge Pump*/
    0x81, 0x01, /** Contrast */
    0xAF, /** Turn on the display */
};

template<typename I2C, typename T, int N>
void send_commands(I2C&& dev, const T(&cmds)[N]) {
    dev.start_commands();
    for(uint8_t i{0}; i < N; ++i)
        dev.send_byte(cmds[i]);
    dev.stop_condition();
}
struct page {
    uint8_t start{0}, end{7};
};

struct column {
    uint8_t start{0}, end{127};
};

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, page pg, column col) {
    i2c.start_commands();
    i2c.send_byte(0x22);
    i2c.send_byte(pg.start);
    i2c.send_byte(pg.end);
    i2c.send_byte(0x21);
    i2c.send_byte(col.start);
    i2c.send_byte(col.end);
    i2c.stop_condition();
}

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, column col) {
    i2c.start_commands();
    i2c.send_byte(0x21);
    i2c.send_byte(col.start);
    i2c.send_byte(col.end);
    i2c.stop_condition();
}

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, page pg) {
    i2c.start_commands();
    i2c.send_byte(0x22);
    i2c.send_byte(pg.start);
    i2c.send_byte(pg.end);
    i2c.stop_condition();
}

struct seven_segment { uint8_t segments; };

constexpr static auto top = 1<<0;
constexpr static auto left_top = 1<<1;
constexpr static auto right_top = 1<<2;
constexpr static auto middle = 1<<3;
constexpr static auto left_bottom = 1<<4;
constexpr static auto right_bottom = 1<<5;
constexpr static auto bottom = 1<<6;

constexpr static auto seg_i = seven_segment{right_bottom};
constexpr static auto seg_n = seven_segment{left_bottom | middle | right_bottom};
constexpr static auto seg_p = seven_segment{
    left_bottom | left_top | top | middle | right_top};
constexpr static auto seg_u = seven_segment{left_bottom | bottom | right_bottom};
constexpr static auto seg_t = seven_segment{
    left_bottom | left_top | middle | bottom};
constexpr static auto seg_o = seven_segment{
    middle | left_bottom | right_bottom | bottom};

template<int pages, typename I2C>
inline void draw_column(I2C& i2c, uint8_t b) {
    for(uint8_t i{}; i < pages / 2; ++i)
        i2c.send_byte(b);
}

template<uint8_t width, uint8_t height, uint8_t spacing = 3, typename I2C>
void send_digit_segmented(I2C& i2c, uint8_t segments)
{
    static_assert(width >= 12 && width <= 64);
    static_assert(height % 16 == 0);
    static_assert(height >= 16 && height <= 64);
    constexpr auto pages = height / 8;
    for(uint8_t col{0}; col < width; ++col) {
        if(col < 2) {
            if(segments & left_top) draw_column<pages>(i2c, 0xff);
            else draw_column<pages>(i2c, 0x00);
            if(segments & left_bottom) draw_column<pages>(i2c, 0xff);
            else draw_column<pages>(i2c, 0x00);
        } else if(col >= 2 && col < (width - 2)) {
            if(pages == 2) {
                if(segments & top) {
                    if(segments & middle) i2c.send_byte(0x83);
                    else i2c.send_byte(0x03);
                } else {
                    if(segments & middle) i2c.send_byte(0x80);
                    else i2c.send_byte(0x00);
                }
                if(segments & bottom) {
                    if(segments & middle) i2c.send_byte(0xc1);
                    else i2c.send_byte(0xc0);
                } else {
                    if(segments & middle) i2c.send_byte(0x01);
                    else i2c.send_byte(0x00);
                }
            } else {
                if(segments & top) i2c.send_byte(0x03);
                else i2c.send_byte(0x00);
                if(segments & middle) {
                    draw_column<pages - 4>(i2c, 0x00);
                    i2c.send_byte(0x80);
                    i2c.send_byte(0x01);
                } else draw_column<pages>(i2c, 0x00);
                draw_column<pages - 4>(i2c, 0x00);
                if(segments & bottom) i2c.send_byte(0xc0);
                else i2c.send_byte(0x00);
            }
        } else if(col >= (width - 2) && col < width) {
            if(segments & right_top) draw_column<pages>(i2c, 0xff);
            else draw_column<pages>(i2c, 0x00);
            if(segments & right_bottom) draw_column<pages>(i2c, 0xff);
            else draw_column<pages>(i2c, 0x00);
        }
    }
    for(uint8_t i{}; i < spacing * pages; ++i)
        i2c.send_byte(0x00);
}

template<uint8_t width, uint8_t height, uint8_t spacing, typename I2C> 
inline void send_digit(I2C& dev, uint8_t i) {
    uint8_t segments;
    if(i == 0)
        segments = top | left_top | left_bottom | right_top | right_bottom
            | bottom;
    else if(i == 1)
        segments = right_top| right_bottom;
    else if(i == 2)
        segments = top | right_top | middle | left_bottom | bottom;
    else if(i == 3)
        segments = top | right_top | middle | right_bottom | bottom;
    else if(i == 4)
        segments = left_top | middle | right_top | right_bottom;
    else if(i == 5)
        segments = top | left_top | middle | right_bottom | bottom;
    else if(i == 6)
        segments = top | left_top | middle | left_bottom | bottom | right_bottom;
    else if(i == 7)
        segments = top | right_top | right_bottom;
    else if(i == 8)
        segments = left_top | top | right_top | middle | left_bottom
            | right_bottom | bottom;
    else if(i == 9)
        segments = left_top | top | right_top | middle | right_bottom | bottom;
    send_digit_segmented<width, height, spacing>(dev, segments);
}

template<uint8_t w, uint8_t h, uint8_t spacing = 5, typename I2C> 
inline void send_int(I2C& dev, uint8_t i) {
    if(i < 10) {
        send_digit<w, h, spacing>(dev, i);
        return;
    }
    send_int<w, h, spacing>(dev, i / 10);
    send_int<w, h, spacing>(dev, i % 10);
}

template<typename I2C>
void send_degree(I2C& dev) {
    dev.start_data();
    dev.send_byte(0xff);
    dev.send_byte(0x81);
    dev.send_byte(0x81);
    dev.send_byte(0x81);
    dev.send_byte(0x81);
    dev.send_byte(0xff);
    dev.stop_condition();
}

template<uint8_t w, uint8_t h, typename I2C>
void out_impl(I2C&& i2c, seven_segment seg)
{ send_digit_segmented<w,h>(i2c, seg.segments); }

template<uint8_t w, uint8_t h, typename I2C>
void out_impl(I2C&& i2c, uint8_t v)
{ send_int<w, h>(i2c, v); }

template<uint8_t w, uint8_t h, typename I2C, int N>
void out_impl(I2C&& i2c, const uint8_t (&bytes)[N]) {
    for(uint8_t i{}; i < N; ++i)
        i2c.send_byte(bytes[i]);
}

template<uint8_t w = 0, uint8_t h = 0, typename I2C, typename... Segs>
void out(I2C&& i2c, column col, Segs... segs) {
    set(i2c, col);
    i2c.start_data();
    (out_impl<w, h>(i2c, segs), ...);
    i2c.stop_condition();
}

template<uint8_t w = 0, uint8_t h = 0, typename I2C, typename... Segs>
void out(I2C&& i2c, page pg, column col, Segs... segs) {
    set(i2c, pg, col);
    i2c.start_data();
    (out_impl<w, h>(i2c, segs), ...);
    i2c.stop_condition();
}

template<uint8_t w = 0, uint8_t h = 0, typename I2C, int N>
void out(I2C&& i2c, const uint8_t (&bytes)[N]) {
    i2c.start_data();
    out_impl<w, h>(i2c, bytes);
    i2c.stop_condition();
}

template<typename I2C, int N>
void out(I2C&& i2c, column col, const uint8_t (&bytes)[N]) {
    set(i2c, col);
    out(i2c, bytes);
}

template<typename I2C, int N>
void out(I2C&& i2c, page pg, column col, const uint8_t (&bytes)[N]) {
    set(i2c, pg, col);
    out(i2c, bytes);
}

template<typename T>
struct repeat{
    T value;
};

template<typename I2C, typename T>
void out(I2C&& i2c, uint8_t byte, const repeat<T>& rep) {
    i2c.start_data();
    for(T i{}; i < rep.value; ++i)
        i2c.send_byte(byte);
    i2c.stop_condition();
}

template<typename I2C, typename T>
void out(I2C&& i2c, page pg, uint8_t byte, const repeat<T>& rep) {
    set(i2c, pg);
    out(i2c, byte, rep);
}
