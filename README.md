# Reflection

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/ReflectionThumb.png" width="1000" />
</figure>

> #### ⚠️ PRE-ALPHA 
> Reflection is still in pre-alpha and still misses a lot of features, please keep this in mind before diving in! :3  
> Some features are still a big WIP but I'm posting this as a bit of a preview, or toy if you wish to play with it!

Reflection aims to replace the old and outdated [ReClass](https://github.com/CoolOppo/ReClass) and its variants.  

Reflection leverages [Wraith](https://github.com/blnchdev/Wraith) to hijack HANDLEs to target processes and uses undocumented routines through [Aether](https://github.com/blnchdev/Aether)

### Why Reflection over ReClass?

ReClass is ancient, back when most processes were 32-bit, and when UI and UX was... lackluster at best. ReClass also blindly opens processes, which could trigger anti-debugging on a target process, whereas Reflection hijacks handles!

Reflection also allows for the saving (and therefore sharing) of user-defined classes, an example is attached with the program:

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/ExampleStruct.png" width="600" />
</figure>

Which, when accessed through the class viewer, results in this:

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/ExampleStruct2.png" width="800" />
</figure>

This enables users to work together on reverse engineering!

### Showcase

#### Address Formatting
Just like ReClass, Reflection allows you to index an address in this fashion:

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/VirtualAddressShowcase.png" width="600" />
</figure>

With those keywords:

| Keyword             | Result                                                 |
|---------------------|--------------------------------------------------------|
| BaseAddress         | Attached Process' Base Address                         |
| PEB                 | Attached Process' PEB Address                          |
| HeapBase            | Attached Process' Heap Base Address                    |
| this                | Container class                                        |
| SymbolName.dll/.exe | Indexes to the DLL/EXE base address                    |
| [...]                  | Reads the result of the expression inside the brackets |

##### Examples
`[PEB+0x10]` resolves to the process base address, as said offset is defined as `ImageBaseAddress`  
`[[PEB+0x18]+0x10]` resolves to _PEB->Ldr->InLoadOrderModuleList->Flink as shown below

| PEB Raw View                                     | Formatted                                        |
|--------------------------------------------------|--------------------------------------------------|
| <figure align="center"><img src="https://blanche.dev/img/Reflection/PEB1.png" width="400" /></figure> | <figure align="center"><img src="https://blanche.dev/img/Reflection/PEB2.png" width="400" /></figure>

Of course, the parser supports both decimal and hexadecimal values. Hexadecimal values need to be prefixed by `0x` to be valid though.  

#### VA Indexing
If possible, Virtual Addresses are indexed via their contained module, e.g.:  

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/Module.png" width="600" />
</figure>

Pointers also check for validity, an invalid (or nullptr) pointer will render differently, e.g.:  

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/nullptr.png" width="600" />
</figure>

#### Function Disassembling
Through Zydis, ASM bytecode is disassembled back into human readable instructions as such:  

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/Zydis1.png" width="200" />
</figure>

The original code being:

<figure align="center">
  <img src="https://blanche.dev/img/Reflection/Zydis2.png" width="200" />
</figure>

### Credits

Reflection uses:
- [Dear ImGui](https://github.com/ocornut/imgui) for rendering (DirectX 11), although most objects are non-native and custom made for our use!  
- Both [nlohmann's JSON library](https://github.com/nlohmann/json) and [TOML++](https://marzer.github.io/tomlplusplus/), the former for saved classes, the latter for app settings
- [xorstr](https://github.com/JustasMasiulis/xorstr) in an effort to make fingerprinting Reflection harder, although this is a WIP
- [Zydis](https://github.com/zyantific/zydis) for disassembling of bytecode
- My libraries [Aether](https://github.com/blnchdev/Aether) and [Wraith](https://github.com/blnchdev/Wraith) for Windows related stuff
