//
// Created by swear on 2025/08/22.
//

#ifndef MYOS_X86_DESCRIPTOR_HPP
#define MYOS_X86_DESCRIPTOR_HPP


enum class DescriptorType {
    // system segment & gate descriptor types
    Upper8Bytes = 0,
    LDT = 2,
    TSSAvailable = 9,
    TSSBusy = 11,
    CallGate = 12,
    InterruptGate = 14,
    TrapGate = 15,

    // code & data segment types
    ReadWrite = 2,
    ExecuteRead = 10,
};


#endif //MYOS_X86_DESCRIPTOR_HPP
