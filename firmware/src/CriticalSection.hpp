
#ifndef CRITICAL_SECTION_HPP
#define CRITICAL_SECTION_HPP


namespace octane
{


/// Disable interrupts within this object's scope.
class CriticalSectionLock
{
public:
    CriticalSectionLock();
    ~CriticalSectionLock();
    CriticalSectionLock& operator=(const CriticalSectionLock&) = delete;

};


}


#endif
