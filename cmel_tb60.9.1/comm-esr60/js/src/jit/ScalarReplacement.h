/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file declares scalar replacement of objects transformation.
#ifndef jit_ScalarReplacement_h
#define jit_ScalarReplacement_h

#include "mozilla/Attributes.h"

namespace js {
namespace jit {

class MIRGenerator;
class MIRGraph;

MOZ_MUST_USE bool ScalarReplacement(MIRGenerator* mir, MIRGraph& graph);

}  // namespace jit
}  // namespace js

#endif /* jit_ScalarReplacement_h */
