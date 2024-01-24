#pragma once

// TODO: Include these where needed!
//   #include <iostream>
//   #include <sstream>
//   #include <fstream>
//
//   #include <ctime>
//   #include <cassert>
//   #include <memory>
//
//   #include <ranges>
//   #include <algorithm>
//   #include <functional>
//
//   #include <string>
//   #include <list>
//   #include <vector>
//   #include <unordered_map>
//   #include <deque>
//   #include <thread>

#ifndef ARC_ASSERT
#  define ARC_ASSERT(X) assert(X)
#endif

#ifndef ARC_UNUSED
#  define ARC_UNUSED(X) (void)(X)
#endif

#if __STDC_VERSION__ < 199901L
#  if __GNUC__ >= 2
#    define __func__ __FUNCTION__
#  else
#    define __func__ "<invalid __func__ define>"
#  endif
#endif

#ifndef ARC_NOT_IMPLEMENTED
#  define ARC_NOT_IMPLEMENTED() ARC_ASSERT(1 && "Function " __func__ " is not implemented yet!")
#endif
