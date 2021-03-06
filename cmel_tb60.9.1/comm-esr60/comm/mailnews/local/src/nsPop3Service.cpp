/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "msgCore.h"    // precompiled header...

#include "nsPop3Service.h"
#include "nsIMsgIncomingServer.h"
#include "nsIPop3IncomingServer.h"

#include "nsPop3URL.h"
#include "nsPop3Sink.h"
#include "nsPop3Protocol.h"
#include "nsMsgLocalCID.h"
#include "nsMsgBaseCID.h"
#include "nsCOMPtr.h"
#include "nsIMsgWindow.h"
#include "nsINetUtil.h"

#include "nsIRDFService.h"
#include "nsRDFCID.h"
#include "nsIDirectoryService.h"
#include "nsMailDirServiceDefs.h"
#include "prprf.h"
#include "nsMsgUtils.h"
#include "nsIMsgAccountManager.h"
#include "nsIMsgAccount.h"
#include "nsLocalMailFolder.h"
#include "nsIMailboxUrl.h"
#include "nsIPrompt.h"
#include "nsINetUtil.h"
#include "nsComponentManagerUtils.h"
#include "nsServiceManagerUtils.h"
#include "mozilla/Services.h"

#define PREF_MAIL_ROOT_POP3 "mail.root.pop3"        // old - for backward compatibility only
#define PREF_MAIL_ROOT_POP3_REL "mail.root.pop3-rel"

static NS_DEFINE_CID(kPop3UrlCID, NS_POP3URL_CID);
static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);

nsPop3Service::nsPop3Service()
{
}

nsPop3Service::~nsPop3Service()
{}

NS_IMPL_ISUPPORTS(nsPop3Service,
                   nsIPop3Service,
                   nsIProtocolHandler,
                   nsIMsgProtocolInfo)

NS_IMETHODIMP nsPop3Service::CheckForNewMail(nsIMsgWindow* aMsgWindow,
                                             nsIUrlListener *aUrlListener,
                                             nsIMsgFolder *aInbox,
                                             nsIPop3IncomingServer *aPopServer,
                                             nsIURI **aURL)
{
  return GetMail(false /* don't download, just check */,
                 aMsgWindow, aUrlListener, aInbox, aPopServer, aURL);
}


nsresult nsPop3Service::GetNewMail(nsIMsgWindow *aMsgWindow,
                                   nsIUrlListener *aUrlListener,
                                   nsIMsgFolder *aInbox,
                                   nsIPop3IncomingServer *aPopServer,
                                   nsIURI **aURL)
{
  return GetMail(true /* download */,
                 aMsgWindow, aUrlListener, aInbox, aPopServer, aURL);
}

