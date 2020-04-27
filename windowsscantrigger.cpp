#include "windowsscantrigger.h"
#include "wlanapiwrapper.h"

#include <system_error>
#include <winerror.h>

void WindowsScanTrigger::doScan(int interfaceIndex) {
  unsigned long waitResult;
  try {
    waitResult = WLANAPI::triggerScan(interfaceIndex);
  } catch (const std::system_error &e) {
    if (e.code() != std::error_code(ERROR_NOT_FOUND, std::system_category()))
      throw e;
    waitResult = 254;
  }

  emit resultReady(waitResult);
}
