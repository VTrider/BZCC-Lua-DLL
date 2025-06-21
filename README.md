# BZCC-Lua-DLL

Ever wanted to make a native plugin for your lua script? Well here is the solution :)

Included in this repo is an example C++ project to build a lua library, as well as an import library for LuaMission.dll so it can link to the same runtime the game is using, no heap corruption, no bullshit. I've also included the .def file for the LuaMission exports if you'd like to make your own .lib. In order for the game to load your library it requires an absolute path to the .dll file (as far as I know), I have an example dll loader lua script that will repair the stock require() function using the workshop id of your item and the folder paths where the .dll is located. You'll need to do an initial upload of your item in order to get a workshop id. When testing locally you can just copy/paste an absolute path from file explorer.

```lua
-- This is how you'd use my script to load the module for example
local dll_loader = require("dll_loader")

-- If you are testing locally you can do:
-- package.cpath = package.cpath .. ";C:\\Path\\To\\Your\\DLL\\?.dll" -- note the last directory is "?.dll" to instruct lua to search for dll modules 

-- The first argument is the workshop id of your item, the next is a vararg for the folder paths to your .dll,
-- you can leave it blank if the .dll is in the root directory of your workshop item.
-- In this example the structure of your workshop item would be 12345\Scripts\Bin\library.dll
dll_loader.find("12345", "Scripts", "Bin")

-- Now you are safe to call require() to load the library and use its functions
local library = require("library")

print(libary.Hello())
```

You can also support user-defined callbacks in Lua:
```lua
local library = require("library")

-- The library calls back into this function if it's defined in script, and can pass values back into lua
function library.MyCallback(param)
  if param == 5 then
    return 1 -- Users can return a value that can be handled by the library
  end
end
```

