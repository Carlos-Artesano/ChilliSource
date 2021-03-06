/**
 * GooglePlayRemoteNotificationsNativeInterface.java
 * Chilli Source
 * Created by Robert Henning on 23/01/2014.
 * 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Tag Games Limited
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

package com.chilliworks.chillisource.core;

public class GooglePlayRemoteNotificationNativeInterface extends com.chilliworks.chillisource.core.System
{
    public static InterfaceId INTERFACE_ID = new InterfaceId();
	
	//---------------------------------------------------------------------
	/// Native On Remote Token Received
	///
	/// Used to pass the remote token to the native code.
	///
	/// @param The device token to pass to the server
	//---------------------------------------------------------------------
	public native void NativeOnRemoteTokenReceived(final String inToken);
	//---------------------------------------------------------------------
	/// Native On Remote Notification Received
	///
	/// Used to pass the remote notification to the native code.
	///
	/// @param Keys for notification data
	/// @param Values for notification data
	//---------------------------------------------------------------------
	public native void NativeOnRemoteNotificationReceived(final String[] inastrKeys, final String[] inastrValues);
	
	//---------------------------------------------------------------------
	/// Constructor
	//---------------------------------------------------------------------
	public GooglePlayRemoteNotificationNativeInterface()
	{
        init();
	}
    /**
     * Allows querying of whether or not the system implements the interface described by the
     * given interface id.
     *
     * @author Ian Copland
     *
     * @param in_interfaceId - The interface id to check
     *
     * @return Whether the system implements the given interface
     */
    @Override public boolean isA(InterfaceId in_interfaceId)
    {
        return (in_interfaceId == INTERFACE_ID);
    }
	//---------------------------------------------------------------------
	/// Request Remote Token
	///
	/// Registers a remote notification token which is returned to the
	/// application ready to be passed to the server.
	//---------------------------------------------------------------------
	public void RequestRemoteToken()
	{
	    String strProjectID = GooglePlayLicensing.getLvlPublicKey();
		
		// Make sure the device has the proper dependencies.
		GCMService.SetSenderID(strProjectID);
        GCMRegistrar.checkDevice(CSApplication.get().getActivityContext());
        GCMRegistrar.checkManifest(CSApplication.get().getActivityContext());
        GCMRegistrar.register(CSApplication.get().getActivityContext(), strProjectID);
	}
	//---------------------------------------------------------------------
	/// On Remote Token Received
	///
	/// Passes the remote notification token for this device back to the
	/// application.
	///
	/// @param The device token to pass to the server
	//---------------------------------------------------------------------
	public void OnRemoteTokenReceived(final String instrToken)
	{
		NativeOnRemoteTokenReceived(instrToken);
	}
	//---------------------------------------------------------------------
	/// On Remote Notification Received
	///
	/// Passes the message and payload to the app.
	///
	/// @param Keys for notification data
	/// @param Values for notification data
	//---------------------------------------------------------------------	
	public void OnRemoteNotificationReceived(final String[] inastrKeys, final String[] inastrValues)
	{
		NativeOnRemoteNotificationReceived(inastrKeys, inastrValues);
	}
}
