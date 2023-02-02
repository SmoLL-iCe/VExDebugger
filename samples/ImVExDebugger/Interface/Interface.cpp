#define STB_IMAGE_IMPLEMENTATION
#include "../header.h"
#include "Interface.h"
#include "CreateWindow.h"
#include "MainWin.h"

void __stdcall FrameControls( GLWindow* Instance, bool* visible );

void Gui::Init( )
{
    auto Window = new GLWindow( "VExDebugger", 10, 10 );

    Window->SetFrameControls( reinterpret_cast<void*>( FrameControls ) );

    Window->Center( );
}

int frame = -1;
void Gui::SetWindowIndex( int index )
{
    frame = index;
}

int Gui::GetWindowIndex( )
{
    return frame;
}

void __stdcall FrameControls( GLWindow* Instance, bool* visible )
{
    switch ( frame )
    {
        case 0:
        {
            Gui::Main( Instance, visible );
            break;
        }
        case 1:
        {
            break;
        }
    default:
        break;
    }
}