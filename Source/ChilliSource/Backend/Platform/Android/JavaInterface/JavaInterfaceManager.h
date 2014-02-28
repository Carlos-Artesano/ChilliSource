/*
 *  JavaInterfaceManager.h
 *  moFlow
 *
 *  Created by Ian Copland on 08/08/2012.
 *  Copyright 2012 Tag Games. All rights reserved.
 *
 */

#ifndef _MOFLOW_PLATFORM_ANDROID_JAVAINTERFACE_JAVAINTERFACEMANAGER_H_
#define _MOFLOW_PLATFORM_ANDROID_JAVAINTERFACE_JAVAINTERFACEMANAGER_H_

#include <ChilliSource/Backend/Platform/Android/ForwardDeclarations.h>
#include <ChilliSource/Backend/Platform/Android/JavaInterface/JavaInterface.h>
#include <jni.h>

namespace ChilliSource
{
	namespace Android
	{
		//========================================================
		/// Java Interface Manager
		///
		/// Handles all of the java interfaces and provides an
		/// interface to access them.
		//========================================================
		class CJavaInterfaceManager
		{
		public:
			//--------------------------------------------------------
			/// Get Singleton Ptr
			///
			/// @return a pointer to the singleton instance.
			//--------------------------------------------------------
			static CJavaInterfaceManager* GetSingletonPtr();
			//--------------------------------------------------------
			/// Get Singleton
			///
			/// @return a reference to the singleton instance.
			//--------------------------------------------------------
			static CJavaInterfaceManager& GetSingleton();
			//--------------------------------------------------------
			/// Initialise
			///
			/// Initialises the Java Interface Manager
			///
			/// @param The JVM.
			//--------------------------------------------------------
			void Initialise(JavaVM* inpJavaVM);
			//--------------------------------------------------------
			/// Get Java VM
			///
			/// @return The Java VM.
			//--------------------------------------------------------
			JavaVM* GetJavaVM();
			//--------------------------------------------------------
			/// Get JNI Environment Ptr
			///
			///	returns the jni enviroment pointer for the current
			/// thread. The value returned from this should NOT be stored
			/// as global or member data, as this will break thread safety
			///
			/// @return returns a pointer to the jni environment.
			//--------------------------------------------------------
			JNIEnv* GetJNIEnvironmentPtr();
			//--------------------------------------------------------
			/// Add Java Interface
			///
			/// Adds a new Java Interface. This should be called on
			/// creation of the system that uses the java interface.
			///
			/// @param The java interface.
			//--------------------------------------------------------
			void AddJavaInterface(JavaInterfacePtr inpJavaInterface);
			//--------------------------------------------------------
			/// Get Java Interface
			///
			/// Looks for a java interface that implements the given interface.
			///
			/// @param The type ID of the java interface you wish to implement
			/// @return Java Interface that implements the given interface or NULL if none
			//--------------------------------------------------------
			JavaInterfacePtr GetJavaInterface(Core::InterfaceIDType inInterfaceID);
			//--------------------------------------------------------
			/// Get Java Interface
			///
			/// Looks for a java interface that implements the given interface.
			///
			/// @return Java Interface that implements the given interface or NULL if none
			//--------------------------------------------------------
			template <typename T> SHARED_PTR<T> GetJavaInterface();
			//--------------------------------------------------------
			/// Get Java Interface
			///
			/// Looks for a java interface that implements the given interface.
			///
			/// @return Java Interface that implements the given interface or NULL if none
			//--------------------------------------------------------
			template <typename T, typename U> SHARED_PTR<T> GetJavaInterface();
			//--------------------------------------------------------
			/// Get Java Interfaces
			///
			/// Looks for Java Interfaces that implement the given interface
			/// and fills an array with them.
			///
			/// @param The type ID of the Java Interface you wish to implement
			/// @param OUT: The array of java interfaces.
			//--------------------------------------------------------
			void GetJavaInterfaces(Core::InterfaceIDType inInterfaceID, DYNAMIC_ARRAY<JavaInterfacePtr>& outJavaInterfaces);
			//--------------------------------------------------------
			/// Get Java Interfaces
			///
			/// Looks for Java Interfaces that implement the given interface
			/// and fills an array with them.
			///
			/// @param OUT: The array of java interfaces.
			//--------------------------------------------------------
			template <typename T> void GetJavaInterfaces(DYNAMIC_ARRAY<SHARED_PTR<T> >& outJavaInterfaces);
			//--------------------------------------------------------
			/// Destructor
			//--------------------------------------------------------
			~CJavaInterfaceManager();
		private:
			//--------------------------------------------------------
			/// Constructor
			//--------------------------------------------------------
			CJavaInterfaceManager();

			static CJavaInterfaceManager* mpJavaInterfaceManagerSingleton;
			DYNAMIC_ARRAY<JavaInterfacePtr> mJavaInterfaces;
			JavaVM* mpJavaVM;
		};
		//========================================================
		/// Templated Definitions
		//========================================================
		//--------------------------------------------------------
		/// Get Java Interface
		//--------------------------------------------------------
		template <typename T> SHARED_PTR<T> CJavaInterfaceManager::GetJavaInterface()
		{
			JavaInterfacePtr pJavaInterface = GetJavaInterface(T::InterfaceID);

			if (pJavaInterface != NULL)
			{
				return SHARED_PTR_CAST<T>(pJavaInterface);
			}

			return SHARED_PTR<T>();
		}
		//--------------------------------------------------------
		/// Get Java Interface
		//--------------------------------------------------------
		template <typename T, typename U> SHARED_PTR<T> CJavaInterfaceManager::GetJavaInterface()
		{
			JavaInterfacePtr pJavaInterface = GetJavaInterface(U::InterfaceID);

			if (pJavaInterface != NULL)
			{
				return SHARED_PTR_CAST<T>(pJavaInterface);
			}

			return SHARED_PTR<T>();
		}
		//--------------------------------------------------------
		/// Get Java Interfaces
		//--------------------------------------------------------
		template <typename T> void CJavaInterfaceManager::GetJavaInterfaces(DYNAMIC_ARRAY<SHARED_PTR<T> >& outJavaInterfaces)
		{
			for (size_t nJavaInterface = 0; nJavaInterface < mJavaInterfaces.size(); nJavaInterface++)
			{
				if (mJavaInterfaces[nJavaInterface]->IsA(T::InterfaceID))
				{
					outJavaInterfaces.push_back(SHARED_PTR_CAST<T>(mJavaInterfaces[nJavaInterface]));
				}
			}
		}
	}
}

#endif
