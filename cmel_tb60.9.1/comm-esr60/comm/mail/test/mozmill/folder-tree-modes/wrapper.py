# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# We install an extension that provides a test folder tree mode.

import os

def on_before_start(profile):
    """
    This installs the extension in the test-extension subdirectory into the
    profile folder. We cannot use on_profile_created here because
    install_plugin/install_addon depends on the profile object being fully
    initialized.
    """

    profile.addon_manager.install_addons(
        [os.path.join(os.path.dirname(__file__), "test-extension")])
