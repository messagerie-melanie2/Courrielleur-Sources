// Default start page
pref("mailnews.start_page.url","http://etat.m2.e2.rie.gouv.fr/");

// start page override to load after an update
pref("mailnews.start_page.override_url","chrome://courrielleur/content/infomaj.xul");

// Interval: Time between checks for a new version (in seconds)
// nightly=8 hours, official=24 hours
pref("app.update.interval", 86400);

// The time interval between the downloading of mar file chunks in the
// background (in seconds)
pref("app.update.download.backgroundInterval", 600);

// Give the user x seconds to react before showing the big UI. default=24 hours
pref("app.update.promptWaitTime", 86400);

pref("app.vendorURL", "https://www.thunderbird.net/%LOCALE%/");

pref("browser.search.param.ms-pc", "MOZT");
