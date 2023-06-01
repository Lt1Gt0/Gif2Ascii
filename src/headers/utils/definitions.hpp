#pragma once
#ifndef _GIF_DEFINITIONS_HPP_
#define _GIF_DEFINITIONS_HPP_

#define PACKED __attribute__((packed))
#define UNUSED __attribute__((unused))

// TODO: If I ever add support for 87a (which I plan to), add it to supported versions
constexpr const char* SUPPORTED_VERSIONS[1] {"89a"};

#endif // _GIF_DEFINITIONS_HPP_
