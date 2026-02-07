//
// Created by Abdul on 2/8/2026.
//

#ifndef CODEFORCES_WMICLIENT_H
#define CODEFORCES_WMICLIENT_H

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <string>


using namespace std;

HRESULT initWmiCOM(IWbemServices** pSvc);

void getRamDetails(IWbemServices* pSvc);

void closeWmiCOM(IWbemServices* pSvc);

#endif //CODEFORCES_WMICLIENT_H