nsresult nsPop3Service::GetMail(bool downloadNewMail,
                                nsIMsgWindow *aMsgWindow,
                                nsIUrlListener *aUrlListener,
                                nsIMsgFolder *aInbox,
                                nsIPop3IncomingServer *aPopServer,
                                nsIURI **aURL)
{

  NS_ENSURE_ARG_POINTER(aInbox);
  int32_t popPort = -1;

  nsCOMPtr<nsIMsgIncomingServer> server;
  nsCOMPtr<nsIURI> url;

  server = do_QueryInterface(aPopServer);
  NS_ENSURE_TRUE(server, NS_MSG_INVALID_OR_MISSING_SERVER);

  nsCOMPtr<nsIMsgLocalMailFolder> destLocalFolder = do_QueryInterface(aInbox);
  if (destLocalFolder)
  {
    // We don't know the needed size yet, so at least check
    // if there is some free space (1MB) in the message store.
    bool destFolderTooBig;
    destLocalFolder->WarnIfLocalFileTooBig(aMsgWindow, 0xFFFF, &destFolderTooBig);
    if (destFolderTooBig)
      return NS_MSG_ERROR_WRITING_MAIL_FOLDER;
  }

  nsCString popHost;
  nsCString popUser;
  nsresult rv = server->GetHostName(popHost);
  NS_ENSURE_SUCCESS(rv, rv);
  if (popHost.IsEmpty())
    return NS_MSG_INVALID_OR_MISSING_SERVER;

  rv = server->GetPort(&popPort);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = server->GetUsername(popUser);
  NS_ENSURE_SUCCESS(rv, rv);
  if (popUser.IsEmpty())
    return NS_MSG_SERVER_USERNAME_MISSING;

  nsCString escapedUsername;
  MsgEscapeString(popUser, nsINetUtil::ESCAPE_XALPHAS, escapedUsername);

  if (NS_SUCCEEDED(rv) && aPopServer)
  {
    // now construct a pop3 url...
    // we need to escape the username because it may contain
    // characters like / % or @
    char * urlSpec = (downloadNewMail)
      ? PR_smprintf("pop3://%s@%s:%d", escapedUsername.get(), popHost.get(), popPort)
      : PR_smprintf("pop3://%s@%s:%d/?check", escapedUsername.get(), popHost.get(), popPort);
    rv = BuildPop3Url(urlSpec, aInbox, aPopServer, aUrlListener, getter_AddRefs(url), aMsgWindow);
    PR_smprintf_free(urlSpec);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  NS_ENSURE_TRUE(url, rv);

  if (NS_SUCCEEDED(rv))
    rv = RunPopUrl(server, url);

  if (aURL) // we already have a ref count on pop3url...
    url.forget(aURL);

  return rv;
}

NS_IMETHODIMP nsPop3Service::VerifyLogon(nsIMsgIncomingServer *aServer,
                                         nsIUrlListener *aUrlListener,
                                         nsIMsgWindow *aMsgWindow,
                                         nsIURI **aURL)
{
  NS_ENSURE_ARG_POINTER(aServer);
  nsCString popHost;
  nsCString popUser;
  int32_t popPort = -1;

  nsresult rv = aServer->GetHostName(popHost);
  NS_ENSURE_SUCCESS(rv, rv);

  if (popHost.IsEmpty())
    return NS_MSG_INVALID_OR_MISSING_SERVER;

  rv = aServer->GetPort(&popPort);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = aServer->GetUsername(popUser);
  NS_ENSURE_SUCCESS(rv, rv);

  if (popUser.IsEmpty())
    return NS_MSG_SERVER_USERNAME_MISSING;

  nsCString escapedUsername;
  MsgEscapeString(popUser, nsINetUtil::ESCAPE_XALPHAS, escapedUsername);

  nsCOMPtr<nsIPop3IncomingServer> popServer = do_QueryInterface(aServer, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  // now construct a pop3 url...
  // we need to escape the username because it may contain
  // characters like / % or @
  char *urlSpec = PR_smprintf("pop3://%s@%s:%d/?verifyLogon",
                              escapedUsername.get(), popHost.get(), popPort);
  NS_ENSURE_TRUE(urlSpec, NS_ERROR_OUT_OF_MEMORY);

  nsCOMPtr<nsIURI> url;
  rv = BuildPop3Url(urlSpec, nullptr, popServer, aUrlListener,
                    getter_AddRefs(url), aMsgWindow);
  PR_smprintf_free(urlSpec);

  if (NS_SUCCEEDED(rv) && url)
  {
    rv = RunPopUrl(aServer, url);
    if (NS_SUCCEEDED(rv) && aURL)
      url.forget(aURL);
  }

  return rv;
}

nsresult nsPop3Service::BuildPop3Url(const char *urlSpec,
                                     nsIMsgFolder *inbox,
                                     nsIPop3IncomingServer *server,
                                     nsIUrlListener *aUrlListener,
                                     nsIURI **aUrl,
                                     nsIMsgWindow *aMsgWindow)
{
  nsresult rv;

  nsPop3Sink *pop3Sink = new nsPop3Sink();

  NS_ENSURE_TRUE(pop3Sink, NS_ERROR_OUT_OF_MEMORY);

  pop3Sink->SetPopServer(server);
  pop3Sink->SetFolder(inbox);

  // now create a pop3 url and a protocol instance to run the url....
  nsCOMPtr<nsIPop3URL> pop3Url = do_CreateInstance(kPop3UrlCID, &rv);
  NS_ENSURE_SUCCESS(rv,rv);

  pop3Url->SetPop3Sink(pop3Sink);

  nsCOMPtr<nsIMsgMailNewsUrl> mailnewsurl;
  rv = pop3Url->QueryInterface(NS_GET_IID(nsIMsgMailNewsUrl), getter_AddRefs(mailnewsurl));
  NS_ENSURE_SUCCESS(rv,rv);

  rv = mailnewsurl->SetSpecInternal(nsDependentCString(urlSpec));
  NS_ENSURE_SUCCESS(rv,rv);

  if (aUrlListener)
    mailnewsurl->RegisterListener(aUrlListener);
  if (aMsgWindow)
    mailnewsurl->SetMsgWindow(aMsgWindow);

  mailnewsurl.forget(aUrl);

  return rv;
}

nsresult nsPop3Service::RunPopUrl(nsIMsgIncomingServer *aServer, nsIURI *aUrlToRun)
{

  NS_ENSURE_ARG_POINTER(aServer);
  NS_ENSURE_ARG_POINTER(aUrlToRun);

  nsCString userName;

  // load up required server information
  // we store the username unescaped in the server
  // so there is no need to unescape it
  nsresult rv = aServer->GetRealUsername(userName);

  // find out if the server is busy or not...if the server is busy, we are
  // *NOT* going to run the url
  bool serverBusy = false;
  rv = aServer->GetServerBusy(&serverBusy);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!serverBusy)
  {
    RefPtr<nsPop3Protocol> protocol = new nsPop3Protocol(aUrlToRun);
    if (protocol)
    {
      // the protocol stores the unescaped username, so there is no need to escape it.
      protocol->SetUsername(userName.get());
      rv = protocol->LoadUrl(aUrlToRun);
      if (NS_FAILED(rv))
        aServer->SetServerBusy(false);
    }
  }
  else
  {
    nsCOMPtr<nsIMsgMailNewsUrl> url = do_QueryInterface(aUrlToRun);
    if (url)
      AlertServerBusy(url);
    rv = NS_ERROR_FAILURE;
  }
  return rv;
}


NS_IMETHODIMP nsPop3Service::GetScheme(nsACString &aScheme)
{
    aScheme.AssignLiteral("pop3");
    return NS_OK;
}

NS_IMETHODIMP nsPop3Service::GetDefaultPort(int32_t *aDefaultPort)
{
    NS_ENSURE_ARG_POINTER(aDefaultPort);
    *aDefaultPort = nsIPop3URL::DEFAULT_POP3_PORT;
    return NS_OK;
}

NS_IMETHODIMP nsPop3Service::AllowPort(int32_t port, const char *scheme, bool *_retval)
{
    *_retval = true; // allow pop on any port
    return NS_OK;
}

NS_IMETHODIMP nsPop3Service::GetDefaultDoBiff(bool *aDoBiff)
{
    NS_ENSURE_ARG_POINTER(aDoBiff);
    // by default, do biff for POP3 servers
    *aDoBiff = true;
    return NS_OK;
}

NS_IMETHODIMP nsPop3Service::GetProtocolFlags(uint32_t *result)
{
    NS_ENSURE_ARG_POINTER(result);
    *result = URI_NORELATIVE | URI_DANGEROUS_TO_LOAD | ALLOWS_PROXY |
              URI_FORBIDS_COOKIE_ACCESS;
    return NS_OK;
}

NS_IMETHODIMP nsPop3Service::NewURI(const nsACString &aSpec,
                                    const char *aOriginCharset, // ignored
                                    nsIURI *aBaseURI,
                                    nsIURI **_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);

    nsAutoCString folderUri(aSpec);
    nsCOMPtr<nsIRDFResource> resource;
    int32_t offset = folderUri.FindChar('?');
    if (offset != kNotFound)
      folderUri.SetLength(offset);

    const char *uidl = PL_strstr(nsCString(aSpec).get(), "uidl=");
    NS_ENSURE_TRUE(uidl, NS_ERROR_FAILURE);

    nsresult rv;

    nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = rdfService->GetResource(folderUri, getter_AddRefs(resource));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIMsgFolder> folder = do_QueryInterface(resource, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIMsgIncomingServer> server;

    nsLocalFolderScanState folderScanState;
    nsCOMPtr<nsIMsgLocalMailFolder> localFolder = do_QueryInterface(folder);
    nsCOMPtr<nsIMailboxUrl> mailboxUrl = do_QueryInterface(aBaseURI);

    if (mailboxUrl && localFolder)
    {
      rv = localFolder->GetFolderScanState(&folderScanState);
      NS_ENSURE_SUCCESS(rv, rv);
      nsCOMPtr<nsIMsgDBHdr> msgHdr;
      nsMsgKey msgKey;
      mailboxUrl->GetMessageKey(&msgKey);
      folder->GetMessageHeader(msgKey, getter_AddRefs(msgHdr));
      // we do this to get the account key
      if (msgHdr)
        localFolder->GetUidlFromFolder(&folderScanState, msgHdr);
      if (!folderScanState.m_accountKey.IsEmpty())
      {
        nsCOMPtr<nsIMsgAccountManager> accountManager =
                 do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
        if (accountManager)
        {
          nsCOMPtr<nsIMsgAccount> account;
          accountManager->GetAccount(folderScanState.m_accountKey, getter_AddRefs(account));
          if (account)
            account->GetIncomingServer(getter_AddRefs(server));
        }
      }
    }

    if (!server)
      rv = folder->GetServer(getter_AddRefs(server));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIPop3IncomingServer> popServer = do_QueryInterface(server,&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCString hostname;
    nsCString username;
    server->GetHostName(hostname);
    server->GetUsername(username);

    int32_t port;
    server->GetPort(&port);
    if (port == -1) port = nsIPop3URL::DEFAULT_POP3_PORT;

    // We need to escape the username before calling SetUsername() because it may
    // contain characters like / % or @. GetUsername() will unescape the username.
    nsCString escapedUsername;
    MsgEscapeString(username, nsINetUtil::ESCAPE_XALPHAS, escapedUsername);

    nsAutoCString popSpec("pop://");
    popSpec += escapedUsername;
    popSpec += "@";
    popSpec += hostname;
    popSpec += ":";
    popSpec.AppendInt(port);
    popSpec += "?";
    popSpec += uidl;
    nsCOMPtr<nsIUrlListener> urlListener = do_QueryInterface(folder, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIURI> newUri;
    rv = BuildPop3Url(popSpec.get(), folder, popServer,
                      urlListener, getter_AddRefs(newUri), nullptr);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIMsgMailNewsUrl> mailnewsurl = do_QueryInterface(newUri, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mailnewsurl->SetUsernameInternal(escapedUsername);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIPop3URL> popurl = do_QueryInterface(newUri, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsAutoCString messageUri (aSpec);
    if (!strncmp(messageUri.get(), "mailbox:", 8))
      messageUri.Replace(0, 8, "mailbox-message:");
    offset = messageUri.Find("?number=");
    if (offset != kNotFound)
      messageUri.Replace(offset, 8, "#");
    offset = messageUri.FindChar('&');
    if (offset != kNotFound)
      messageUri.SetLength(offset);
    popurl->SetMessageUri(messageUri.get());
    nsCOMPtr<nsIPop3Sink> pop3Sink;
    rv = popurl->GetPop3Sink(getter_AddRefs(pop3Sink));
    NS_ENSURE_SUCCESS(rv, rv);

    pop3Sink->SetBuildMessageUri(true);

    newUri.forget(_retval);
    return NS_OK;
}

void nsPop3Service::AlertServerBusy(nsIMsgMailNewsUrl *url)
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> bundleService =
    mozilla::services::GetStringBundleService();
  if (!bundleService)
    return;
  nsCOMPtr<nsIStringBundle> bundle;
  rv = bundleService->CreateBundle("chrome://messenger/locale/localMsgs.properties", getter_AddRefs(bundle));
  NS_ENSURE_SUCCESS_VOID(rv);

  nsCOMPtr<nsIMsgWindow> msgWindow;
  nsCOMPtr<nsIPrompt> dialog;
  rv = url->GetMsgWindow(getter_AddRefs(msgWindow)); //it is ok to have null msgWindow, for example when biffing
  if (NS_FAILED(rv) || !msgWindow)
    return;

  rv = msgWindow->GetPromptDialog(getter_AddRefs(dialog));
  NS_ENSURE_SUCCESS_VOID(rv);

  nsString accountName;
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = url->GetServer(getter_AddRefs(server));
  NS_ENSURE_SUCCESS_VOID(rv);
  rv = server->GetPrettyName(accountName);
  NS_ENSURE_SUCCESS_VOID(rv);

  const char16_t *params[] = { accountName.get() };
  nsString alertString;
  nsString dialogTitle;
  bundle->FormatStringFromName(
    "pop3ServerBusy",
    params, 1, alertString);
  bundle->FormatStringFromName(
    "pop3ErrorDialogTitle",
    params, 1, dialogTitle);
  if (!alertString.IsEmpty())
    dialog->Alert(dialogTitle.get(), alertString.get());
}

NS_IMETHODIMP nsPop3Service::NewChannel(nsIURI *aURI, nsIChannel **_retval)
{
  return NewChannel2(aURI, nullptr, _retval);
}

NS_IMETHODIMP nsPop3Service::NewChannel2(nsIURI *aURI,
                                         nsILoadInfo *aLoadInfo,
                                         nsIChannel **_retval)
{
  NS_ENSURE_ARG_POINTER(aURI);
  nsresult rv;

  nsCOMPtr<nsIMsgMailNewsUrl> url = do_QueryInterface(aURI, &rv);
  nsCString realUserName;
  if (NS_SUCCEEDED(rv) && url)
  {
    nsCOMPtr<nsIMsgIncomingServer> server;
    url->GetServer(getter_AddRefs(server));
    if (server)
    {
      // find out if the server is busy or not...if the server is busy, we are
      // *NOT* going to run the url. The error code isn't quite right...
      // We might want to put up an error right here.
      bool serverBusy = false;
      rv = server->GetServerBusy(&serverBusy);
      if (serverBusy)
      {
        AlertServerBusy(url);
        return NS_MSG_FOLDER_BUSY;
      }
      server->GetRealUsername(realUserName);
    }
  }

  RefPtr<nsPop3Protocol> protocol = new nsPop3Protocol(aURI);
  NS_ENSURE_TRUE(protocol, NS_ERROR_OUT_OF_MEMORY);

  rv = protocol->Initialize(aURI);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = protocol->SetLoadInfo(aLoadInfo);
  NS_ENSURE_SUCCESS(rv, rv);

  protocol->SetUsername(realUserName.get());

  return CallQueryInterface(protocol, _retval);
}


NS_IMETHODIMP
nsPop3Service::SetDefaultLocalPath(nsIFile *aPath)
{
    NS_ENSURE_ARG(aPath);
    return NS_SetPersistentFile(PREF_MAIL_ROOT_POP3_REL, PREF_MAIL_ROOT_POP3, aPath);
}

NS_IMETHODIMP
nsPop3Service::GetDefaultLocalPath(nsIFile **aResult)
{
    NS_ENSURE_ARG_POINTER(aResult);
    *aResult = nullptr;

    bool havePref;
    nsCOMPtr<nsIFile> localFile;
    nsresult rv = NS_GetPersistentFile(PREF_MAIL_ROOT_POP3_REL,
                                       PREF_MAIL_ROOT_POP3,
                                       NS_APP_MAIL_50_DIR,
                                       havePref,
                                       getter_AddRefs(localFile));
    NS_ENSURE_SUCCESS(rv, rv);

    bool exists;
    rv = localFile->Exists(&exists);
    if (NS_SUCCEEDED(rv) && !exists)
        rv = localFile->Create(nsIFile::DIRECTORY_TYPE, 0775);
    NS_ENSURE_SUCCESS(rv, rv);

    if (!havePref || !exists) {
        rv = NS_SetPersistentFile(PREF_MAIL_ROOT_POP3_REL, PREF_MAIL_ROOT_POP3, localFile);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to set root dir pref.");
    }

    localFile.forget(aResult);
    return NS_OK;
}


NS_IMETHODIMP
nsPop3Service::GetServerIID(nsIID **aServerIID)
{
    *aServerIID = new nsIID(NS_GET_IID(nsIPop3IncomingServer));
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetRequiresUsername(bool *aRequiresUsername)
{
    NS_ENSURE_ARG_POINTER(aRequiresUsername);
    *aRequiresUsername = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetPreflightPrettyNameWithEmailAddress(bool *aPreflightPrettyNameWithEmailAddress)
{
    NS_ENSURE_ARG_POINTER(aPreflightPrettyNameWithEmailAddress);
    *aPreflightPrettyNameWithEmailAddress = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetCanLoginAtStartUp(bool *aCanLoginAtStartUp)
{
    NS_ENSURE_ARG_POINTER(aCanLoginAtStartUp);
    *aCanLoginAtStartUp = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetCanDelete(bool *aCanDelete)
{
    NS_ENSURE_ARG_POINTER(aCanDelete);
    *aCanDelete = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetCanDuplicate(bool *aCanDuplicate)
{
    NS_ENSURE_ARG_POINTER(aCanDuplicate);
    *aCanDuplicate = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetCanGetMessages(bool *aCanGetMessages)
{
    NS_ENSURE_ARG_POINTER(aCanGetMessages);
    *aCanGetMessages = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetCanGetIncomingMessages(bool *aCanGetIncomingMessages)
{
    NS_ENSURE_ARG_POINTER(aCanGetIncomingMessages);
    *aCanGetIncomingMessages = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetShowComposeMsgLink(bool *showComposeMsgLink)
{
    NS_ENSURE_ARG_POINTER(showComposeMsgLink);
    *showComposeMsgLink = true;
    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetFoldersCreatedAsync(bool *aAsyncCreation)
{
  NS_ENSURE_ARG_POINTER(aAsyncCreation);
  *aAsyncCreation = false;
  return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::GetDefaultServerPort(bool isSecure, int32_t *aPort)
{
    NS_ENSURE_ARG_POINTER(aPort);

    if (!isSecure)
      return GetDefaultPort(aPort);

    *aPort = nsIPop3URL::DEFAULT_POP3S_PORT;

    return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::NotifyDownloadStarted(nsIMsgFolder *aFolder)
{
  nsTObserverArray<nsCOMPtr<nsIPop3ServiceListener> >::ForwardIterator
    iter(mListeners);
  nsCOMPtr<nsIPop3ServiceListener> listener;
  while (iter.HasMore()) {
    listener = iter.GetNext();
    listener->OnDownloadStarted(aFolder);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::NotifyDownloadProgress(nsIMsgFolder *aFolder,
                                      uint32_t aNumMessages,
                                      uint32_t aNumTotalMessages)
{
  nsTObserverArray<nsCOMPtr<nsIPop3ServiceListener> >::ForwardIterator
    iter(mListeners);
  nsCOMPtr<nsIPop3ServiceListener> listener;
  while (iter.HasMore()) {
    listener = iter.GetNext();
    listener->OnDownloadProgress(aFolder, aNumMessages, aNumTotalMessages);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsPop3Service::NotifyDownloadCompleted(nsIMsgFolder *aFolder,
                                       uint32_t aNumMessages)
{
  nsTObserverArray<nsCOMPtr<nsIPop3ServiceListener> >::ForwardIterator
    iter(mListeners);
  nsCOMPtr<nsIPop3ServiceListener> listener;
  while (iter.HasMore()) {
    listener = iter.GetNext();
    listener->OnDownloadCompleted(aFolder, aNumMessages);
  }
  return NS_OK;
}

NS_IMETHODIMP nsPop3Service::AddListener(nsIPop3ServiceListener *aListener)
{
  NS_ENSURE_ARG_POINTER(aListener);
  mListeners.AppendElementUnlessExists(aListener);
  return NS_OK;
}

NS_IMETHODIMP nsPop3Service::RemoveListener(nsIPop3ServiceListener *aListener)
{
  NS_ENSURE_ARG_POINTER(aListener);
  mListeners.RemoveElement(aListener);
  return NS_OK;
}
