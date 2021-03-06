/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef TelemetryTestHelpers_h_
#define TelemetryTestHelpers_h_

#include "js/TypeDecls.h"
#include "nsITelemetry.h"

namespace TelemetryTestHelpers {

void CheckUintScalar(const char* aName, JSContext* aCx,
                     JS::HandleValue aSnapshot, uint32_t expectedValue);

void CheckBoolScalar(const char* aName, JSContext* aCx,
                     JS::HandleValue aSnapshot, bool expectedValue);

void CheckStringScalar(const char* aName, JSContext* aCx,
                       JS::HandleValue aSnapshot, const char* expectedValue);

void CheckKeyedUintScalar(const char* aName, const char* aKey, JSContext* aCx,
                          JS::HandleValue aSnapshot, uint32_t expectedValue);

void CheckKeyedBoolScalar(const char* aName, const char* aKey, JSContext* aCx,
                          JS::HandleValue aSnapshot, bool expectedValue);

void CheckNumberOfProperties(const char* aName, JSContext* aCx,
                             JS::HandleValue aSnapshot,
                             uint32_t expectedNumProperties);

void GetScalarsSnapshot(bool aKeyed, JSContext* aCx,
                        JS::MutableHandle<JS::Value> aResult);

void GetAndClearHistogram(JSContext* cx, nsCOMPtr<nsITelemetry> mTelemetry,
                          const nsACString& name, bool is_keyed);

void GetProperty(JSContext* cx, const char* name, JS::HandleValue valueIn,
                 JS::MutableHandleValue valueOut);

void GetElement(JSContext* cx, uint32_t index, JS::HandleValue valueIn,
                JS::MutableHandleValue valueOut);

void GetSnapshots(JSContext* cx, nsCOMPtr<nsITelemetry> mTelemetry,
                  const char* name, JS::MutableHandleValue valueOut,
                  bool is_keyed);

}  // namespace TelemetryTestHelpers

#endif
