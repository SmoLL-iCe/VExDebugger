# VExDebugger

A debugger library using VEH.

[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)

## What is it ?

This is a simple debugger library for monitoring data access and writing using hardware breakpoints which manages debugging using the Windows Exception Handling system (inspired by VEH of the Cheat Engine).

## How to use this?

You can compile your own project using the Debugger library or you can use one of the ready-made examples using the lib.
In the examples you will find ways to debug your own executable or create a dll to attach to another process and debug it. (use an injector for this.)
And add the addresses you want to know who write, access or execute.

## An example of what can be done with this library.

### This example uses [ImGui](https://github.com/ocornut/imgui)

<h3 align="center">
  <img src="README/vex_debug_gui.png" alt="screeen" />
</h3>

## Example of use:

Hook using VExDebugger
```cpp
  // Init the debugger
  VExDebugger::Init( 
    HandlerType::VectoredExceptionHandler, // Handle the exception type
    true                                   // Enable save text logs
  );

  VExDebugger::SetTracerAddress(
      pTargetAddress,         // Target function address
      BkpMethod::Hardware,    // Hardware breakpoint
      BkpTrigger::Execute,    // When executing
      BkpSize::Size_1,        // Whatever for type execute
      [ pDetour ]( PEXCEPTION_RECORD pExceptRec, PCONTEXT pContext ) -> CBReturn {

          pContext->Rip = reinterpret_cast<uintptr_t>(hookFunction); // function that must be detoured

          return CBReturn::StopTrace; // stop until next run
      }
  );
```

Tracer using VExDebugger
```cpp
  // Init the debugger
  VExDebugger::Init( 
    HandlerType::VectoredExceptionHandler, // Handle the exception type
    true                                   // Enable save text logs
  );

  VExDebugger::SetTracerAddress(
      pTargetAddress,            // Target function address
      BkpMethod::Hardware,       // Hardware breakpoint
      BkpTrigger::ReadWrite,     // When any instruction reads or writes to that address
      BkpSize::Size_1,           // Check in 1 byte space
      [ pDetour ]( PEXCEPTION_RECORD pExceptRec, PCONTEXT pContext ) -> CBReturn  {

          std::cout << "Address: 0x" << std::hex << std::uppercase << pContext->Rip << "\n";

          return CBReturn::StepOver; // Once you reach the trigger, you can continue debugging the next instructions. between the StepOver or StepInto
      }
  );
```

Using monitor address
```cpp
  // Init the debugger
  VExDebugger::Init( 
    HandlerType::VectoredExceptionHandler, // Handle the exception type
    true                                   // Enable save text logs
  );

	VExDebugger::StartMonitorAddress(
      pTargetAddress,               // Target function address
      BkpMethod::PageExceptions,    // Hardware breakpoint
      BkpTrigger::ReadWrite,        // When any instruction reads or writes to that address
      BkpSize::Size_8               // Check in 8 bytes space
    );

  ...

 /*
 To know which instructions fell into triggers.
 You must list using:
 */  
  VExDebugger::CallBreakpointList( []( TBreakpointList BreakpointList ) -> void {

    for ( const auto& [Address, BpInfo] : BreakpointList ) {

      if ( !Address )
        continue;

      VExDebugger::CallAssocExceptionList( [&]( TAssocExceptionList AssocExceptionList ) -> void {

        auto ItExceptionList = AssocExceptionList.find( Address );

        if ( ItExceptionList == AssocExceptionList.end( ) )
          return;
      
        auto& ExceptionList = ItExceptionList->second;

        std::cout << "\nIndex: " << ( BpInfo.Pos + 1 ) << ", Address: 0x" << std::hex << std::uppercase << Address << "\n";

        for ( const auto& [ExceptionAddress, ExceptionInfo] : ExceptionList ) { 

          std::cout << "\tCount " <<
            std::setfill( ' ' ) << std::setw( 8 ) << std::dec << ExceptionInfo.Details.Count << "\n";

          if ( BpInfo.Trigger != BkpTrigger::Execute )
            std::cout << " ExceptionAddress: 0x" << std::hex << std::uppercase << ExceptionAddress << "\n";
          else
            std::cout << " ThreadId: " << std::dec << ExceptionAddress << "\n";          
        }

      } );
    }
  } );

```

### .NET Sample C++/CLI

- **Common Language Runtime Support** - CLR
- **.NET version** - v4.8

<h3 align="center">
  <img src="README/vex_debug_form.png" alt="screeen" />
</h3>

> **Nota:** _It may have problems depending on some variant not yet tested._

## Compatibility

✔ Windows : 64 bits or 32 bits

## Breakpoint methods
- [x] Hardware
- [x] Page Exceptions

## Breakpoint types

- [x] Write
- [x] Read/Write
- [x] Execute

## Download

[Releases](https://github.com/SmoLL-iCe/VExDebugger/releases)

## What was used to compile?

- **Use Visual Studio** - ([Download](https://visualstudio.microsoft.com/pt-br/))
- **Platform tools** - Visual Studio 2022 (v143)
- **C++ SDK version used** - 10.0.22621.0

## Author

[SmoLL-iCe](https://github.com/SmoLL-iCe)

## Thanks

- **VEH inspired:** [cheat-engine](https://github.com/cheat-engine)

## License

[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://raw.githubusercontent.com/SmoLL-iCe/VExDebugger/master/LICENSE)
