/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * A common base class for representing WebIDL callback interface types in C++.
 *
 * This class implements common functionality like lifetime management,
 * initialization with the callback object, and setup of the call environment.
 * Subclasses corresponding to particular callback interface types should
 * provide methods that actually do the various necessary calls.
 */

#ifndef mozilla_dom_CallbackInterface_h
#define mozilla_dom_CallbackInterface_h

#include "mozilla/dom/CallbackObject.h"

namespace mozilla {
namespace dom {

class CallbackInterface : public CallbackObject {
 public:
  // See CallbackObject for an explanation of the arguments.
  explicit CallbackInterface(JSContext* aCx, JS::Handle<JSObject*> aCallback,
                             nsIGlobalObject* aIncumbentGlobal)
      : CallbackObject(aCx, aCallback, aIncumbentGlobal) {}

  // See CallbackObject for an explanation of the arguments.
  explicit CallbackInterface(JS::Handle<JSObject*> aCallback,
                             JS::Handle<JSObject*> aAsyncStack,
                             nsIGlobalObject* aIncumbentGlobal)
      : CallbackObject(aCallback, aAsyncStack, aIncumbentGlobal) {}

 protected:
  bool GetCallableProperty(JSContext* cx, JS::Handle<jsid> aPropId,
                           JS::MutableHandle<JS::Value> aCallable);

  // See CallbackObject for an explanation of the arguments.
  CallbackInterface(JS::Handle<JSObject*> aCallable,
                    const FastCallbackConstructor&)
      : CallbackObject(aCallable, FastCallbackConstructor()) {}
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_CallbackFunction_h
