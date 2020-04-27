#pragma once

#include <string>
#include <vector>

namespace WLANAPI {

#include <Windows.h>
#include <Wlanapi.h>

struct windows_scan_info {
  std::string bssid;
  std::string ssid;
  unsigned long long last_seen;
  int freq;
  long signal;
};

class Handle {
private:
  HANDLE mHandle = nullptr;

public:
  Handle();
  ~Handle();

  Handle(const Handle &that) = delete;
  Handle &operator=(const Handle &that) = delete;

  operator HANDLE() const { return mHandle; }
};

class Interfaces {
private:
  PWLAN_INTERFACE_INFO_LIST interfaceList = nullptr;

public:
  Interfaces(const Handle &handle);
  ~Interfaces();

  Interfaces(const Interfaces &that) = delete;
  Interfaces &operator=(const Interfaces &that) = delete;

  GUID guidByIndex(int index);
  std::wstring descriptionByIndex(int index);
  unsigned long size();
};

class BssList {
private:
  PWLAN_BSS_LIST wlanBssList = nullptr;

public:
  BssList(const Handle &handle, int interfaceIndex);
  ~BssList();

  BssList(const BssList &that) = delete;
  BssList &operator=(const BssList &that) = delete;

  std::vector<windows_scan_info> getScan();
};

unsigned long triggerScan(int interfaceIndex);
} // namespace WLANAPI
