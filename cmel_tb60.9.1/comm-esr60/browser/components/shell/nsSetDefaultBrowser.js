/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * --setDefaultBrowser commandline handler
 * Makes the current executable the "default browser".
 */

ChromeUtils.import("resource:///modules/ShellService.jsm");
ChromeUtils.import("resource://gre/modules/XPCOMUtils.jsm");

function nsSetDefaultBrowser() {}

nsSetDefaultBrowser.prototype = {
  handle: function nsSetDefault_handle(aCmdline) {
    if (aCmdline.handleFlag("setDefaultBrowser", false)) {
      ShellService.setDefaultBrowser(true, true);
    }
  },

  helpInfo: "  --setDefaultBrowser Set this app as the default browser.\n",

  classID: Components.ID("{F57899D0-4E2C-4ac6-9E29-50C736103B0C}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsICommandLineHandler]),
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([nsSetDefaultBrowser]);
