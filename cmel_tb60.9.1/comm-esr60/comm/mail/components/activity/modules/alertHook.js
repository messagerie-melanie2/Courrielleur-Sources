/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

this.EXPORTED_SYMBOLS = ['alertHook'];

var nsActWarning = Components.Constructor("@mozilla.org/activity-warning;1",
                                            "nsIActivityWarning", "init");

ChromeUtils.import("resource://gre/modules/Services.jsm");
ChromeUtils.import("resource:///modules/mailServices.js");
ChromeUtils.import("resource://gre/modules/XPCOMUtils.jsm");

// This module provides a link between the send later service and the activity
// manager.
var alertHook =
{
  get activityMgr() {
    delete this.activityMgr;
    return this.activityMgr = Cc["@mozilla.org/activity-manager;1"]
                                .getService(Ci.nsIActivityManager);
  },

  get alertService() {
    delete this.alertService;
    return this.alertService = Cc["@mozilla.org/alerts-service;1"]
                                 .getService(Ci.nsIAlertsService);
  },

  get brandShortName() {
    delete this.brandShortName;
    return this.brandShortName = Services.strings
      .createBundle("chrome://branding/locale/brand.properties")
      .GetStringFromName("brandShortName");
  },

  QueryInterface: XPCOMUtils.generateQI([Ci.nsIMsgUserFeedbackListener]),

  onAlert: function (aMessage, aUrl) {
    
    // courrielleur
    if (-1!=aMessage.indexOf("Mailbox does not exist")){
      Services.console.logStringMessage("Mailbox does not exist");
      return true;
    }
    if (-1!=aMessage.indexOf("Invalid mailbox name")){
      Services.console.logStringMessage("Invalid mailbox name");
      return true;
    }
    //cas v2.4 /Le serveur de courrier .* n'est pas un serveur de type IMAP4/
    if (aMessage.match(/Le serveur de courrier .* n'est pas un serveur de type IMAP4/)){
      Services.console.logStringMessage(aMessage);
      return true;
    }
  
    //cm2 - mantis 3872
    if (aUrl && aUrl.server && !aUrl.isMessageUri && aUrl.folder){
      if (null!=aMessage.match(/.*Mailbox does not exist\.$/) &&
          null!=aUrl.folder.server.getCharValue("pacome.confid")){
        Services.console.logStringMessage("Mailbox does not exist pour dossier:"+aUrl.folder.URI);
        return true;
      }
    }
    //fin cm2 mantis 3872
    
    // Create a new warning.
    let warning = new nsActWarning(aMessage, this.activityMgr, "");

    if (aUrl && aUrl.server && aUrl.server.prettyName) {
      warning.groupingStyle = Ci.nsIActivity.GROUPING_STYLE_BYCONTEXT;
      warning.contextType = "incomingServer";
      warning.contextDisplayText = aUrl.server.prettyName;
      warning.contextObj = aUrl.server;
    }
    else
      warning.groupingStyle = Ci.nsIActivity.GROUPING_STYLE_STANDALONE;

    this.activityMgr.addActivity(warning);

    // If we have a message window in the url, then show a warning prompt,
    // just like the modal code used to. Otherwise, don't.
    try {
      if (!aUrl || !aUrl.msgWindow)
        return true;
    }
    // nsIMsgMailNewsUrl.msgWindow will throw on a null pointer, so that's
    // what we're handling here.
    catch (ex) {
      if (ex instanceof Ci.nsIException &&
          ex.result == Cr.NS_ERROR_INVALID_POINTER) {
        return true;
      } else {
        throw ex;
      }
    }

    try {
      this.alertService
          .showAlertNotification("chrome://branding/content/icon48.png",
                                 this.brandShortName, aMessage);
    }
    catch (ex) {
      // XXX On Linux, if libnotify isn't supported, showAlertNotification
      // can throw an error, so fall-back to the old method of modal dialogs.
      return false;
    }

    return true;
  },

  init: function() {
    // We shouldn't need to remove the listener as we're not being held by
    // anyone except by the send later instance.
    MailServices.mailSession.addUserFeedbackListener(this);
  }
};
