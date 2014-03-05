/*
*  HttpConnectionSystemWindows.cpp
*  moFlow
*
*  Created by Scott Downie on 23/05/2011.
*  Copyright 2012 Tag Games. All rights reserved.
*
*/

#include <ChilliSource/Backend/Platform/Windows/Networking/Http/HttpConnectionSystem.h>

#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/MakeDelegate.h>
#include <ChilliSource/Core/Math/MathUtils.h>
#include <ChilliSource/Core/Threading/TaskScheduler.h>

#include <Windows.h>
#include <winhttp.h>
#include <sstream>

#pragma comment(lib, "winhttp")

namespace ChilliSource
{
	namespace Windows
	{
		const u32 kudwBufferSize = 1024 * 50;
		const u32 kudwKeepAliveTimeInSeconds = 120;
		const u32 kudwReadThreadSleepInMS = 100;

		//--------------------------------------------------------------------------------------------------
		/// Constructor
		///
		/// Creates a WinHTTP session
		//--------------------------------------------------------------------------------------------------
		HttpConnectionSystem::HttpConnectionSystem()
		{
			mSessionHandle = ::WinHttpOpen(L"MyWinHttpClient", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
			if(!mSessionHandle)
			{
				CS_LOG_FATAL("Failed to open WinHTTP session");
			}

			//Set the connection timeout time
			const s32 kdwTimeoutMS = (s32)(Networking::kfDefaultHTTPTimeout * 1000.0f);
			::WinHttpSetTimeouts(mSessionHandle, kdwTimeoutMS, kdwTimeoutMS, kdwTimeoutMS, kdwTimeoutMS);
		}
		//--------------------------------------------------------------------------------------------------
		/// Is A
		///
		/// @param Interace ID
		/// @return Whether object if of argument type
		//--------------------------------------------------------------------------------------------------
		bool HttpConnectionSystem::IsA(Core::InterfaceIDType inInterfaceID) const
		{
			return inInterfaceID == Networking::HttpConnectionSystem::InterfaceID || inInterfaceID == IUpdateable::InterfaceID;
		}
		//--------------------------------------------------------------------------------------------------
		/// Make Request 
		///
		/// Causes the system to issue a request with the given details.
		/// @param A HttpRequestDetails struct with valid params per the documentation of HttpRequestDetails
		/// @param (Optional) A function to call when the request is completed. Note that the request can be completed by failure/cancellation as well as success.
		/// @return A pointer to the request. The system owns this pointer. Returns nullptr if the request cannot be created.
		//--------------------------------------------------------------------------------------------------
		Networking::HttpRequest* HttpConnectionSystem::MakeRequest(const Networking::HttpRequestDetails & insRequestDetails, Networking::HttpRequest::CompletionDelegate inOnComplete)
		{
			URL_COMPONENTS sUrlComp;
			DWORD dwUrlLen = 0;

			//Initialize the URL_COMPONENTS structure.
			ZeroMemory(&sUrlComp, sizeof(URL_COMPONENTS));
			sUrlComp.dwStructSize = sizeof(URL_COMPONENTS);

			//Set required component lengths to non-zero so that they are cracked.
			sUrlComp.dwSchemeLength    = (DWORD)-1;
			sUrlComp.dwHostNameLength  = (DWORD)-1;
			sUrlComp.dwUrlPathLength   = (DWORD)-1;
			sUrlComp.dwExtraInfoLength = (DWORD)-1;

			//Change the URL to wide string
			std::wstring strWideURL(insRequestDetails.strURL.length(), L' ');
			std::copy(insRequestDetails.strURL.begin(), insRequestDetails.strURL.end(), strWideURL.begin());

			//Crack the URL.
			if(!WinHttpCrackUrl(strWideURL.c_str(), (DWORD)strWideURL.length(), 0, &sUrlComp))
			{
				CS_LOG_ERROR("Cannot crack URL: " + insRequestDetails.strURL);
				return nullptr;
			}

			//Weirdly the cracked URL struct only gives you the ability to crack your own URL
			std::wstring strWideHostName = sUrlComp.lpszHostName;
			strWideHostName = strWideHostName.substr(0, sUrlComp.dwHostNameLength);
			HINTERNET ConnectionHandle = ::WinHttpConnect(mSessionHandle, strWideHostName.c_str(), INTERNET_DEFAULT_PORT, 0); 

			if(!ConnectionHandle)
			{
				CS_LOG_ERROR("Failed to connect to server: " + insRequestDetails.strURL);
				return nullptr;
			}

			//Set up the request based on whether it is POST or GET and whether it is SSL
			LPCWSTR strTypeVerb = (insRequestDetails.eType == Networking::HttpRequestDetails::Type::k_get ? L"GET" : L"POST");
			DWORD dwFlags = 0;
			HINTERNET RequestHandle = 0;

			std::wstring strWidePath = sUrlComp.lpszUrlPath;
			strWidePath = strWidePath.substr(0, sUrlComp.dwUrlPathLength);

			std::wstring strWideScheme = sUrlComp.lpszScheme;
			strWideScheme = strWideScheme.substr(0, sUrlComp.dwSchemeLength);

			if(strWideScheme == L"https")
			{
				RequestHandle = ::WinHttpOpenRequest(ConnectionHandle, strTypeVerb, strWidePath.c_str(), L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
				if(RequestHandle)
				{
					if(!ApplySSLSettings(RequestHandle))
					{
						WinHttpCloseHandle(ConnectionHandle);
						return nullptr;
					}
				}
			}
			else
			{
				RequestHandle = ::WinHttpOpenRequest(ConnectionHandle, strTypeVerb, strWidePath.c_str(), L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			}

			if(!RequestHandle)
			{
				CS_LOG_ERROR("Failed to open request: " + insRequestDetails.strURL);
				WinHttpCloseHandle(ConnectionHandle);
				return nullptr;
			}

			ConnectionInfo sInfo;
			sInfo.RequestHandle = RequestHandle;
			sInfo.ConnectionHandle = ConnectionHandle;

			//Begin the request on a seperate thread
			CHttpRequest* pHttpRequest = new CHttpRequest(insRequestDetails, sInfo, inOnComplete);
			mapRequests.push_back(pHttpRequest);
			return pHttpRequest;
		}
		//--------------------------------------------------------------------------------------------------
		/// Apply SSL Settings
		///
		/// Setup the request to use SSL level 3
		///
		/// @param Request handle
		/// @return Success
		//--------------------------------------------------------------------------------------------------
		bool HttpConnectionSystem::ApplySSLSettings(HINTERNET inRequestHandle) const
		{
			DWORD dwOptions =	SECURITY_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
								SECURITY_FLAG_IGNORE_UNKNOWN_CA|SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
			if(!WinHttpSetOption(inRequestHandle, WINHTTP_OPTION_SECURITY_FLAGS, &dwOptions, sizeof(DWORD)))
			{
				CS_LOG_ERROR("Failed to set HTTP SSL security flag options");
				return false;
			}

			dwOptions = WINHTTP_FLAG_SECURE_PROTOCOL_SSL3;
			if(!WinHttpSetOption(mSessionHandle, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwOptions, sizeof(DWORD)))
			{
				CS_LOG_ERROR("Failed to set HTTP SSL secure protocol options");
				return false;
			}

			return true;
		}
		//--------------------------------------------------------------------------------------------------
		/// Cancel All Request
		///
		/// Equivalent to calling the above on every incomplete request in progress.
		//--------------------------------------------------------------------------------------------------
		void HttpConnectionSystem::CancelAllRequests()
		{
			for(u32 nRequest = 0; nRequest < mapRequests.size(); nRequest++)
			{
				mapRequests[nRequest]->Cancel();
			}
		}
		//--------------------------------------------------------------------------------------------------
		/// Check Reachability
		///
		/// Checks if the device is internet ready and pings google for a response.
		///
		/// @return Success if URL is reachable
		//--------------------------------------------------------------------------------------------------
		bool HttpConnectionSystem::CheckReachability() const
		{
			return true;
		}
		//--------------------------------------------------------------------------------------------------
		/// Update
		///
		/// For all active requests in the system call update on them allowing them to check the status
		/// of thier requests
		///
		/// @param Time between frames
		//--------------------------------------------------------------------------------------------------
		void HttpConnectionSystem::Update(f32 infDT)
		{
			RequestVector RequestCopy = mapRequests;

			//We should do this in two loops incase anyone tries to insert from the callback hence the copy above
			for(RequestVector::iterator it = RequestCopy.begin(); it != RequestCopy.end(); ++it) 
			{
				(*it)->Update(infDT);
			}

			for(RequestVector::iterator it = mapRequests.begin(); it != mapRequests.end(); )
			{
				if((*it)->HasCompleted())
				{
					//...and remove the completed request
					CS_SAFEDELETE(*it);
					it = mapRequests.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		//--------------------------------------------------------------------------------------------------
		/// Destructor
		///
		/// Destroys the WinHTTP session
		//--------------------------------------------------------------------------------------------------
		HttpConnectionSystem::~HttpConnectionSystem()
		{
			if(mSessionHandle)
			{
				WinHttpCloseHandle(mSessionHandle);
			}
		}
		//=====================================================================================================
		/// Http Request
		///
		/// Constructor
		///
		/// @param Request details
		/// @param Completion delegate
		//=====================================================================================================
		HttpConnectionSystem::CHttpRequest::CHttpRequest(const Networking::HttpRequestDetails & insDetails, const HttpConnectionSystem::ConnectionInfo& insConnectionInfo, const Networking::HttpRequest::CompletionDelegate & inCompletionDelegate)
			: msDetails(insDetails), mbCompleted(false), mCompletionDelegate(inCompletionDelegate), mfActiveTime(0.0f), mbReceivedResponse(false), mbThreadCompleted(false),
			mudwResponseCode(0), mudwBytesRead(0), mbRequestCompleted(false), mConnectionInfo(insConnectionInfo), mudwBytesReadThisBlock(0)
		{
			//Begin the read loop
			//Run this as a threaded task
			Core::TaskScheduler::ScheduleTask(Core::Task<HINTERNET, HINTERNET>(Core::MakeDelegate(this, &HttpConnectionSystem::CHttpRequest::PollReadStream), mConnectionInfo.RequestHandle, mConnectionInfo.ConnectionHandle));
		}
		//------------------------------------------------------------------
		/// Update
		///
		/// Checks the stream to see if any data is available for reading
		/// and reads this into a buffer. Once all the data is read
		/// the request will call the complete delegate
		///
		/// @param Time since last frame
		//------------------------------------------------------------------
		void HttpConnectionSystem::CHttpRequest::Update(f32 infDT)
		{
			//Check if the data has finished streaming and invoke the completion delegate on the main thread
			if(mbCompleted)
			{
				mfActiveTime = 0.0f;
				mbRequestCompleted = true;

				if(mCompletionDelegate)
				{
					if(meRequestResult == HttpRequest::CompletionResult::k_cancelled)
					{
						mCompletionDelegate(this, meRequestResult);
					}
					
					mCompletionDelegate = nullptr;
				}
			}
			//Track the time the request has been active so we can manually timeout
			else if (!mbCompleted && !mbReceivedResponse && ((mfActiveTime += (Core::MathUtils::Min(infDT, 0.5f))) > Networking::kfDefaultHTTPTimeout))
			{
				CS_LOG_DEBUG("HTTP Connection timed out on request: " + msDetails.strURL);
				//Flag to stop the polling thread which should 
				//exit gracefully
				mfActiveTime = 0.0f;
				mbCompleted = true;
				mbReceivedResponse = true;
				mbRequestCompleted = true;

				if(mCompletionDelegate)
				{
					mCompletionDelegate(this, HttpRequest::CompletionResult::k_timeout);
					mCompletionDelegate = nullptr;
				}
			}
		}
		//------------------------------------------------------------------
		/// Poll Read Stream
		///
		/// Reads data from the open stream when it becomes available
		/// buffers the data flags on complete
		//------------------------------------------------------------------
		void HttpConnectionSystem::CHttpRequest::PollReadStream(HINTERNET inRequestHandle, HINTERNET inConnectionHandle)
		{
			if(WinHttpSendRequest(inRequestHandle, 0, WINHTTP_NO_REQUEST_DATA, (LPVOID)msDetails.strBody.data(), msDetails.strBody.length(), msDetails.strBody.length(), nullptr))
			{
				if(WinHttpReceiveResponse(inRequestHandle, nullptr))
				{
					mbReceivedResponse = true;
				}
				else
				{
					WinHttpCloseHandle(inRequestHandle);
					WinHttpCloseHandle(inConnectionHandle);
					meRequestResult = HttpRequest::CompletionResult::k_failed;
					mbCompleted = true;
					mbThreadCompleted = true;
					return;
				}
			}
			else
			{
				WinHttpCloseHandle(inRequestHandle);
				WinHttpCloseHandle(inConnectionHandle);
				meRequestResult = HttpRequest::CompletionResult::k_failed;
				mbCompleted = true;
				mbThreadCompleted = true;
				return;
			}

			//Get the status from the server
			DWORD dwCode = 500;
			DWORD dwSize = sizeof(DWORD);
			if(WinHttpQueryHeaders(inRequestHandle, WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, nullptr, &dwCode, &dwSize, nullptr))
			{
				mudwResponseCode = (u32)dwCode;
			}

			// Keep reading from the remote server until there's
			// nothing left to read
			DWORD dwBytesToBeRead = 0;
			DWORD dwBytesRead = 0;
			s8 abyDataBuffer[kudwBufferSize];
			std::stringstream strBuffer;

			do
			{
				if(!WinHttpQueryDataAvailable(inRequestHandle, &dwBytesToBeRead))
				{
					CS_LOG_ERROR("No data avilable from server");
					WinHttpCloseHandle(inRequestHandle);
					WinHttpCloseHandle(inConnectionHandle);
					meRequestResult = HttpRequest::CompletionResult::k_failed;
					mbCompleted = true;
					mbThreadCompleted = true;
					return;
				}

				if(!WinHttpReadData(inRequestHandle, abyDataBuffer, sizeof(abyDataBuffer), &dwBytesRead))
				{
					CS_LOG_ERROR("Failed to read data from server");
					WinHttpCloseHandle(inRequestHandle);
					WinHttpCloseHandle(inConnectionHandle);
					meRequestResult = HttpRequest::CompletionResult::k_failed;
					mbCompleted = true;
					mbThreadCompleted = true;
					return;
				}

				//We have read some data
				if(dwBytesRead > 0)
				{
					strBuffer.write(abyDataBuffer, dwBytesRead);
					mudwBytesRead += dwBytesRead;
					mudwBytesReadThisBlock += dwBytesRead;

					if(mudwMaxBufferSize != 0 && mudwBytesReadThisBlock >= mudwMaxBufferSize)
					{
						mResponseData = strBuffer.str();
						strBuffer.clear();
						strBuffer.str("");
						mudwBytesReadThisBlock = 0;
						mCompletionDelegate(this, HttpRequest::CompletionResult::k_flushed);
					}
				}

			} while (dwBytesRead > 0);	

			mResponseData = strBuffer.str();
			meRequestResult = HttpRequest::CompletionResult::k_completed;
			mbCompleted = true;

			WinHttpCloseHandle(inRequestHandle);
			WinHttpCloseHandle(inConnectionHandle);

			mbThreadCompleted = true;
		}
		//----------------------------------------------------------------------------------------
		/// Cancel
		///
		/// Close the request and invoke the completion delegate with the cancel response
		//----------------------------------------------------------------------------------------
		void HttpConnectionSystem::CHttpRequest::Cancel()
		{
			mbCompleted = true;
			mbReceivedResponse = true;
			meRequestResult = HttpRequest::CompletionResult::k_cancelled;
		}
		//----------------------------------------------------------------------------------------
		/// Has Completed
		///
		/// @return Whether the request has completed - regardless of success or failure
		//----------------------------------------------------------------------------------------
		bool HttpConnectionSystem::CHttpRequest::HasCompleted() const
		{
			return mbRequestCompleted && mbThreadCompleted;
		}
		//----------------------------------------------------------------------------------------
		/// Get Details
		///
		/// @return The original request details (i.e. whether it is post/get the body and header)
		//----------------------------------------------------------------------------------------
		const Networking::HttpRequestDetails & HttpConnectionSystem::CHttpRequest::GetDetails() const
		{
			return msDetails;
		}
		//----------------------------------------------------------------------------------------
		/// Get Completion Delegate
		///
		/// @return The delegate that will be invoked on request complete
		//----------------------------------------------------------------------------------------
		const Networking::HttpRequest::CompletionDelegate & HttpConnectionSystem::CHttpRequest::GetCompletionDelegate() const
		{
			return mCompletionDelegate;
		}
		//----------------------------------------------------------------------------------------
		/// Get Response String
		///
		/// @return The contents of the response as a string. This could be binary data
		//----------------------------------------------------------------------------------------
		const std::string & HttpConnectionSystem::CHttpRequest::GetResponseString() const
		{
			return mResponseData;
		}
		//----------------------------------------------------------------------------------------
		/// Get Response Code
		///
		/// @return HTTP response code (i.e. 200 = OK, 400 = Error)
		//----------------------------------------------------------------------------------------
		u32 HttpConnectionSystem::CHttpRequest::GetResponseCode() const 
		{
			return mudwResponseCode;
		}
		//----------------------------------------------------------------------------------------
		/// Get Bytes Read
		///
		/// @return Number of bytes read til now
		//----------------------------------------------------------------------------------------
		u32 HttpConnectionSystem::CHttpRequest::GetBytesRead() const
		{
			return mudwBytesRead;
		}
		//----------------------------------------------------------------------------------------
		/// Get Connection Info
		///
		/// @return Connection Info
		//----------------------------------------------------------------------------------------
		const HttpConnectionSystem::ConnectionInfo& HttpConnectionSystem::CHttpRequest::GetConnectionInfo()
		{
			return mConnectionInfo;
		}
	}
}