#pragma once

#include "pch.h"

namespace logic {
    std::map<std::wstring, std::wstring> wmi_query
        (const std::wstring &class_name, const std::list<std::wstring> &properties);
}