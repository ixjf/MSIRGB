#include "pch.h"
#include "wmi_helper.h"

using namespace std::string_literals;

namespace logic {
    // God damn it, if something fails let it crash - it should NEVER fail
    std::map<std::wstring, std::wstring> wmi_query
        (const std::wstring &class_name, const std::list<std::wstring> &properties)
    {
        // Not needed in C++/CLI
        //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
        /*CoInitializeSecurity(NULL,
                             -1,
                             NULL,
                             NULL,
                             RPC_C_AUTHN_LEVEL_DEFAULT,
                             RPC_C_IMP_LEVEL_IMPERSONATE,
                             NULL,
                             EOAC_NONE,
                             NULL);*/

        IWbemLocator *loc = nullptr;
        CoCreateInstance(CLSID_WbemLocator,
                         0,
                         CLSCTX_INPROC_SERVER,
                         IID_IWbemLocator,
                         reinterpret_cast<LPVOID *>(&loc));

        IWbemServices *svc = nullptr;
        loc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), // Why the fuck is this call so slow in Debug mode w/ debugger attached?
                           NULL,
                           NULL,
                           0,
                           NULL,
                           0,
                           0,
                           &svc);

        CoSetProxyBlanket(svc,
                          RPC_C_AUTHN_WINNT,
                          RPC_C_AUTHZ_NONE,
                          NULL,
                          RPC_C_AUTHN_LEVEL_CALL,
                          RPC_C_IMP_LEVEL_IMPERSONATE,
                          NULL,
                          EOAC_NONE);

        IEnumWbemClassObject *enumerator = nullptr;

        // Thanks: https://stackoverflow.com/q/46281637/2268041
        std::wostringstream query;

        query << "SELECT ";

        std::copy(properties.begin(), 
                  std::prev(properties.end()), 
                  std::ostream_iterator<std::wstring, wchar_t, std::char_traits<wchar_t>>(query, L", "));

        query << properties.back();

        query << " FROM " << class_name;

        svc->ExecQuery(_bstr_t("WQL"), 
                       _bstr_t(query.str().c_str()),
                       WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                       NULL,
                       &enumerator);

        IWbemClassObject *object = nullptr;
        ULONG no_results = 0;

        enumerator->Next(WBEM_INFINITE,
                              1,
                              &object,
                              &no_results);

        std::map<std::wstring, std::wstring> results;

        for (auto p_name : properties) {
            VARIANT vt_prop;
            object->Get(p_name.c_str(), 0, &vt_prop, 0, 0);

            results.emplace(p_name, std::wstring(vt_prop.bstrVal, SysStringLen(vt_prop.bstrVal)));
        }

        object->Release();
        enumerator->Release();
        svc->Release();
        loc->Release();
        //CoUninitialize();

        return results;
    }
}