/*
 *  TwitterPostSystem.cpp
 *  moFlow
 *
 *  Created by Scott Downie on 06/09/2011.
 *  Copyright 2011 Tag Games. All rights reserved.
 *
 */

#include <ChilliSource/Core/Cryptographic/BaseEncoding.h>
#include <ChilliSource/Core/File/LocalDataStore.h>
#include <ChilliSource/Social/Twitter/TwitterPostSystem.h>

#ifdef TARGET_OS_IPHONE
#include <ChilliSource/Backend/Platform/iOS/Social/Twitter/TwitterPostSystem.h>
#elif TARGET_ANDROID
#include <ChilliSource/Backend/Platform/Android/Social/Twitter/TwitterPostSystem.h>
#endif

namespace ChilliSource
{
	namespace Social
	{
		DEFINE_NAMED_INTERFACE(ITwitterPostSystem);

        ITwitterPostSystem* ITwitterPostSystem::CreateSystem(Networking::IHttpConnectionSystem* inpHttpConnectionSystem, Core::COAuthSystem* inpOAuthSystem)
        {
#ifdef TARGET_OS_IPHONE
            return new ChilliSource::iOS::CTwitterPostSystem(static_cast<iOS::CHttpConnectionSystem*>(inpHttpConnectionSystem), inpOAuthSystem);
#elif TARGET_ANDROID
            return new ChilliSource::Android::CTwitterPostSystem(static_cast<Android::CHttpConnectionSystem*>(inpHttpConnectionSystem), inpOAuthSystem);
#endif
			return NULL;
        }
        
