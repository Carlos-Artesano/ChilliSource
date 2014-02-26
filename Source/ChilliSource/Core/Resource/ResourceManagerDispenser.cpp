//
//  ResourceManagerDispenser.cpp
//  moFloTest
//
//  Created by Scott Downie on 02/05/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#include <ChilliSource/Core/Resource/ResourceManagerDispenser.h>
#include <ChilliSource/Core/Resource/ResourceGroupManager.h>
#include <ChilliSource/Core/Resource/ResourceManager.h>
#include <ChilliSource/Core/Resource/ResourceProvider.h>

#include <ChilliSource/Rendering/Texture/Texture.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheet.h>

#include <ChilliSource/Rendering/Font/Font.h>

namespace ChilliSource
{
    namespace Core
    {
        CResourceManagerDispenser* CResourceManagerDispenser::pInstance = nullptr;
        //----------------------------------------------------
        /// Constructor
        ///
        /// Singleton instance
        //----------------------------------------------------
        CResourceManagerDispenser::CResourceManagerDispenser(CApplication* inpApp) : mpApp(inpApp)
        {
            pInstance = this;
			mpResourceGroupMgr = new IResourceGroupManager(pInstance);
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Singleton Ptr
        ///
        /// @return Singleton instance
        //--------------------------------------------------------------------------------------------------
        CResourceManagerDispenser* CResourceManagerDispenser::GetSingletonPtr()
        {
            return pInstance;
        }
		//--------------------------------------------------------------------------------------------------
		/// @return Singleton instance
		//--------------------------------------------------------------------------------------------------
		CResourceManagerDispenser& CResourceManagerDispenser::GetSingleton()
        {
			return *pInstance;
		}
        //--------------------------------------------------------------------------------------------------
		/// Register Resource Manager
		///
		/// Adds a resource manager to the application pool. This allows systems to load resources
		/// that they themselves cannot provide
		/// @param Handle to resource manager
		//--------------------------------------------------------------------------------------------------
		void CResourceManagerDispenser::RegisterResourceManager(IResourceManager * inpResourceManager)
		{
			inpResourceManager->SetApplicationOwner(mpApp);
			mResourceManagers.push_back(inpResourceManager);
		}
        //--------------------------------------------------------------------------------------------------
        /// Get Resource Manager With Interface
        ///
        /// Looks for a manager with the given interface
        /// @param The type ID of the manager interface you are seeking
        /// @return Manager that implements the given interface or nullptr if none available
        //--------------------------------------------------------------------------------------------------
        IResourceManager* CResourceManagerDispenser::GetResourceManagerWithInterface(InterfaceIDType inInterfaceID)
        {
            for (DYNAMIC_ARRAY<IResourceManager*>::iterator pMgr = mResourceManagers.begin(); pMgr != mResourceManagers.end(); ++pMgr) 
			{
				if ((*pMgr)->IsA(inInterfaceID)) 
				{
					return (*pMgr);
				}
			}
            
            WARNING_LOG("Application cannot find resource manager for type");
            return nullptr;
        }
		//--------------------------------------------------------------------------------------------------
		/// Get Resource Manager For Type
		///
		/// Looks through the applications pool of resource managers til it finds one that can create
		/// a resource of given type
		/// @param Resource type ID that you wish to load
		//--------------------------------------------------------------------------------------------------
		IResourceManager* CResourceManagerDispenser::GetResourceManagerForType(InterfaceIDType inResourceInterfaceID)
		{
			for (DYNAMIC_ARRAY<IResourceManager*>::iterator pMgr = mResourceManagers.begin(); pMgr != mResourceManagers.end(); ++pMgr) 
			{
				if ((*pMgr)->ManagesResourceOfType(inResourceInterfaceID)) 
				{
					return (*pMgr);
				}
			}
			
			WARNING_LOG("Application cannot find resource manager for type");
			return nullptr;
		}
		//--------------------------------------------------------------------------------------------------
		/// Get Resource Manager For Extension
		///
		/// Looks through the applications pool of resource managers til it finds one that can create
		/// a resource with the given extension
		/// @param Resource extension string
		//--------------------------------------------------------------------------------------------------
		IResourceManager* CResourceManagerDispenser::GetResourceManagerForExtension(const std::string &instrExtension)
		{
			for (DYNAMIC_ARRAY<IResourceManager*>::iterator pMgr = mResourceManagers.begin(); pMgr != mResourceManagers.end(); ++pMgr) 
			{
				if((*pMgr)->ManagesResourceWithExtension(instrExtension)) 
				{
					return (*pMgr);
				}
			}
			
			WARNING_LOG("Application cannot find resource manager for extension " + instrExtension);
			return nullptr;
		}
        //----------------------------------------------------------------------
		/// Free Resource Caches
		///
		/// Release the cached resources that are not in use
		//----------------------------------------------------------------------
		void CResourceManagerDispenser::FreeResourceCaches()
		{
			//Tell our resource managers to ditch their cache
            u32 udwReleasedCount = 0;
			do
			{
                udwReleasedCount = 0;
                for (DYNAMIC_ARRAY<IResourceManager*>::iterator pMgr = mResourceManagers.begin(); pMgr != mResourceManagers.end(); ++pMgr)
                {
                    udwReleasedCount += (*pMgr)->ReleaseAllUnused();
                }
			}
			while (udwReleasedCount > 0);
			//Free our resource groups too. Actually all the resources will be freed by this stage
			//Just let the group manager know this
			mpResourceGroupMgr->InvalidateCache();
		}
        //--------------------------------------------------------------------------------------------------
        /// Get Resource Group Manager
        ///
        /// @return A reference to the resource group manager
        //--------------------------------------------------------------------------------------------------
        IResourceGroupManager& CResourceManagerDispenser::GetResourceGroupManager()
        {
            return *mpResourceGroupMgr;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Resource Group Manager Pointer
        ///
        /// @return A pointer to the resource group manager
        //--------------------------------------------------------------------------------------------------
        IResourceGroupManager* CResourceManagerDispenser::GetResourceGroupManagerPtr()
        {
            return mpResourceGroupMgr;
        }
        //----------------------------------------------------------------------
        /// Set Resource Providers
        ///
        /// Iterate through the list of resource providers and add them to
        /// any managers of that type. This allows the managers to create
        /// their own resources
        ///
        /// @param Vector of resource provider pointers
        //----------------------------------------------------------------------
        void CResourceManagerDispenser::SetResourceProviders(const DYNAMIC_ARRAY<IResourceProvider*> inProviders)
        {
            //For each resource manager, give it resource providers of the same resource type
			for (DYNAMIC_ARRAY<IResourceManager*>::iterator pMgr = mResourceManagers.begin(); pMgr != mResourceManagers.end(); ++pMgr) 
			{
				InterfaceIDType ResourceType = (*pMgr)->GetProviderType();
				DYNAMIC_ARRAY<IResourceProvider*> UsefulResourceProviders;
				
				for (DYNAMIC_ARRAY<IResourceProvider*>::const_iterator pProv = inProviders.begin(); pProv != inProviders.end(); ++pProv) 
				{
					if ((*pProv)->CanCreateResourceOfKind(ResourceType)) 
					{
						UsefulResourceProviders.push_back((*pProv));
					}
				}
				(*pMgr)->SetResourceProviders(UsefulResourceProviders);
			}
        }
        //--------------------------------------------------------------------
        /// Destructor
        //--------------------------------------------------------------------
        CResourceManagerDispenser::~CResourceManagerDispenser()
        {
            SAFE_DELETE(mpResourceGroupMgr);
        }
    }
}
