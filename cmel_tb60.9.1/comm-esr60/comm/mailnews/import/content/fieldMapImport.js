/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

var importService;
var fieldMap = null;
var recordNum = 0;
var addInterface = null;
var dialogResult = null;
var gPreviousButton;
var gNextButton;
var gMoveUpButton;
var gMoveDownButton;
var gListbox;
var gSkipFirstRecordButton;

function OnLoadFieldMapImport()
{
  top.importService = Cc["@mozilla.org/import/import-service;1"]
                        .getService(Ci.nsIImportService);

  // We need a field map object...
  // assume we have one passed in? or just make one?
  if (window.arguments && window.arguments[0]) {
    top.fieldMap = window.arguments[0].fieldMap;
    top.addInterface = window.arguments[0].addInterface;
    top.dialogResult = window.arguments[0].result;
  }
  if (top.fieldMap == null) {
    top.fieldMap = top.importService.CreateNewFieldMap();
    top.fieldMap.DefaultFieldMap( top.fieldMap.numMozFields);
  }

  gMoveUpButton = document.getElementById("upButton");
  gMoveDownButton = document.getElementById("downButton");
  gPreviousButton = document.getElementById("previous");
  gNextButton = document.getElementById("next");
  gListbox = document.getElementById("fieldList");
  gSkipFirstRecordButton = document.getElementById("skipFirstRecord");

  // Set the state of the skip first record button
  gSkipFirstRecordButton.checked = top.fieldMap.skipFirstRecord;

  ListFields();
  Browse(1);
  gListbox.selectedItem = gListbox.getItemAtIndex(0);
  disableMoveButtons();
}

function IndexInMap( index)
{
  var count = top.fieldMap.mapSize;
  for (var i = 0; i < count; i++) {
    if (top.fieldMap.GetFieldMap( i) == index)
      return( true);
  }

  return( false);
}

function ListFields() {
  if (top.fieldMap == null)
    return;

  var count = top.fieldMap.mapSize;
  var index;
  var i;
  for (i = 0; i < count; i++) {
    index = top.fieldMap.GetFieldMap( i);
    AddFieldToList(top.fieldMap.GetFieldDescription( index), index, top.fieldMap.GetFieldActive( i));
  }

  count = top.fieldMap.numMozFields;
  for (i = 0; i < count; i++) {
    if (!IndexInMap( i))
      AddFieldToList(top.fieldMap.GetFieldDescription( i), i, false);
  }
}

function CreateField(name, index, on)
{
  var item = document.createElement("listitem");
  item.setAttribute("field-index", index);
  item.setAttribute("allowevents", "true");

  var checkboxCell = document.createElement("listcell");
  checkboxCell.setAttribute("type", "checkbox");
  checkboxCell.addEventListener("click", cellClicked);
  if (on)
    checkboxCell.setAttribute("checked", "true");

  var firstCell = document.createElement("listcell");
  firstCell.setAttribute("label", name);

  var secondCell = document.createElement("listcell");
  secondCell.setAttribute("class", "importsampledata");
  secondCell.setAttribute("label", "");

  item.appendChild(checkboxCell);
  item.appendChild(firstCell);
  item.appendChild(secondCell);
  return item;
}

function AddFieldToList(name, index, on)
{
  var item = CreateField(name, index, on);
  gListbox.appendChild(item);
}

function cellClicked(event)
{
  if (event.button == 0) {
    var on = gListbox.selectedItem.firstChild.getAttribute('checked');
    gListbox.selectedItem.firstChild.setAttribute('checked', (on != "true"));
  }
}

// The "Move Up/Move Down" buttons should move the items in the left column
// up/down but the values in the right column should not change.
function moveItem(up)
{
  var selectedItem = gListbox.selectedItem;
  var swapPartner = (up ? gListbox.getPreviousItem(selectedItem, 1)
                        : gListbox.getNextItem(selectedItem, 1));

  var tmpLabel = swapPartner.lastChild.getAttribute('label');
  swapPartner.lastChild.setAttribute('label', selectedItem.lastChild.getAttribute('label'));
  selectedItem.lastChild.setAttribute('label', tmpLabel);

  var newItemPosition = (up ? selectedItem.nextSibling : selectedItem);
  gListbox.insertBefore(swapPartner, newItemPosition);
  gListbox.ensureElementIsVisible(selectedItem);
  disableMoveButtons();
}

function disableMoveButtons()
{
  var selectedIndex = gListbox.selectedIndex;
  gMoveUpButton.disabled = (selectedIndex == 0);
  gMoveDownButton.disabled = (selectedIndex == (gListbox.getRowCount() - 1));
}

function ShowSampleData(data)
{
  var fields = data.split("\n");
  for (var i = 0; i < gListbox.getRowCount(); i++)
    gListbox.getItemAtIndex(i).lastChild.setAttribute('label', (i < fields.length) ? fields[i] : '');
}

function FetchSampleData(num)
{
  if (!top.addInterface)
    return false;

  var data = top.addInterface.GetData( "sampleData-" + num);
  if (!(data instanceof Ci.nsISupportsString))
    return false;
  ShowSampleData( data.data);
  return true;
}

function Browse(step)
{
  recordNum += step;
  if (FetchSampleData(recordNum - 1))
    document.getElementById('recordNumber').setAttribute('value', ("" + recordNum));

  gPreviousButton.disabled = (recordNum == 1);
  gNextButton.disabled = (addInterface.GetData("sampleData-" + recordNum) == null);
}

function FieldImportOKButton()
{
  var max = gListbox.getRowCount();
  var fIndex;
  var on;
  // Ensure field map is the right size
  top.fieldMap.SetFieldMapSize(max);

  for (var i = 0; i < max; i++) {
    fIndex = gListbox.getItemAtIndex(i).getAttribute( 'field-index');
    on = gListbox.getItemAtIndex(i).firstChild.getAttribute('checked');
    top.fieldMap.SetFieldMap( i, fIndex);
    top.fieldMap.SetFieldActive( i, (on == "true"));
  }

  top.fieldMap.skipFirstRecord = gSkipFirstRecordButton.checked;

  top.dialogResult.ok = true;

  return true;
}
