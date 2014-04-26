//
//  PlatformSystem.cpp
//  Chilli Source
//
//  Created by Ian Copland on 24/11/2010.
//  Copyright (c) 2014 Tag Games Ltd. All rights reserved.
//

#include <ChilliSource/Backend/Platform/Android/Core/Base/PlatformSystem.h>

#include <ChilliSource/Backend/Platform/Android/Core/Base/CoreJavaInterface.h>
#include <ChilliSource/Backend/Platform/Android/Core/JNI/JavaInterfaceManager.h>
#include <ChilliSource/Backend/Rendering/OpenGL/Shader/GLSLShaderProvider.h>
#include <ChilliSource/Backend/Rendering/OpenGL/Texture/TextureUnitSystem.h>
#include <ChilliSource/Core/Image/ETC1ImageProvider.h>

namespace ChilliSource 
{
	namespace Android
	{
		//-----------------------------------------
		//-----------------------------------------
		void PlatformSystem::Init()
		{

		}
		//-------------------------------------------------
		//-------------------------------------------------
		void PlatformSystem::CreateDefaultSystems(Core::Application* in_application)
		{
			in_application->CreateSystem<OpenGL::GLSLShaderProvider>();
			in_application->CreateSystem<Core::ETC1ImageProvider>();
			in_application->CreateSystem<OpenGL::TextureUnitSystem>();
		}
		//-------------------------------------------------
		//-------------------------------------------------
		void PlatformSystem::PostCreateSystems()
		{

		}
        //-----------------------------------------
        //-----------------------------------------
        void PlatformSystem::SetMaxFPS(u32 in_fps)
        {
        	JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>()->SetMaxFPS(in_fps);
        }
		//-----------------------------------------
        //-----------------------------------------
        void PlatformSystem::TerminateUpdater()
        {
        	JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>()->ForceQuit();
        }
		//---------------------------------------------
		//---------------------------------------------
		Core::Vector2 PlatformSystem::GetScreenDimensions() const
		{
			Core::Vector2 dims;
			CoreJavaInterfaceSPtr coreJI = JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>();
			dims.x = coreJI->GetScreenWidth();
			dims.y = coreJI->GetScreenHeight();

			s32 orientation = coreJI->GetOrientation();
#ifdef CS_ENABLE_DEBUG
			if(orientation < 0)
				CS_LOG_ERROR("PlatformSystem::GetScreenDimensions() - Could not get orientation of device!");
#endif
			if(Core::ScreenOrientation::k_landscapeRight == (Core::ScreenOrientation)orientation)
			{
				// Swap round as we want dimensions the other way
				std::swap(dims.x, dims.y);
			}

			return dims;
		}

		//-------------------------------------------------
		//-------------------------------------------------
		f32 PlatformSystem::GetScreenDensity() const
		{
			return JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>()->GetScreenDensity();
		}
        //-------------------------------------------------
        //-------------------------------------------------
        std::string PlatformSystem::GetAppVersion() const
		{
			return JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>()->GetApplicationVersionName();
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		TimeIntervalMs PlatformSystem::GetSystemTimeMS() const
		{
			return JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<CoreJavaInterface>()->GetSystemTimeInMilliseconds();
		}
	}
}
