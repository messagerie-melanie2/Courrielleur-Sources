/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsMathMLParts_h___
#define nsMathMLParts_h___

#include "nscore.h"
#include "nsISupports.h"

class nsTableFrame;

// Factory methods for creating MathML objects
nsIFrame* NS_NewMathMLTokenFrame(nsIPresShell* aPresShell,
                                 nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmoFrame(nsIPresShell* aPresShell,
                              nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmrowFrame(nsIPresShell* aPresShell,
                                nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmpaddedFrame(nsIPresShell* aPresShell,
                                   nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmspaceFrame(nsIPresShell* aPresShell,
                                  nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmsFrame(nsIPresShell* aPresShell,
                              nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmfencedFrame(nsIPresShell* aPresShell,
                                   nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmfracFrame(nsIPresShell* aPresShell,
                                 nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmsubFrame(nsIPresShell* aPresShell,
                                nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmsupFrame(nsIPresShell* aPresShell,
                                nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmsubsupFrame(nsIPresShell* aPresShell,
                                   nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmunderoverFrame(nsIPresShell* aPresShell,
                                      nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmmultiscriptsFrame(nsIPresShell* aPresShell,
                                         nsStyleContext* aContext);
nsContainerFrame* NS_NewMathMLmtableOuterFrame(nsIPresShell* aPresShell,
                                               nsStyleContext* aContext);
nsContainerFrame* NS_NewMathMLmtableFrame(nsIPresShell* aPresShell,
                                          nsStyleContext* aContext);
nsContainerFrame* NS_NewMathMLmtrFrame(nsIPresShell* aPresShell,
                                       nsStyleContext* aContext);
nsContainerFrame* NS_NewMathMLmtdFrame(nsIPresShell* aPresShell,
                                       nsStyleContext* aContext,
                                       nsTableFrame* aTableFrame);
nsContainerFrame* NS_NewMathMLmtdInnerFrame(nsIPresShell* aPresShell,
                                            nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmsqrtFrame(nsIPresShell* aPresShell,
                                 nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmrootFrame(nsIPresShell* aPresShell,
                                 nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmactionFrame(nsIPresShell* aPresShell,
                                   nsStyleContext* aContext);
nsIFrame* NS_NewMathMLmencloseFrame(nsIPresShell* aPresShell,
                                    nsStyleContext* aContext);
nsIFrame* NS_NewMathMLsemanticsFrame(nsIPresShell* aPresShell,
                                     nsStyleContext* aContext);

nsContainerFrame* NS_NewMathMLmathBlockFrame(nsIPresShell* aPresShell,
                                             nsStyleContext* aContext);
nsContainerFrame* NS_NewMathMLmathInlineFrame(nsIPresShell* aPresShell,
                                              nsStyleContext* aContext);
#endif /* nsMathMLParts_h___ */
