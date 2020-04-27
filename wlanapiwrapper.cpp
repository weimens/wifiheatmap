#include "wlanapiwrapper.h"
#include "helper.h"

#include <system_error>

#include <Synchapi.h>
#include <Wlantypes.h>

#pragma comment(lib, "wlanapi.lib") // FIXME: cmake

namespace WLANAPI {

struct WLAN_CALLBACK_INFO {
  GUID InterfaceGuid;
  HANDLE event;
  DWORD notificationCode;
};

void scanCallback(WLAN_NOTIFICATION_DATA *wlanNotificationData, PVOID Arg2) {
  WLAN_CALLBACK_INFO *callbackInfo = (WLAN_CALLBACK_INFO *)Arg2;
  if (callbackInfo == nullptr)
    return;
  if (memcmp(&callbackInfo->InterfaceGuid, &wlanNotificationData->InterfaceGuid,
             sizeof(GUID)) != 0)
    return;

  if ((wlanNotificationData->NotificationCode ==
       wlan_notification_acm_scan_complete) ||
      (wlanNotificationData->NotificationCode ==
       wlan_notification_acm_scan_fail)) {
    callbackInfo->notificationCode = wlanNotificationData->NotificationCode;
    SetEvent(callbackInfo->event);
  }

  return;
}

Handle::Handle() {
  HRESULT result = 0;
  DWORD negotiatedVersion = 0;

  result =
      WlanOpenHandle(WLAN_API_VERSION_2_0, NULL, &negotiatedVersion, &mHandle);

  if (result != ERROR_SUCCESS) {
    std::error_code ec(result, std::system_category());
    throw std::system_error(ec);
  }
}

Handle::~Handle() { WlanCloseHandle(mHandle, NULL); }

Interfaces::Interfaces(const Handle &handle) {
  HRESULT result = 0;

  result = WlanEnumInterfaces(handle, NULL, &interfaceList);

  if (result != ERROR_SUCCESS) {
    std::error_code ec(result, std::system_category());
    throw std::system_error(ec);
  }
}

Interfaces::~Interfaces() { WlanFreeMemory(interfaceList); }

GUID Interfaces::guidByIndex(int index) {
  return interfaceList->InterfaceInfo[index].InterfaceGuid;
}

std::wstring Interfaces::descriptionByIndex(int index) {
  return std::wstring{
      interfaceList->InterfaceInfo[index].strInterfaceDescription};
}

unsigned long Interfaces::size() { return interfaceList->dwNumberOfItems; }

BssList::BssList(const Handle &handle, int interfaceIndex) {
  HRESULT result = 0;
  Interfaces interfaces(handle);
  auto guid = interfaces.guidByIndex(interfaceIndex);

  result = WlanGetNetworkBssList(handle, &guid, NULL, dot11_BSS_type_any, false,
                                 NULL, &wlanBssList);

  if (result != ERROR_SUCCESS) {
    std::error_code ec(result, std::system_category());
    throw std::system_error(ec);
  }
}

BssList::~BssList() { WlanFreeMemory(wlanBssList); }

std::vector<windows_scan_info> BssList::getScan() {
  std::vector<windows_scan_info> scans(wlanBssList->dwNumberOfItems);
  for (ULONG num = 0; num < wlanBssList->dwNumberOfItems; num++) {
    windows_scan_info scanInfo;

    scanInfo.bssid = mac_addr(wlanBssList->wlanBssEntries[num].dot11Bssid);
    scanInfo.ssid =
        std::string(wlanBssList->wlanBssEntries[num].dot11Ssid.ucSSID,
                    wlanBssList->wlanBssEntries[num].dot11Ssid.ucSSID +
                        wlanBssList->wlanBssEntries[num].dot11Ssid.uSSIDLength);
    scanInfo.last_seen = wlanBssList->wlanBssEntries[num].ullTimestamp;
    scanInfo.freq = wlanBssList->wlanBssEntries[num].ulChCenterFrequency;
    scanInfo.signal = wlanBssList->wlanBssEntries[num].lRssi;

    scans[num] = scanInfo;
  }
  return scans;
}

unsigned long triggerScan(int interfaceIndex) {
  Handle handle;
  Interfaces interfaces{handle};
  auto guid = interfaces.guidByIndex(interfaceIndex);

  HRESULT result = 0;

  WLAN_CALLBACK_INFO callbackInfo = {
      guid, CreateEvent(nullptr, FALSE, FALSE, nullptr), 0};

  result = WlanRegisterNotification(handle, WLAN_NOTIFICATION_SOURCE_ACM, false,
                                    (WLAN_NOTIFICATION_CALLBACK)scanCallback,
                                    (PVOID)&callbackInfo, NULL, NULL);
  if (result != ERROR_SUCCESS) {
    std::error_code ec(result, std::system_category());
    throw std::system_error(ec);
  }

  result = WlanScan(handle, &guid, NULL, NULL, NULL);
  if (result != ERROR_SUCCESS) {
    std::error_code ec(result, std::system_category());
    throw std::system_error(ec);
  }

  unsigned long waitResult = WaitForSingleObject(callbackInfo.event, 5000);
  if (waitResult == WAIT_OBJECT_0 &&
      callbackInfo.notificationCode != wlan_notification_acm_scan_complete) {
    waitResult = 254;
  }

  return waitResult;
}

} // namespace WLANAPI
