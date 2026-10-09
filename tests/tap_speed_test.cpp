#include "../src/TapSpeed.hpp"
#include <cassert>
#include <iostream>
int main() {
    TapSpeed speed;
    for (int i = 0; i < 12; i++) assert(!speed.press(i*.20)); // 5 CPS
    speed.reset();
    for (int i = 0; i < 9; i++) assert(speed.press(i*.10) == (i >= 3)); // 10 CPS
    speed.reset();
    for (int i = 0; i < 10; i++) assert(!speed.press(i*.125)); // exactly 8 CPS
    speed.reset();
    for (int i = 0; i < 7; i++) speed.press(i*.075);
    assert(speed.fast());
    assert(!speed.press(6*.075 + .55)); // pause resets
    speed.reset();
    assert(!speed.fast());
    std::cout << "TapSpeed tests passed: slow/fast/equal threshold/pause/reset\n";
}
