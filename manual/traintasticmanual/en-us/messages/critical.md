# Critical {#messages-critical}

Critical messages indicate that action is required.


## C1001: Loading world failed (*reason*) {#c1001}

TODO


## C1002: Creating client failed (*reason*) {#c1002}

TODO


## C1003: Can't write to settings file (*reason*) {#c1003}

TODO


## C1004: Reading world failed (*reason*) (*filename*) {#c1004}

TODO


## C1005: Saving world failed (*reason*) {#c1005}

TODO


## C1006: Creating world backup failed (*reason*) {#c1006}

TODO


## C1007: Creating world backup directory failed (*reason*) {#c1007}

TODO

## C1008: Creating backup directory failed (*reason*) {#c1008}
TODO

## C1009: Creating settings backup failed (*reason*) {#c1009}
TODO

## C1010: Exporting world failed (*reason*) {#c1010}
TODO

## C1011: Importing world failed (*reason*) {#c1011}
TODO

## C1012: Unknown class '*class id*', can't recreate object '*object id*' {#c1012}
TODO

## C1013: Can't load world saved with newer version, requires at least: Traintastic server *version* {#c1013}

**Cause:** The world is saved with a newer version of Traintastic server than currently is running.

**Solution:** Update Traintastic server (and client) to at least *version*.


## C2001: Address already used at #*object* {#c2001}

TODO


## C2002: DCC++ only supports the DCC protocol {#c2002}

**Cause:** The selected decoder protocol isn't supported by the DCC++ command station.

**Solution:** Change the decoder protocol to *DCC* or *Auto*.


## C2003: DCC++ doesn't support DCC long addresses below 128 {#c2003}

**Cause:** The DCC++ command station considers all addresses below 128 a DCC short address.

**Solution:** Change the locomotive decoder address.


## C2004: Can't get free slot {#c2004}
TODO

## C9999: *message* {#c9999}

Custom critical message generated by a [Lua script](../lua.md).
