      <menu id="helpMenu"
#ifdef XP_WIN
            label="&helpMenuWin.label;" accesskey="&helpMenuWin.accesskey;">
#else
            label="&helpMenu.label;" accesskey="&helpMenu.accesskey;">
#endif
        <menupopup id="menu_HelpPopup">
          <menuitem id="menu_openHelp"
#ifdef XP_MACOSX
                    label="&openHelpMac2.label;"
#else
                    label="&openHelp.label;"
                    accesskey="&openHelp.accesskey;"
#endif
                    key="key_openHelp"
                    oncommand="openSupportURL();"/>
          <menuitem id="releaseNotes"
                    label="&releaseCmd2.label;" accesskey="&releaseCmd2.accesskey;"
                    oncommand="openFormattedURL('app.releaseNotesURL');"/>
          <menuitem id="whatsNew"
                    label="&whatsNewCmd.label;" accesskey="&whatsNewCmd.accesskey;"
                    oncommand="openWhatsNew();"/>
          <menuseparator id="functionsSeparator"/>
          <menuitem id="helpSafeMode"
                    label="&helpSafeMode.label;"
                    accesskey="&helpSafeMode.accesskey;"
                    oncommand="safeModeRestart();"/>
#ifndef XP_MACOSX
          <menuseparator id="aboutSeparator"/>
#endif
      <menuitem id="checkForUpdates"
#ifdef MOZ_UPDATER
                label="&updateCmd.label;"
                oncommand="checkForUpdates();"/>
#else
                hidden="true"/>
#endif
          <menuitem id="aboutName" hidden="true"
                    label="&aboutMenuCmd2.label;"
                    accesskey="&aboutMenuCmd2.accesskey;"
                    oncommand="openAboutDialog();"/>
        </menupopup>
      </menu>
