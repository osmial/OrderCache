Project to build requires cmake and conan on host OS.
Build steps:
1. 'mkdir build && cd build'
2. 'cmake .. -DCMAKE_BUILD_TYPE=Release' or 'cmake .. -DCMAKE_BUILD_TYPE=Debug'

Unit test binary: OrderCache/build/bin/tests

Disclaimer:
Headers ordercacheinterface.hpp and order.hpp were provided with the task description and the note to do not alter defined interfaces, only change I did to the interfaces was to add some operators in Order's public interface. Besides that I have left some comments how I would improve/change the interface due to various reasons.

The task's requierement was not to get best performance yet I've tried to make it as optimal as I was able in terms of required public API and reasonable time spent on the implementation.

It was also requested to be delivered as a zip archive so I didn't use git from very beginning, that's why there is no
commit history. It's only for archive purposes here.

