/*
 * Test to check that pre-existing cards are given a UID,
 * and that the UID remains the same after a shutdown.
 */
Cu.importGlobalProperties(["fetch"]);

var profD = do_get_profile();

// Tests that directories have UIDs.
add_test(function directoryUID() {
  newAddressBookFile();

  let books = MailServices.ab.directories;
  while (books.hasMoreElements()) {
    let book = books.getNext().QueryInterface(Ci.nsIAbDirectory);
    equal(36, book.UID.length, "Existing directory has a UID");
  }

  let dirName = MailServices.ab.newAddressBook("test", "", kPABData.dirType);
  let directory = MailServices.ab.getDirectoryFromId(dirName);
  equal(36, directory.UID.length, "New directory has a UID");

  run_next_test();
});

// Tests that an existing contact has a UID generated, and that that UID is
// saved to the database so that the same UID is used next time.
add_task(async function existingContactUID() {
  let book = newAddressBookFile();

  let card = getChildCards(book).find(c => !c.isMailList);
  equal(36, card.UID.length, "Existing contact has a UID");

  let existingUID = card.UID;
  card = getChildCards(book).find(c => !c.isMailList);
  equal(existingUID, card.UID, "New reference to contact has the same UID");

  await checkFileForUID(card.UID, book.fileName);
});

// Tests that new contacts have UIDs. Do this test last so we don't muck up
// the others by adding new things to the address book.
add_task(async function newContactUID() {
  let book = newAddressBookFile();

  let contact = Cc["@mozilla.org/addressbook/cardproperty;1"].createInstance(Ci.nsIAbCard);
  let newContact = book.addCard(contact);
  equal(36, newContact.UID.length, "New contact has a UID");

  await checkFileForUID(newContact.UID, book.fileName);
});

// Tests that existing lists have UIDs. Reference the nsIAbCard first.
add_task(async function existingListUID1() {
  let book = newAddressBookFile();

  let card = getChildCards(book).find(c => c.isMailList);
  equal(36, card.UID.length, "Existing list's card has a UID");

  let directory = MailServices.ab.getDirectory(card.mailListURI);
  equal(36, directory.UID.length, "Existing list's directory has a UID");

  equal(card.UID, directory.UID, "Existing list's card and directory UIDs match");

  await checkFileForUID(card.UID, book.fileName);
});

// Tests that existing lists have UIDs. Reference the nsIAbDirectory first.
add_task(async function existingListUID2() {
  let book = newAddressBookFile();

  let directory = MailServices.ab.getDirectory(`${book.URI}/MailList1`);
  equal(36, directory.UID.length, "Existing list's directory has a UID");

  let card = getChildCards(book).find(c => c.isMailList);
  equal(36, card.UID.length, "Existing list's card has a UID");

  equal(card.UID, directory.UID, "Existing list's card and directory UIDs match");

  await checkFileForUID(card.UID, book.fileName);
});

// Tests that new lists have UIDs.
add_task(async function newListUID() {
  let book = newAddressBookFile();

  let list = Cc["@mozilla.org/addressbook/directoryproperty;1"].createInstance(Ci.nsIAbDirectory);
  list = book.addMailList(list);

  equal(36, list.UID.length, "New list's directory has a UID");
  equal(list.UID, MailServices.ab.getDirectory(list.URI).UID, "New reference to list's directory has the same UID");

  let bookCards = getChildCards(book);
  ok(!!bookCards.find(c => c.UID == list.UID, "New reference to list has the same UID"));

  await checkFileForUID(list.UID, book.fileName);
});

// 3 seems to be the lowest number that works here. I don't know why.
var count = 3;
function newAddressBookFile() {
  let foo = MailServices.ab.newAddressBook(`book${count}`, `moz-abmdbdirectory://abook-${count}.mab`, 2);

  let testAB = do_get_file("data/existing.mab");
  testAB.copyTo(profD, `abook-${count}.mab`);

  Services.prefs.setCharPref(`ldap_2.servers.book${count}.filename`, `abook-${count}.mab`);

  let book = MailServices.ab.getDirectory(`moz-abmdbdirectory://abook-${count}.mab`);
  equal(2, getChildCards(book).length);

  count++;
  return book;
}

async function checkFileForUID(needle, bookFileName) {
  let abFile = profD.clone();
  abFile.append(bookFileName);
  let response = await fetch(Services.io.newFileURI(abFile).spec);
  let text = await response.text();

  ok(text.includes(needle), "UID has been saved to file");
}

function getChildCards(book) {
  let bookCards = [];
  childCards = book.childCards;
  while (childCards.hasMoreElements())
    bookCards.push(childCards.getNext().QueryInterface(Ci.nsIAbCard));
  return bookCards;
}
