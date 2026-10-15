#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace axbox {

struct Asset {
    const unsigned char* data;
    size_t size;
};

// 构建时由 cmake/embed_web.cmake 把 web/ 下的文件编进程序
std::optional<Asset> find_asset(const std::string& path);

}  // namespace axbox
