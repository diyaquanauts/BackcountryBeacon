//
// Created by brett on 11/5/2020.
//

#ifndef BITFLIP_FUNCTIONTIMER_H
#define BITFLIP_FUNCTIONTIMER_H

class FunctionTimer{
public:
    void (* func_ptr)();
    int update_time;
    int last;
    FunctionTimer(void (* _func_ptr)(), int _update_time){
        func_ptr = _func_ptr;
        update_time = _update_time;
        last = 0;
    }

    void service() {
        if ((millis() - last) > update_time) {
            func_ptr();
            last = millis();
        }
    }
};

#endif //BITFLIP_FUNCTIONTIMER_H