#include "pch.h"
#include "wmi_helper.h"

using namespace std::string_literals;

std::map<std::wstring, std::wstring> wmi_query
    (const std::wstring &class_name, const std::list<std::wstring> &properties)
{
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        throw std::runtime_error("CoInitializeEx failed (hr: "s + std::to_string(hr) + ")"s);    
    }

    hr = CoInitializeSecurity(NULL,
                              -1,
                              NULL,
                              NULL,
                              RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              NULL,
                              EOAC_NONE,
                              NULL);

    if (FAILED(hr)) {
        throw std::runtime_error("CoInitializeSecurity failed (hr: "s + std::to_string(hr) + ")"s);
    }

    IWbemLocator *loc = nullptr;

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          reinterpret_cast<LPVOID *>(&loc));

    if (FAILED(hr)) {
        CoUninitialize();

        throw std::runtime_error("CoCreateInstance failed (hr: "s + std::to_string(hr) + ")"s);
    }

    IWbemServices *svc = nullptr;

    hr = loc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
                            NULL,
                            NULL,
                            0,
                            NULL,
                            0,
                            0,
                            &svc);

    if (FAILED(hr)) {
        loc->Release();
        CoUninitialize();

        throw std::runtime_error("IWbemLocator::ConnectServer failed (hr: "s + std::to_string(hr) + ")"s);
    }

    hr = CoSetProxyBlanket(svc,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           EOAC_NONE);

    if (FAILED(hr)) {
        svc->Release();
        loc->Release();
        CoUninitialize();

        throw std::runtime_error("CoSetProxyBlanket failed (hr: "s + std::to_string(hr) + ")"s);
    }

    IEnumWbemClassObject *enumerator = nullptr;

    // Thanks: https://stackoverflow.com/q/46281637/2268041
    std::wostringstream query;
    query << "SELECT ";
    std::copy(properties.begin(), 
              std::prev(properties.end()), 
              std::ostream_iterator<std::wstring, wchar_t, std::char_traits<wchar_t>>(query, L", "));
    query << properties.back();
    query << " FROM " << class_name;

    hr = svc->ExecQuery(_bstr_t("WQL"), 
                        _bstr_t(query.str().c_str()),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL,
                        &enumerator);

    if (FAILED(hr)) {
        svc->Release();
        loc->Release();
        CoUninitialize();

        throw std::runtime_error("IEnumWbemClassObject::ExecQuery failed (hr: "s + std::to_string(hr) + ")"s);
    }

    IWbemClassObject *object = nullptr;
    ULONG no_results = 0;

    hr = enumerator->Next(WBEM_INFINITE,
                          1,
                          &object,
                          &no_results);

    if (FAILED(hr)) {
        enumerator->Release();
        svc->Release();
        loc->Release();
        CoUninitialize();

        throw std::runtime_error("IWbemClassObject::Next failed (hr: "s + std::to_string(hr) + ")"s);
    }

    std::map<std::wstring, std::wstring> results;

    for (auto p_name : properties) {
        VARIANT vt_prop;

        hr = object->Get(p_name.c_str(), 0, &vt_prop, 0, 0);

        if (FAILED(hr)) {
            object->Release();
            enumerator->Release();
            svc->Release();
            loc->Release();
            CoUninitialize();

            throw std::runtime_error("IWbemClassObject::Get failed (hr: "s + std::to_string(hr) + ")"s);
        }

        results.emplace(p_name, std::wstring(vt_prop.bstrVal, SysStringLen(vt_prop.bstrVal)));
    }

    object->Release();
    enumerator->Release();
    svc->Release();
    loc->Release();
    CoUninitialize();

    return results;
}