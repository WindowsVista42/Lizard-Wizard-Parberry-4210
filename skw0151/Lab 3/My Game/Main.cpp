/// \file Main.cpp 
/// \brief Every program has to have a main.

#include "Game.h"
#include "Window.h"

//#define USE_DEBUG_CONSOLE ///< Define to use a console window for debug messages.

#ifdef _DEBUG
//  #include <vld.h> //Visual Leak Detector from http://vld.codeplex.com/
#endif

static CWindow g_cWindow; ///< The window class.
static CGame g_cGame; ///< The game class.

/// \brief The main entry point for this application.  
///
/// The main entry point for this application. 
/// \param hInstance Handle to the current instance of this application.
/// \param hPrevInstance Unused.
/// \param lpCmdLine Unused.
/// \param nCmdShow Nonzero if window is to be shown.
/// \return 0 If this application terminates correctly, otherwise an error code.

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
  _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);
  
  #ifdef USE_DEBUG_CONSOLE
    const bool console = true;
  #else
    const bool console = false;
  #endif //USE_DEBUG_CONSOLE

  auto init    = [&](){g_cGame.Initialize();};
  auto process = [&](){g_cGame.ProcessFrame();};
  auto release = [&](){g_cGame.Release();};

  return g_cWindow.WinMain(hInstance, console, init, process, release);
} //wWinMain