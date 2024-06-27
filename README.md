This is a internal mod for Deutsche Arma Allianz.

Though the Extension inside here is universal and can be used for Web requests.

To download just the Extension, check the [releases](https://github.com/dedmen/DAA_Mod/releases) tab.


The extension returns results via ExtensionCallback.
So if you want to retrieve results, you need to add a Callback eventhandler

Place this script somewhere where it runs once.

```sqf
addMissionEventHandler ["ExtensionCallback", {
  params ["_name", "_handle", "_data"];

  if (_name != "daa") exitWith {};

  // _handle contains the handle you passed when you sent the request
  // This is an example handler
  if (_handle == "JsonTest") exitWith {
    diag_log _data;
  };

}];
"daa" callExtension "callbackReady";
```

There are currently 4 types of requests imlemented

- HTTP GET
```sqf
// "daa" callExtension ["get", [Handle, URL]];
"daa" callExtension ["get", ["JsonTest", "http://headers.jsontest.com"]];
```
- HTTP POST
```sqf
// "daa" callExtension ["post", [Handle, URL, PostData(, Headers)]];
"daa" callExtension ["post", ["JsonTest", "https://webhook.site/1234", '{"JsonData": true}', "content-type:application/json"]];
"daa" callExtension ["post", ["JsonTest", "https://webhook.site/1234", '{"JsonData": false}', "content-type:application/json;SomeCustomHeader:SomeValue"]];
```
- HTTP PUT
```sqf
// "daa" callExtension ["put", [Handle, URL, PostData(, Headers)]];
"daa" callExtension ["put", ["JsonTest", "https://webhook.site/1234", '{"JsonData": true}', "content-type:application/json"]];
"daa" callExtension ["put", ["JsonTest", "https://webhook.site/1234", '{"JsonData": false}', "content-type:application/json;SomeCustomHeader:SomeValue"]];
```
- DNS TXT record lookup
```sqf
// "daa" callExtension ["dns", [Handle, Domain]];
"daa" callExtension ["dns", ["JsonTest", "1234.dnshook.site"]];
```

Plus some utility commands

- Add global HTTP header to every future request
```sqf
// "daa" callExtension ["addGlobalHeader", [Headers]];
"daa" callExtension ["addGlobalHeader", ["SomeCustomHeader:SomeValue;SomeOtherHeader:SomeOtherValue"]];
```

- Remove ALL global HTTP headers
```sqf
// "daa" callExtension ["clearGlobalHeaders", []];
"daa" callExtension ["clearGlobalHeaders", []];
```

