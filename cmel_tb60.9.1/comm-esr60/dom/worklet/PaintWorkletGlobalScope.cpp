/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PaintWorkletGlobalScope.h"
#include "mozilla/dom/PaintWorkletGlobalScopeBinding.h"
#include "mozilla/dom/FunctionBinding.h"

namespace mozilla {
namespace dom {

PaintWorkletGlobalScope::PaintWorkletGlobalScope(nsPIDOMWindowInner* aWindow)
    : WorkletGlobalScope(aWindow) {}

PaintWorkletGlobalScope::~PaintWorkletGlobalScope() {}

bool PaintWorkletGlobalScope::WrapGlobalObject(
    JSContext* aCx, nsIPrincipal* aPrincipal,
    JS::MutableHandle<JSObject*> aReflector) {
  JS::CompartmentOptions options;
  return PaintWorkletGlobalScopeBinding::Wrap(aCx, this, this, options,
                                              nsJSPrincipals::get(aPrincipal),
                                              true, aReflector);
}

void PaintWorkletGlobalScope::RegisterPaint(const nsAString& aType,
                                            VoidFunction& aProcessorCtor) {
  // Nothing to do here, yet.
}

}  // namespace dom
}  // namespace mozilla
