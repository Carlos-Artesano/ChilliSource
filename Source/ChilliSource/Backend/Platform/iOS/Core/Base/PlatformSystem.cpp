//
//  PlatformSystem.cpp
//  Chilli Source
//
//  Created by Scott Downie on 24/11/2010.
//  Copyright (c) 2014 Tag Games Ltd. All rights reserved.
//
#include <ChilliSource/Backend/Platform/iOS/Core/Base/PlatformSystem.h>

#include <ChilliSource/Backend/Platform/iOS/Core/String/NSStringUtils.h>
#include <ChilliSource/Backend/Platform/iOS/Core/String/NSString+MD5Addition.h>
#include <ChilliSource/Backend/Platform/iOS/Core/Base/UIDevice+IdentifierAddition.h>
#include <ChilliSource/Backend/Platform/iOS/Core/Base/NativeSystem.h>
#include <ChilliSource/Backend/Rendering/OpenGL/Shader/GLSLShaderProvider.h>
#include <ChilliSource/Backend/Rendering/OpenGL/Texture/TextureUnitSystem.h>
#include <ChilliSource/Core/String/StringUtils.h>

#include <UIKit/UIKit.h>

namespace ChilliSource 
{
	namespace iOS
	{   
		//-----------------------------------------
		//-----------------------------------------
		PlatformSystem::PlatformSystem()
        : m_physicalScreenSize(-1.0f)
		{
		}
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
            in_application->CreateSystem<OpenGL::TextureUnitSystem>();
		}
		//-------------------------------------------------
		//-------------------------------------------------
		void PlatformSystem::PostCreateSystems()
		{

		}
        //-----------------------------------------
        //-----------------------------------------
        void PlatformSystem::Run()
        {
            iOSInit();
        }
        //-----------------------------------------
        //-----------------------------------------
        void PlatformSystem::SetMaxFPS(u32 in_fps)
        {
            iOSSetMaxFPS(in_fps);
        }
		//-----------------------------------------
		//-----------------------------------------
		void PlatformSystem::SetUpdaterActive(bool in_isActive)
		{
			iOSSetUpdaterActive(in_isActive);
		}
        //-----------------------------------------
        //-----------------------------------------
        void PlatformSystem::TerminateUpdater() 
        {
            iOSInvalidateUpdater();
        }
		//------------------------------------------
		//------------------------------------------
		Core::Vector2 PlatformSystem::GetScreenDimensions() const
		{
			Core::Vector2 result;
			CGSize size = [[UIScreen mainScreen] bounds].size;
            
            f32 scale = 1.0f;
            
            bool isiOS4_0 = ([[[UIDevice currentDevice] systemVersion] floatValue] >= 4.0f);
            
            if(isiOS4_0)
            {
                scale = [UIScreen mainScreen].scale;
            }
            
			result.x = size.width * scale;
			result.y = size.height * scale;
            
			return result;
		}

        //----------------------------------------------
        //----------------------------------------------
        f32 PlatformSystem::GetScreenDensity() const
        {
            if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 4.0f) 
            {
				CGFloat scale = [UIScreen mainScreen].scale;
                
                return scale;
			}
            
            return 1.0f;
        }
        //-------------------------------------------------
        //-------------------------------------------------
        std::string PlatformSystem::GetAppVersion() const
        {
            NSString* version = [[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString *)kCFBundleVersionKey];
            return [NSStringUtils newStringWithNSString:version];
        }
		//---------------------------------------------------
		//---------------------------------------------------
		TimeIntervalMs PlatformSystem::GetSystemTimeMS() const
		{
			return GetSystemTimeInNanoSeconds() / 1000000;
		}
	}
}
