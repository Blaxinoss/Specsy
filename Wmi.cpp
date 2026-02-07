#include "WmiClient.h"


wstring getMemoryTypeString(UINT type) {
    switch (type) {
        case 20: return L"DDR";
        case 21: return L"DDR2";
        case 24: return L"DDR3";
        case 26: return L"DDR4";
        case 30: return L"DDR5";
        case 34: return L"DDR5";
        case 0:  return L"Unknown";
        default: return L"Other";
    }
}

// =========================================================
// Connection
// =========================================================
HRESULT initWmiCOM(IWbemServices** pSvc) {
    HRESULT hr;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return hr;

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) { CoUninitialize(); return hr; }

    IWbemLocator* pLoc = 0;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) { CoUninitialize(); return hr; }

    hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, 0, 0, pSvc);
    if (FAILED(hr)) { pLoc->Release(); CoUninitialize(); return hr; }

    hr = CoSetProxyBlanket(*pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    pLoc->Release();
    return hr; // status of success or fail
}

// =========================================================
// Query on Meory
// =========================================================
void getRamDetails(IWbemServices* pSvc) {
    IEnumWbemClassObject* pEnumerator = NULL;

    // B. تنفيذ جملة الـ SQL (WQL)
    HRESULT hr = pSvc->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT * FROM Win32_PhysicalMemory"), // هات كل الرامات
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (FAILED(hr)) {
        cout << "Query failed." << endl;
        return;
    }

    // loop over query results
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    cout << "\n--- RAM Details ---" << endl;

    while (pEnumerator) {
        // next row
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);


        if (0 == uReturn) break;

        // columns
        VARIANT vtProp;

        // 1. قراءة السعة (Capacity)
        hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);

        wcout << L"Size: " << _wtof(vtProp.bstrVal) / (1024*1024*1024) << " GB" << endl;
        VariantClear(&vtProp); // لازم تنظف الـ VARIANT بعد كل استخدام

        //speed
        hr = pclsObj->Get(L"Speed", 0, &vtProp, 0, 0);

        wcout << L"Speed: " << vtProp.uintVal << " MHz" << endl;
        VariantClear(&vtProp);

        // Manu
        hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
        wcout << L"Make: " << vtProp.bstrVal << endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"SMBIOSMemoryType",0 , &vtProp,0,0);
        wcout << L"Type: " << getMemoryTypeString(vtProp.uintVal) << endl;
        VariantClear(&vtProp);


        pclsObj->Get(L"PartNumber", 0, &vtProp, 0, 0);
        wcout << L"Model: " << vtProp.bstrVal << endl;
        VariantClear(&vtProp);

        pclsObj->Get(L"DeviceLocator", 0, &vtProp, 0, 0);
        wcout << L"Slot: " << vtProp.bstrVal << endl;
        VariantClear(&vtProp);

        pclsObj->Get(L"ConfiguredVoltage", 0, &vtProp, 0, 0);
        if (vtProp.vt == VT_I4) { // نتأكد إنه رقم
            wcout << L"Voltage:    " << vtProp.intVal / 1000.0 << " V" << endl;
        }
        VariantClear(&vtProp);

        cout << "-------------------" << endl;

        pclsObj->Release();
    }

    pEnumerator->Release();
}



// =========================================================
// Clean
// =========================================================
void closeWmiCOM(IWbemServices* pSvc) {
    if (pSvc) pSvc->Release();
    CoUninitialize();
}