#ifndef MAGNACHAIN_MONITOR_INIT_H
#define MAGNACHAIN_MONITOR_INIT_H

#include <string>

class MCScheduler;

namespace boost
{
    class thread_group;
} // namespace boost

bool MonitorInitMain(boost::thread_group& threadGroup, MCScheduler& scheduler);

#endif