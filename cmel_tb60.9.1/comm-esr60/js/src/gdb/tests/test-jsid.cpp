#include "gdb-tests.h"
#include "jsapi.h"

FRAGMENT(jsid, simple) {
  JS::Rooted<JSString*> string(cx, JS_NewStringCopyZ(cx, "moon"));
  JS::Rooted<JSString*> interned(cx, JS_AtomizeAndPinJSString(cx, string));
  JS::Rooted<jsid> string_id(cx, INTERNED_STRING_TO_JSID(cx, interned));
  jsid int_id = INT_TO_JSID(1729);
  JS::Rooted<jsid> unique_symbol_id(
      cx, SYMBOL_TO_JSID(JS::NewSymbol(cx, interned)));
  JS::Rooted<jsid> registry_symbol_id(
      cx, SYMBOL_TO_JSID(JS::GetSymbolFor(cx, interned)));
  JS::Rooted<jsid> well_known_symbol_id(
      cx, SYMBOL_TO_JSID(JS::GetWellKnownSymbol(cx, JS::SymbolCode::iterator)));
  jsid void_id = JSID_VOID;
  jsid empty_id = JSID_EMPTY;

  breakpoint();

  use(string_id);
  use(int_id);
  use(unique_symbol_id);
  use(registry_symbol_id);
  use(well_known_symbol_id);
  use(void_id);
  use(empty_id);
}

void jsid_handles(JS::Handle<jsid> jsid_handle,
                  JS::MutableHandle<jsid> mutable_jsid_handle) {
  // Prevent the linker from unifying this function with others that are
  // equivalent in machine code but not type.
  fprintf(stderr, "Called " __FILE__ ":jsid_handles\n");
  breakpoint();
}

FRAGMENT(jsid, handles) {
  JS::Rooted<JSString*> string(cx, JS_NewStringCopyZ(cx, "shovel"));
  JS::Rooted<JSString*> interned(cx, JS_AtomizeAndPinJSString(cx, string));
  JS::Rooted<jsid> string_id(cx, INTERNED_STRING_TO_JSID(cx, interned));
  jsid_handles(string_id, &string_id);
}
