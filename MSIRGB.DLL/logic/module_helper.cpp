#include "pch.h"
#include "module_helper.h"

HMODULE get_current_hmodule()
{
    HMODULE mod;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      reinterpret_cast<LPCWSTR>(&get_current_hmodule),
                      &mod);

    return mod;
}