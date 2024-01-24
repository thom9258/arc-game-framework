#pragma once

#include <memory>

namespace arc {
namespace core {

// Very useful function from Eric Scott Barr:
// https://eb2.co/blog/2014/04/c-plus-plus-14-and-sdl2-managing-resources/
template <typename Creator, typename Destructor, typename... Args>
auto unique_resource(Creator c, Destructor d, Args&&... args) {
    auto r = c(std::forward<Args>(args)...);
    return std::unique_ptr<std::decay_t<decltype(*r)>, decltype(d)>(r, d);
}

template <typename Creator, typename Destructor, typename... Args>
auto shared_resource(Creator c, Destructor d, Args&&... args) {
    // auto r = c(std::forward<Args>(args)...);
    // auto rptr = std::unique_ptr<std::decay_t<decltype(*r)>, decltype(d)>(r,
    // d); return std::make_shared(std::move(rptr));
    return std::make_shared(unique_resource(c, d, (args)...));
}

} /*ns*/
} /*ns*/
