#include <ntstatus.h>
#include <ranges>
#include <print>
#include <Windows.h>

#include "Components/Class Manager/ClassManager.h"
#include "Components/Config/Config.h"
#include "Components/FontManager/FontManager.h"
#include "Components/Layout/Dissector/Grid/Grid.h"
#include "Components/Notifications/Notifications.h"
#include "Components/Renderer/Renderer.h"
#include "Components/Theme Manager/Theme Manager.h"
#include "Definitions/Globals.h"
#include "Libraries/Aether/Aether.h"

#include "Definitions/NtDefs.h"

int main()
{
	Config::Initialize();
	ClassManager::Initialize();

	if (!Renderer::Window::Initialize()) return 1;

	Themes::Manager::Initialize();
	Renderer::FontManager::Initialize();

	Renderer::Window::ApplyWindowData();

	const uint32_t SelfPID = GetProcessId( reinterpret_cast<HANDLE>( -1 ) );
	const auto     Status  = Process::Attach( SelfPID );
	Renderer::Layout::Grid::OnAttachProcess( SelfPID, Status );

	Renderer::Window::Execute();
}
