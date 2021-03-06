/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Main Composer window debug menu functions */

// --------------------------- Output ---------------------------


function EditorGetText()
{
  try {
    dump("Getting text\n");
    var  outputText = GetCurrentEditor().outputToString("text/plain", kOutputFormatted);
    dump("<<" + outputText + ">>\n");
  } catch (e) {}
}

function EditorGetHTML()
{
  try {
    dump("Getting HTML\n");
    var  outputHTML = GetCurrentEditor().outputToString("text/html", kOutputEncodeW3CEntities);
    dump(outputHTML + "\n");
  } catch (e) {}
}

function EditorInsertText(textToInsert)
{
  GetCurrentEditor().insertText(textToInsert);
}

function EditorTestSelection()
{
  dump("Testing selection\n");
  var selection = GetCurrentEditor().selection;
  if (!selection)
  {
    dump("No selection!\n");
    return;
  }

  dump("Selection contains:\n");
  // 3rd param = column to wrap
  dump(selection.QueryInterface(Ci.nsISelectionPrivate)
       .toStringWithFormat("text/plain",
                           kOutputFormatted | kOutputSelectionOnly,
                           0) + "\n");

  var output, i;

  dump("====== Selection as node and offsets==========\n");
  dump("rangeCount = " + selection.rangeCount + "\n");
  for (i = 0; i < selection.rangeCount; i++)
  {
    var range = selection.getRangeAt(i);
    if (range)
    {
      dump("Range "+i+": StartParent="+range.startContainer.nodeName+", offset="+range.startOffset+"\n");
      dump("Range "+i+":   EndParent="+range.endContainer.nodeName+", offset="+range.endOffset+"\n\n");
    }
  }

  var editor = GetCurrentEditor();

  dump("====== Selection as unformatted text ==========\n");
  output = editor.outputToString("text/plain", kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as formatted text ============\n");
  output = editor.outputToString("text/plain", kOutputFormatted | kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as HTML ======================\n");
  output = editor.outputToString("text/html", kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Selection as prettyprinted HTML ========\n");
  output = editor.outputToString("text/html", kOutputFormatted | kOutputSelectionOnly);
  dump(output + "\n\n");

  dump("====== Length and status =====================\n");
  output = "Document is ";
  if (editor.documentIsEmpty)
    output += "empty\n";
  else
    output += "not empty\n";
  output += "Text length is " + editor.textLength + " characters";
  dump(output + "\n\n");
}

function EditorTestTableLayout()
{
  dump("\n\n\n************ Dump Selection Ranges ************\n");
  var selection = GetCurrentEditor().selection;
  var i;
  for (i = 0; i < selection.rangeCount; i++)
  {
    var range = selection.getRangeAt(i);
    if (range)
    {
      dump("Range "+i+": StartParent="+range.startParent+", offset="+range.startOffset+"\n");
    }
  }
  dump("\n\n");

  var editor = GetCurrentEditor();
  var table = editor.getElementOrParentByTagName("table", null);
  if (!table) {
    dump("Enclosing Table not found: Place caret in a table cell to do this test\n\n");
    return;
  }

  var cell;
  var startRowIndexObj = { value: null };
  var startColIndexObj = { value: null };
  var rowSpanObj = { value: null };
  var colSpanObj = { value: null };
  var actualRowSpanObj = { value: null };
  var actualColSpanObj = { value: null };
  var isSelectedObj = { value: false };
  var startRowIndex = 0;
  var startColIndex = 0;
  var rowSpan;
  var colSpan;
  var actualRowSpan;
  var actualColSpan;
  var isSelected;
  var col = 0;
  var row = 0;
  var rowCount = 0;
  var maxColCount = 0;
  var doneWithRow = false;
  var doneWithCol = false;

  dump("\n\n\n************ Starting Table Layout test ************\n");

  // Note: We could also get the number of rows, cols and use for loops,
  //   but this tests using out-of-bounds offsets to detect end of row or column

  while (!doneWithRow)  // Iterate through rows
  {
    dump("* Data for ROW="+row+":\n");
    while(!doneWithCol)  // Iterate through cells in the row
    {
      try {
        cell = editor.getCellDataAt(table, row, col,
                                    startRowIndexObj, startColIndexObj,
                                    rowSpanObj, colSpanObj,
                                    actualRowSpanObj, actualColSpanObj,
                                    isSelectedObj);

        if (cell)
        {
          rowSpan = rowSpanObj.value;
          colSpan = colSpanObj.value;
          actualRowSpan = actualRowSpanObj.value;
          actualColSpan = actualColSpanObj.value;
          isSelected = isSelectedObj.value;

          dump(" Row="+row+", Col="+col+"  StartRow="+startRowIndexObj.value+", StartCol="+startColIndexObj.value+"\n");
          dump("  RowSpan="+rowSpan+", ColSpan="+colSpan+"  ActualRowSpan="+actualRowSpan+", ActualColSpan="+actualColSpan);
          if (isSelected)
            dump("  Cell is selected\n");
          else
            dump("  Cell is NOT selected\n");

          // Save the indexes of a cell that will span across the cellmap grid
          if (rowSpan > 1)
            startRowIndex = startRowIndexObj.value;
          if (colSpan > 1)
            startColIndex = startColIndexObj.value;

          // Initialize these for efficient spanned-cell search
          startRowIndexObj.value = startRowIndex;
          startColIndexObj.value = startColIndex;

          col++;
        } else {
          doneWithCol = true;
          // Get maximum number of cells in any row
          if (col > maxColCount)
            maxColCount = col;
          dump("  End of row found\n\n");
        }
      }
      catch (e) {
        dump("  *** GetCellDataAt failed at Row="+row+", Col="+col+" ***\n\n");
        return;
      }
    }
    if (col == 0) {
      // Didn't find a cell in the first col of a row,
      // thus no more rows in table
      doneWithRow = true;
      rowCount = row;
      dump("No more rows in table\n\n");
    } else {
      // Setup for next row
      col = 0;
      row++;
      doneWithCol = false;
    }
  }
  dump("Counted during scan: Number of rows="+rowCount+" Number of Columns="+maxColCount+"\n");
  rowCount = editor.getTableRowCount(table);
  maxColCount = editor.getTableColumnCount(table);
  dump("From nsITableLayout: Number of rows="+rowCount+" Number of Columns="+maxColCount+"\n****** End of Table Layout Test *****\n\n");
}

function EditorShowEmbeddedObjects()
{
  dump("\nEmbedded Objects:\n");
  try {
    var objectArray = GetCurrentEditor().getEmbeddedObjects();
    dump(objectArray.length + " embedded objects\n");
    for (let i = 0; i < objectArray.length; ++i)
      dump(objectArray.queryElementAt(i, Ci.nsIDOMNode) + "\n");
  } catch(e) {}
}

function EditorTestDocument()
{
  dump("Getting document\n");
  var theDoc = GetCurrentEditor().document;
  if (theDoc)
  {
    dump("Got the doc\n");
    dump("Document name:" + theDoc.nodeName + "\n");
    dump("Document type:" + theDoc.doctype + "\n");
  }
  else
  {
    dump("Failed to get the doc\n");
  }
}

// --------------------------- Logging stuff ---------------------------

function EditorExecuteScript(theFile)
{
  var inputStream = Cc["@mozilla.org/network/file-input-stream;1"].createInstance();
  inputStream = inputStream.QueryInterface(Ci.nsIFileInputStream);

  inputStream.init(theFile, 1, 0, false);    // open read only

  var scriptableInputStream = Cc["@mozilla.org/scriptableinputstream;1"].createInstance();
  scriptableInputStream = scriptableInputStream.QueryInterface(Ci.nsIScriptableInputStream);

  scriptableInputStream.init(inputStream);    // open read only

  var buf         = { value:null };
  var tmpBuf      = { value:null };
  var didTruncate = { value:false };
  var lineNum     = 0;
  var ex;

/*
  // Log files can be quite huge, so read in a line
  // at a time and execute it:

  while (!inputStream.eof())
  {
    buf.value         = "";
    didTruncate.value = true;

    // Keep looping until we get a complete line of
    // text, or we hit the end of file:

    while (didTruncate.value && !inputStream.eof())
    {
      didTruncate.value = false;
      fileSpec.readLine(tmpBuf, 1024, didTruncate);
      buf.value += tmpBuf.value;

      // XXX Need to null out tmpBuf.value to avoid crashing
      // XXX in some JavaScript string allocation method.
      // XXX This is probably leaking the buffer allocated
      // XXX by the readLine() implementation.

      tmpBuf.value = null;
    }

    ++lineNum;
*/
  {
    // suck in the entire file
    var fileSize = scriptableInputStream.available();
    var fileContents = scriptableInputStream.read(fileSize);

    dump(fileContents);

    try       { eval(fileContents); }
    catch(ex) { dump("Playback ERROR: Line " + lineNum + "  " + ex + "\n"); return; }
  }

  buf.value = null;
}

function EditorGetScriptFileSpec()
{
  var processDir = GetSpecialDirectory("Home");
  processDir.append("journal.js");
  return processDir;
}

function EditorStartLog()
{
  try {
    var edlog = GetCurrentEditor().QueryInterface(Ci.nsIEditorLogging);
    var fs = EditorGetScriptFileSpec();
    edlog.startLogging(fs);
    window.content.focus();

    fs = null;
  }
  catch(ex) { dump("Can't start logging!:\n" + ex + "\n"); }
}

function EditorStopLog()
{
  try {
    var edlog = GetCurrentEditor().QueryInterface(Ci.nsIEditorLogging);
    edlog.stopLogging();
    window.content.focus();
  }
  catch(ex) { dump("Can't stop logging!:\n" + ex + "\n"); }
}

function EditorRunLog()
{
  var fs;
  fs = EditorGetScriptFileSpec();
  EditorExecuteScript(fs);
  window.content.focus();
}

// --------------------------- TransactionManager ---------------------------


function DumpUndoStack()
{
  try {
    var txmgr = GetCurrentEditor().transactionManager;

    if (!txmgr)
    {
      dump("**** Editor has no TransactionManager!\n");
      return;
    }

    dump("---------------------- BEGIN UNDO STACK DUMP\n");
    dump("<!-- Bottom of Stack -->\n");
    PrintTxnList(txmgr.getUndoList(), "");
    dump("<!--  Top of Stack  -->\n");
    dump("Num Undo Items: " + txmgr.numberOfUndoItems + "\n");
    dump("---------------------- END   UNDO STACK DUMP\n");
  } catch (e) {
    dump("ERROR: DumpUndoStack() failed: " + e);
  }
}

function DumpRedoStack()
{
  try {
    var txmgr = GetCurrentEditor().transactionManager;

    if (!txmgr)
    {
      dump("**** Editor has no TransactionManager!\n");
      return;
    }

    dump("---------------------- BEGIN REDO STACK DUMP\n");
    dump("<!-- Bottom of Stack -->\n");
    PrintTxnList(txmgr.getRedoList(), "");
    dump("<!--  Top of Stack  -->\n");
    dump("Num Redo Items: " + txmgr.numberOfRedoItems + "\n");
    dump("---------------------- END   REDO STACK DUMP\n");
  } catch (e) {
    dump("ERROR: DumpUndoStack() failed: " + e);
  }
}

function PrintTxnList(txnList, prefixStr)
{
  var i;

  for (i=0 ; i < txnList.numItems; i++)
  {
    var txn = txnList.getItem(i);
    var desc = "TXMgr Batch";

    if (txn)
    {
      try {
        txn = txn.QueryInterface(Ci.nsPIEditorTransaction);
        desc = txn.txnDescription;
      } catch(e) {
        desc = "UnknownTxnType";
      }
    }
    dump(prefixStr + "+ " + desc + "\n");
    PrintTxnList(txnList.getChildListForItem(i), prefixStr + "|    ");
  }
}

// ------------------------ 3rd Party Transaction Test ------------------------


function sampleJSTransaction()
{
  this.wrappedJSObject = this;
}

sampleJSTransaction.prototype = {

  isTransient: false,
  mStrData:    "[Sample-JS-Transaction-Content]",
  mObject:     null,
  mContainer:  null,
  mOffset:     null,

  doTransaction: function()
  {
    if (this.mContainer.nodeType != Node.TEXT_NODE)
    {
      // We're not in a text node, so create one and
      // we'll just insert it at (mContainer, mOffset).

      this.mObject = this.mContainer.ownerDocument.createTextNode(this.mStrData);
    }

    this.redoTransaction();
  },

  undoTransaction: function()
  {
    if (!this.mObject)
      this.mContainer.deleteData(this.mOffset, this.mStrData.length);
    else
      this.mObject.remove();
  },

  redoTransaction: function()
  {
    if (!this.mObject)
      this.mContainer.insertData(this.mOffset, this.mStrData);
    else
      this.insert_node_at_point(this.mObject, this.mContainer, this.mOffset);
  },

  merge: function(aTxn)
  {
    // We don't do any merging!

    return false;
  },

  QueryInterface: function(aIID, theResult)
  {
    if (aIID.equals(Ci.nsITransaction) ||
        aIID.equals(Ci.nsISupports))
      return this;

    throw Cr.NS_ERROR_NO_INTERFACE;
  },

  insert_node_at_point: function(node, container, offset)
  {
    var childList = container.childNodes;

    if (childList.length == 0 || offset >= childList.length)
      container.appendChild(node);
    else
      container.insertBefore(node, childList.item(offset));
  }
}

function ExecuteJSTransactionViaTxmgr()
{
  try {
    var editor = GetCurrentEditor();
    var txmgr = editor.transactionManager;
    txmgr = txmgr.QueryInterface(Ci.nsITransactionManager);

    var selection = editor.selection;
    var range =  selection.getRangeAt(0);

    var txn = new sampleJSTransaction();

    txn.mContainer = range.startContainer;
    txn.mOffset = range.startOffset;

    txmgr.doTransaction(txn);
  } catch (e) {
    dump("ExecuteJSTransactionViaTxmgr() failed!");
  }
}

function ExecuteJSTransactionViaEditor()
{
  try {
    var editor = GetCurrentEditor();

    var selection = editor.selection;
    var range =  selection.getRangeAt(0);

    var txn = new sampleJSTransaction();

    txn.mContainer = range.startContainer;
    txn.mOffset = range.startOffset;

    editor.doTransaction(txn);
  } catch (e) {
    dump("ExecuteJSTransactionViaEditor() failed!");
  }
}

function EditorNewPlaintext(aUrl, aCharsetArg)
{
  window.openDialog( "chrome://debugqa/content/debugQATextEditorShell.xul",
                     "_blank",
                     "chrome,dialog=no,all",
                     aUrl || "about:blank",
                     aCharsetArg);
}
