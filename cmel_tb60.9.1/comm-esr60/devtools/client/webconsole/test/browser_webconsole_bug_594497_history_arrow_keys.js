/* -*- indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* vim: set ft=javascript ts=2 et sw=2 tw=80: */
/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

"use strict";

var jsterm, inputNode, values;

var TEST_URI = "data:text/html;charset=utf-8,Web Console test for " +
               "bug 594497 and bug 619598";

add_task(function* () {
  yield loadTab(TEST_URI);

  let hud = yield openConsole();

  setup(hud);
  performTests();

  jsterm = inputNode = values = null;
});

function setup(HUD) {
  jsterm = HUD.jsterm;
  inputNode = jsterm.inputNode;

  jsterm.focus();

  ok(!jsterm.getInputValue(), "jsterm.getInputValue() is empty");

  values = ["document", "window", "document.body"];
  values.push(values.join(";\n"), "document.location");

  // Execute each of the values;
  for (let i = 0; i < values.length; i++) {
    jsterm.setInputValue(values[i]);
    jsterm.execute();
  }
}

function performTests() {
  EventUtils.synthesizeKey("KEY_ArrowUp");


  is(jsterm.getInputValue(), values[4],
     "KEY_ArrowUp: jsterm.getInputValue() #4 is correct");

  ok(inputNode.selectionStart == values[4].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowUp: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart == values[3].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  inputNode.setSelectionRange(values[3].length - 2, values[3].length - 2);

  EventUtils.synthesizeKey("KEY_ArrowUp");
  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowUp two times: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart == jsterm.getInputValue().indexOf("\n") &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowUp again: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart == 0 &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[2],
     "KEY_ArrowUp: jsterm.getInputValue() #2 is correct");

  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[1],
     "KEY_ArrowUp: jsterm.getInputValue() #1 is correct");

  EventUtils.synthesizeKey("KEY_ArrowUp");

  is(jsterm.getInputValue(), values[0],
     "KEY_ArrowUp: jsterm.getInputValue() #0 is correct");

  ok(inputNode.selectionStart == values[0].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[1],
     "KEY_ArrowDown: jsterm.getInputValue() #1 is correct");

  ok(inputNode.selectionStart == values[1].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[2],
     "KEY_ArrowDown: jsterm.getInputValue() #2 is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowDown: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart == values[3].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  inputNode.setSelectionRange(2, 2);

  EventUtils.synthesizeKey("KEY_ArrowDown");
  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowDown two times: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart > jsterm.getInputValue().lastIndexOf("\n") &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[3],
     "KEY_ArrowDown again: jsterm.getInputValue() #3 is correct");

  ok(inputNode.selectionStart == values[3].length &&
     inputNode.selectionStart == inputNode.selectionEnd,
     "caret location is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  is(jsterm.getInputValue(), values[4],
     "KEY_ArrowDown: jsterm.getInputValue() #4 is correct");

  EventUtils.synthesizeKey("KEY_ArrowDown");

  ok(!jsterm.getInputValue(),
     "KEY_ArrowDown: jsterm.getInputValue() is empty");
}