		//------------------------------------------------------------------------
		/// Constructor
		//------------------------------------------------------------------------
		ITwitterPostSystem::ITwitterPostSystem(Networking::IHttpConnectionSystem* inpHttpConnectionSystem,
											   Core::COAuthSystem* inpOAuthSystem) : mstrCustomerKey(""),
																						   mstrCustomerSecret(""),
																						   mpAuthenticationView(NULL)
		{
				mpHttpConnectionSystem = inpHttpConnectionSystem;
				mpOAuthSystem = inpOAuthSystem;
				MOFLOW_ASSERT(mpHttpConnectionSystem != NULL && mpOAuthSystem != NULL, "Twitter post system requires the http request system and oauth system.");
		}
		//------------------------------------------------------------------------
		/// Destructor
		//------------------------------------------------------------------------
		ITwitterPostSystem::~ITwitterPostSystem()
		{
		}
		//------------------------------------------------------------------------
        /// Init
        //------------------------------------------------------------------------
        bool ITwitterPostSystem::Init(const std::string& instrCustomerKey, const std::string& instrCustomerSecret)
        {
        	bool bResult = false;
			
			msDesc.strUrl.clear();
			msDesc.strText.clear();
			
        	if(!instrCustomerKey.empty() && !instrCustomerSecret.empty())
        	{
        		mstrCustomerKey = instrCustomerKey;
        		mstrCustomerSecret = instrCustomerSecret;
        		if(mpOAuthSystem)
        		{
        			mpOAuthSystem->SetConsumerKey(mstrCustomerKey);
        			mpOAuthSystem->SetConsumerSecret(mstrCustomerSecret);
        			bResult = true;
        		}
				else
					ERROR_LOG("CTwitterPostSystem::Init() - ERROR: Twitter does not have an OAuth system!");
        	}
        	else
        		ERROR_LOG("CTwitterPostSystem::Init() - ERROR: Twitter customer key and/or customer secret string(s) empty!");
			
        	return bResult;
        }
        //------------------------------------------------------------------------
        /// Run the OAuth process and, if successful, leave the system in state
        /// ready to communicate with Twitter
        //------------------------------------------------------------------------
        bool ITwitterPostSystem::Authenticate()
        {
        	WARNING_LOG("ITwitterPostSystem::Authenticate() - This platform does not have an Authenticate() method\n\tTwitter may not function correctly or at all!");

        	return true;
        }
        //------------------------------------------------------------------------
		/// Post Using moFlow
		//------------------------------------------------------------------------
		bool ITwitterPostSystem::PostUsingMoFlow(const Social::TwitterPostDesc & insDesc)
		{
			bool bResult = false;

			// But frist we need to check our post is within character count limits
			if(insDesc.strText.size() <= 0 || insDesc.strText.size() > Social::TwitterDefault::kudwCharacterLimit)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_failed);
				}
				bResult = false;
			}
			else
			if(mpHttpConnectionSystem->CheckReachability())
			{
				// Construct our Tweet request URL
				std::string strStatus = Social::TwitterDefault::TWITTER_STATUS_STRING + Core::CBaseEncoding::URLEncode(insDesc.strText.ToASCII());
				std::string strStatusURL = Social::TwitterURL::TWITTER_STATUS_UPDATE_URL;
				std::string strOAuthHeader;

				if(mpOAuthSystem->GetOAuthHeader(Core::COAuthSystem::OAuthHttpRequestType::k_httpPost,
												 strStatusURL,
												 strStatus, strOAuthHeader))
				{
					Networking::HttpRequestDetails sHttpRequest;
					sHttpRequest.strURL = Social::TwitterURL::TWITTER_STATUS_UPDATE_URL;
					sHttpRequest.strBody = strStatus;
					sHttpRequest.eType = Networking::HttpRequestDetails::Type::k_post;
					sHttpRequest.sHeaders.SetValueForKey("Content-Type", "application/x-www-form-urlencoded");
					sHttpRequest.sHeaders.SetValueForKey(Social::TwitterOAuthAPIHeaders::TWITTER_OAUTH_REQUEST_HEADER_AUTHORIZATION, strOAuthHeader);

					mpHttpConnectionSystem->MakeRequest(sHttpRequest, Networking::IHttpRequest::CompletionDelegate(this, &ITwitterPostSystem::OnStatusUpdateComplete));
					bResult = true;
				}
			}

			return bResult;
		}
		//------------------------------------------------------------------------
		/// Delegate method called when a status update has completed
		//------------------------------------------------------------------------
		void ITwitterPostSystem::OnStatusUpdateComplete(Networking::HttpRequestPtr inpRequest, Networking::IHttpRequest::CompletionResult ineResult)
		{
			if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_completed)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_success);
				}
			}
			else if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_failed)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_failed);
				}
			}
			else if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_cancelled)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_cancelled);
				}
			}
			else if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_timeout)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_failed);
				}
			}
			else if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_flushed)
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_failed);
				}
			}
			else
			{
				if(mCompletionDelegate)
				{
					mCompletionDelegate(Social::ITwitterPostSystem::PostResult::k_failed);
				}
			}
		}
		//------------------------------------------------------------------------
		/// Gets a request token and secret. This is used to authorise the user
		/// and get a PIN from Twitter
		//------------------------------------------------------------------------
		bool ITwitterPostSystem::RequestOAuthToken(std::string& outstrAuthoriseURL)
		{
			bool bResult = false;

			outstrAuthoriseURL.clear();

			if(mpHttpConnectionSystem->CheckReachability())
			{
				// Construct our OAuth request URL - 'oob' means out-of-band and tells Twitter we
				// are on a mobile device.
				std::string strURL = Social::TwitterOAuthAPIURLs::TWITTER_OAUTH_REQUEST_TOKEN_URL + Social::TwitterDefault::TWITTER_URL_SEP_QUESTION_MARK + Core::OAUTHLIB_CALLBACK_KEY + "=oob";
				std::string strOAuthHeader;

				if(mpOAuthSystem->GetOAuthHeader(Core::COAuthSystem::OAuthHttpRequestType::k_httpGet,
												 strURL,
												 std::string(""), strOAuthHeader))
				{
					Networking::HttpRequestDetails sHttpRequest;
					sHttpRequest.strURL = Social::TwitterOAuthAPIURLs::TWITTER_OAUTH_REQUEST_TOKEN_URL;
					sHttpRequest.eType = Networking::HttpRequestDetails::Type::k_get;
					sHttpRequest.sHeaders.SetValueForKey("Content-Type", "application/x-www-form-urlencoded");
					sHttpRequest.sHeaders.SetValueForKey(Social::TwitterOAuthAPIHeaders::TWITTER_OAUTH_REQUEST_HEADER_AUTHORIZATION, strOAuthHeader);

					mpHttpConnectionSystem->MakeRequest(sHttpRequest, Networking::IHttpRequest::CompletionDelegate(this, &ITwitterPostSystem::OnRequestOAuthTokenComplete));
					bResult = true;
				}
			}

			return bResult;
		}
		//------------------------------------------------------------------------
		/// Delegate called with RequestOAuthToken() completes.
		//------------------------------------------------------------------------
		void ITwitterPostSystem::OnRequestOAuthTokenComplete(ChilliSource::Networking::HttpRequestPtr inpRequest, ChilliSource::Networking::IHttpRequest::CompletionResult ineResult)
		{
			if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_completed)
			{
				// Tell OAuth system to save access token and secret from web response
				mpOAuthSystem->ExtractOAuthTokenKeySecret(inpRequest->GetResponseString());

				// Get access token and secret from OAuth object
				std::string strOAuthTokenKey;
				mpOAuthSystem->GetOAuthTokenKey(strOAuthTokenKey);

				// Build authorise URL so that user can visit in browser and get PIN
				std::string outstrAuthoriseURL;
				outstrAuthoriseURL.assign(Social::TwitterOAuthAPIURLs::TWITTER_OAUTH_AUTHORIZE_URL);
				outstrAuthoriseURL.append(strOAuthTokenKey.c_str());

				// OK, let's tell the authentication view our URL
				// From here the user will be able to login to Twitter and enter the PIN
				if(mpAuthenticationView)
					mpAuthenticationView->PresentURL(outstrAuthoriseURL);
			}
		}
		//------------------------------------------------------------------------
		/// Exchange our request token with an access token
		//------------------------------------------------------------------------
		bool ITwitterPostSystem::RequestOAuthAccessToken()
		{
			bool bResult = false;

			if(mpHttpConnectionSystem->CheckReachability())
			{
				std::string strOAuthHeader;

				if(mpOAuthSystem->GetOAuthHeader(Core::COAuthSystem::OAuthHttpRequestType::k_httpGet,
												 Social::TwitterOAuthAPIURLs::TWITTER_OAUTH_ACCESS_TOKEN_URL,
												 std::string(""), strOAuthHeader, true))
				{
					Networking::HttpRequestDetails sHttpRequest;
					sHttpRequest.strURL = Social::TwitterOAuthAPIURLs::TWITTER_OAUTH_ACCESS_TOKEN_URL;
					sHttpRequest.eType = Networking::HttpRequestDetails::Type::k_get;
					sHttpRequest.sHeaders.SetValueForKey("Content-Type", "application/x-www-form-urlencoded");
					sHttpRequest.sHeaders.SetValueForKey(Social::TwitterOAuthAPIHeaders::TWITTER_OAUTH_REQUEST_HEADER_AUTHORIZATION, strOAuthHeader);

					mpHttpConnectionSystem->MakeRequest(sHttpRequest, Networking::IHttpRequest::CompletionDelegate(this, &ITwitterPostSystem::OnRequestOAuthAccessTokenComplete));
					bResult = true;
				}
			}

			return bResult;
		}
		//------------------------------------------------------------------------
		/// Delegate called with RequestOAuthToken() completes.
		//------------------------------------------------------------------------
		void ITwitterPostSystem::OnRequestOAuthAccessTokenComplete(ChilliSource::Networking::HttpRequestPtr inpRequest, ChilliSource::Networking::IHttpRequest::CompletionResult ineResult)
		{
			if(ineResult == ChilliSource::Networking::IHttpRequest::CompletionResult::k_completed)
			{
				DEBUG_LOG("CTwitterPostSystem::OnRequestOAuthAccessTokenComplete() - Got response:\n"+inpRequest->GetResponseString());
				// Tell OAuth system to save access token and secret from web response
				mpOAuthSystem->ExtractOAuthTokenKeySecret(inpRequest->GetResponseString());
				SaveOAuthTokenKeyAndSecretKey();
				// Now that we've finish with all that shenanigans, let us Tweet...
				PostUsingMoFlow(msDesc);
			}
		}
		//------------------------------------------------------------------------
		/// Trys to load save token and secret keys
		//------------------------------------------------------------------------
		void ITwitterPostSystem::TryLoadAuthenticationKeys()
		{
			std::string strTokenKey;
			std::string strSecretKey;

			ChilliSource::Core::CLocalDataStore& pLocalData = ChilliSource::Core::CLocalDataStore::GetSingleton();

			if(pLocalData.TryGetValue(Social::TwitterDataStore::kstrDataStoreKey[(u32)Social::TwitterDataStore::DataStoreKey::k_OAuthToken], strTokenKey))
			{
				mstrSavedOAuthTokenKey.assign(strTokenKey);
			}
			else
			{
				mstrSavedOAuthTokenKey.clear();
			}

			if(pLocalData.TryGetValue(Social::TwitterDataStore::kstrDataStoreKey[(u32)Social::TwitterDataStore::DataStoreKey::k_OAuthSecret], strSecretKey))
			{
				mstrSavedOAuthTokenSecret.assign(strSecretKey);
			}
			else
			{
				mstrSavedOAuthTokenSecret.clear();
			}

			if(mstrSavedOAuthTokenKey.empty() || mstrSavedOAuthTokenSecret.empty())
			{
				// At least one has failed to load
				mstrSavedOAuthTokenKey.clear();
				mstrSavedOAuthTokenSecret.clear();
			}
		}
		//------------------------------------------------------------------------
		/// Saves out current OAuth token key and secret key
		//------------------------------------------------------------------------
		void ITwitterPostSystem::SaveOAuthTokenKeyAndSecretKey()
		{
			ChilliSource::Core::CLocalDataStore& pLocalData = ChilliSource::Core::CLocalDataStore::GetSingleton();

			// Save our OAuth Token and Secret
			mpOAuthSystem->GetOAuthTokenKey(mstrSavedOAuthTokenKey);
			mpOAuthSystem->GetOAuthTokenSecret(mstrSavedOAuthTokenSecret);

			pLocalData.SetValueForKey(Social::TwitterDataStore::kstrDataStoreKey[(u32)Social::TwitterDataStore::DataStoreKey::k_OAuthToken], mstrSavedOAuthTokenKey);
			pLocalData.SetValueForKey(Social::TwitterDataStore::kstrDataStoreKey[(u32)Social::TwitterDataStore::DataStoreKey::k_OAuthSecret], mstrSavedOAuthTokenSecret);
			pLocalData.Synchronise();
		}
	}
}

