//
//  ContentManagementSystem.cpp
//  Chilli Source
//
//  Created by S Downie on 04/07/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#include <ChilliSource/Networking/ContentDownload/ContentManagementSystem.h>

#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/MakeDelegate.h>
#include <ChilliSource/Core/Cryptographic/BaseEncoding.h>
#include <ChilliSource/Core/Cryptographic/HashMD5.h>
#include <ChilliSource/Core/File/FileSystem.h>
#include <ChilliSource/Core/File/FileStream.h>
#include <ChilliSource/Core/File/LocalDataStore.h>
#include <ChilliSource/Core/File/TweakableConstants.h>
#include <ChilliSource/Core/Minizip/unzip.h>
#include <ChilliSource/Core/Threading/TaskScheduler.h>

namespace ChilliSource
{
    namespace Networking
    {
        namespace
        {
            const std::string k_ldsKeyHasCached = "_CMSCachedDLC";
            
            //--------------------------------------------------------
            /// @author S Downie
            ///
            /// @param A filepath.
            ///
            /// @return Whether or not the given path contains a
            /// directory path.
            //--------------------------------------------------------
            bool ContainsDirectoryPath(const std::string& instrPath)
            {
                return (instrPath.find_first_of("/") != std::string::npos);
            }
            //--------------------------------------------------------
            /// @author S Downie
            ///
            /// @param A filepath.
            ///
            /// @return Whether or not the filepath points to a file.
            //--------------------------------------------------------
            bool IsFile(const std::string& instrPath)
            {
                return (instrPath.find_first_of(".") != std::string::npos);
            }
            //--------------------------------------------------------
            /// @author S Downie
            ///
            /// @param A filepath.
            ///
            /// @return The path of the directory the file is contained
            /// in.
            //--------------------------------------------------------
            std::string GetPathExcludingFileName(const std::string& instrPath)
            {
                u32 udwOffset = instrPath.find_last_of("/");
                if(udwOffset != std::string::npos)
                {
                    return instrPath.substr(0, udwOffset);
                }
                
                return "";
            }
            //--------------------------------------------------------
            /// @author S Downie
            ///
            /// @param A filepath.
            ///
            /// @return Returns just the filename portion of the file
            /// path.
            //--------------------------------------------------------
            std::string GetFileNameExcludingPath(const std::string& instrPath)
            {
                u32 udwOffset = instrPath.find_last_of("/");
                if(udwOffset != std::string::npos)
                {
                    if(udwOffset + 1 < instrPath.size())
                    {
                        return instrPath.substr(udwOffset+1, instrPath.size());
                    }
                }
                
                return instrPath;
            }
            //--------------------------------------------------------
            /// @author S Downie
            ///
            /// @param A filepath.
            ///
            /// @return The root folder without the path.
            //--------------------------------------------------------
            std::string GetRootFolderExcludingPath(const std::string& instrPath)
            {
                u32 udwOffset = instrPath.find_first_of("/", 1);
                if(udwOffset != std::string::npos)
                {
                    return instrPath.substr(0, udwOffset);
                }
                
                return "";
            }
        }
        CS_DEFINE_NAMEDTYPE(ContentManagementSystem);
        //--------------------------------------------------------
        //--------------------------------------------------------
        ContentManagementSystemUPtr ContentManagementSystem::Create(IContentDownloader* in_contentDownloader)
        {
            return ContentManagementSystemUPtr(new ContentManagementSystem(in_contentDownloader));
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        ContentManagementSystem::ContentManagementSystem(IContentDownloader* in_contentDownloader)
            : m_contentDownloader(in_contentDownloader), m_serverManifest(nullptr),  m_runningToDownloadTotal(0),
            m_runningDownloadedTotal(0), m_dlcCachePurged(false)
        {
        }
        //------------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::OnInit()
        {
            m_contentDirectory = Core::Application::Get()->GetFileSystem()->GetAbsolutePathToStorageLocation(Core::StorageLocation::k_DLC);
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        bool ContentManagementSystem::IsA(Core::InterfaceIDType in_interfaceId) const
        {
            return in_interfaceId == ContentManagementSystem::InterfaceID;
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::LoadLocalManifest(TiXmlDocument* in_currentManifest)
        {
            //The manifest lives in the documents directory
            in_currentManifest->LoadFile(Core::StorageLocation::k_DLC, "ContentManifest.moman");
            
            //If there is no DLC we should check to see if there ever was any
            Core::LocalDataStore* lds = Core::Application::Get()->GetSystem<Core::LocalDataStore>();
            if(!in_currentManifest->RootElement() && lds->Contains("") == true)
            {
                m_dlcCachePurged = true;
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        std::string ContentManagementSystem::GetManifestChecksumForFile(const std::string& in_filename)
        {
            return CalculateChecksum(Core::StorageLocation::k_DLC, in_filename);
        }
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		std::string ContentManagementSystem::CalculateChecksum(Core::StorageLocation in_location, const std::string& in_filePath)
		{
            std::string strMD5Checksum = Core::Application::Get()->GetFileSystem()->GetFileChecksumMD5(in_location, in_filePath);
			std::string strBase64Encoded = Core::BaseEncoding::Base64Encode(strMD5Checksum);
			Core::StringUtils::ChopTrailingChars(strBase64Encoded, '=');
			return strBase64Encoded;
		}
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::ClearDownloadData()
        {
        	//Clear the old crap
            CS_SAFEDELETE(m_serverManifest);
            m_removePackageIds.clear();
            m_packageDetails.clear();
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::CheckForUpdates(const ContentManagementSystem::CheckForUpdateDelegate& in_delegate)
        {
            //Clear any stale data from last update check
            ClearDownloadData();
            
            //Have the downloader request the manifest in it's own way
            if(m_contentDownloader->DownloadContentManifest(Core::MakeDelegate(this, &ContentManagementSystem::OnContentManifestDownloadComplete)))
            {
                //The request has started successfully
                m_onUpdateCheckCompleteDelegate = in_delegate;
            }
            else
            {
                //The request has failed to start most likely due to internet connection
                CS_LOG_ERROR("CMS: Internet not reachable");
                if(m_dlcCachePurged)
                {
                    in_delegate(CheckForUpdatesResult::k_checkFailedBlocking);
                }
                else
                {
                    in_delegate(CheckForUpdatesResult::k_checkFailed);
                }
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::DownloadUpdates(const ContentManagementSystem::CompleteDelegate& in_delegate)
        {
        	m_onDownloadCompleteDelegate = in_delegate;
            m_currentPackageDownload = 0;
            
            if(!m_packageDetails.empty())
            {
            	//Add a temp directory so that the packages are stored atomically and only overwrite
                //the originals on full success
                Core::Application::Get()->GetFileSystem()->CreateDirectoryPath(Core::StorageLocation::k_DLC, "Temp");
                m_contentDownloader->DownloadPackage(m_packageDetails[m_currentPackageDownload].m_url, Core::MakeDelegate(this, &ContentManagementSystem::OnContentDownloadComplete));
            }
            else
            {
                m_onDownloadCompleteDelegate(Result::k_succeeded);
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::DownloadNextPackage()
        {
            m_currentPackageDownload++;
            m_contentDownloader->DownloadPackage(m_packageDetails[m_currentPackageDownload].m_url, Core::MakeDelegate(this, &ContentManagementSystem::OnContentDownloadComplete));
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::InstallUpdates(const CompleteDelegate& inDelegate)
        {
            if(!m_packageDetails.empty() || !m_removePackageIds.empty())
            {
				if(!m_packageDetails.empty())
				{
					//Unzip all the files and overwrite the old manifest
					for (const auto& details : m_packageDetails)
					{
						ExtractFilesFromPackage(details);
					}
				}
                
                //Remove the temp zips
                DeleteDirectory("Temp");
                
                m_packageDetails.clear();
			
				if(!m_removePackageIds.empty())
				{
					//Remove any unused files from the documents
					for (const auto& packageId : m_removePackageIds)
					{
						DeleteDirectory(packageId);
					}
				}
                
                //Save the new content manifest
                m_serverManifest->SaveFile(Core::StorageLocation::k_DLC, "ContentManifest.moman");
                
                m_dlcCachePurged = false;
                
                //Store that we have DLC cached. If there is no DLC on next check then 
                //we know the cache has been purged and we have to block on download
                Core::LocalDataStore* lds = Core::Application::Get()->GetSystem<Core::LocalDataStore>();
                lds->SetValue(k_ldsKeyHasCached, true);
                
                //Tell the delegate all is good
                inDelegate(Result::k_succeeded);
            }
            else
            {
                //Tell the delegate all is bad
                inDelegate(Result::k_failed);
            }
            
            ClearDownloadData();
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::OnContentManifestDownloadComplete(IContentDownloader::Result in_result, const std::string& in_manifest)
        {
            switch(in_result)
            {
                case IContentDownloader::Result::k_succeeded:
                    m_serverManifestData += in_manifest;
                    BuildDownloadList(m_serverManifestData);
                    m_serverManifestData.clear();
                    break;
                case IContentDownloader::Result::k_failed:
                    m_serverManifestData.clear();
                    m_dlcCachePurged ? m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_checkFailedBlocking) : m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_checkFailed);
                    break;
                case IContentDownloader::Result::k_flushed:
                    m_serverManifestData += in_manifest;
                    break;
            };
            
            //Reset the listener
            m_onUpdateCheckCompleteDelegate = nullptr;
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::OnContentDownloadComplete(IContentDownloader::Result in_result, const std::string& in_data)
        {
        	switch(in_result)
            {
                case IContentDownloader::Result::k_succeeded:
                {
                    
                    if(SavePackageToFile(m_packageDetails[m_currentPackageDownload], in_data, true))
                    {
                        m_runningDownloadedTotal += m_packageDetails[m_currentPackageDownload].m_size;
                        
                        //Don't overwrite the old manifest until all the content has been downloaded 
                        if(m_currentPackageDownload >= (m_packageDetails.size() - 1))
                        {
                            m_onDownloadCompleteDelegate(Result::k_succeeded);
                        }
                        else
                        {
                            DownloadNextPackage();
                        }
                        
                        break;
                    }
                }
                case IContentDownloader::Result::k_failed:
                {
                    //Delete all temp zip files and cancel the outstanding requests
                    DeleteDirectory("Temp");
                    
                    if(m_onDownloadCompleteDelegate)
                    {
                        m_onDownloadCompleteDelegate(Result::k_failed);
                    }
                    break;
                }
                case IContentDownloader::Result::k_flushed:
                {
                    SavePackageToFile(m_packageDetails[m_currentPackageDownload], in_data, false);
                    break;
                }
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::BuildDownloadList(const std::string& in_serverManifest)
        {
			//Validate the server manifest
            m_serverManifest = new TiXmlDocument();
            m_serverManifest->Parse(in_serverManifest.c_str(), 0, TIXML_ENCODING_UTF8);
            
            if(!m_serverManifest->RootElement())
            {
                CS_LOG_ERROR("CMS: Server content manifest is invalid");
                if(m_dlcCachePurged)
                {
                    m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_checkFailedBlocking);
                }
                else
                {
                    m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_checkFailed);
                }

                return;
            }
            
            //Check if DLC is enabled
            if(!Core::XMLUtils::GetAttributeValueOrDefault<bool>(m_serverManifest->RootElement(), "DLCEnabled", false))
            {
				m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_notAvailable);
                return;
            }
            
            TiXmlDocument* pCurrentManifest = new TiXmlDocument();
            LoadLocalManifest(pCurrentManifest);
            
			//If we have not successfully loaded a manifest from file we need to check if any of the assets 
            //are in the bundle and pull down the others
			if(!pCurrentManifest || !pCurrentManifest->RootElement())
			{
                //Grab all the URL's from the new manifest
                TiXmlElement* pPackageEl = Core::XMLUtils::FirstChildElementWithName(m_serverManifest->RootElement(), "Package");
                while(pPackageEl)
                {
                    //If the package is not in the bundle it will download
                    AddToDownloadListIfNotInBundle(pPackageEl);
                    
                    //On to the next package
                    pPackageEl = Core::XMLUtils::NextSiblingElementWithName(pPackageEl);
                }
			}
            //Lets find out what we need already have in the manifest
			else
            {
                std::unordered_map<std::string, std::string> mapPackageIDToChecksum;
                
                //Store the data from the local manifest to make a comparison with the server manifest
                if(pCurrentManifest->RootElement())
                {
                    //Loop round 
                    TiXmlElement* pLocalPackageEl = Core::XMLUtils::FirstChildElementWithName(pCurrentManifest->RootElement(), "Package");
                    while(pLocalPackageEl)
                    {
                        //Store the local ID's and checksums for comparison later
                        std::string strLocalPackageID = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pLocalPackageEl, "ID", "");
                        std::string strLocalPackageChecksum = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pLocalPackageEl, "Checksum", "");
                        
                        mapPackageIDToChecksum.insert(std::make_pair(strLocalPackageID, strLocalPackageChecksum));
                        
                        //On to the next package
                        pLocalPackageEl = Core::XMLUtils::NextSiblingElementWithName(pLocalPackageEl);
                    }
                }
				
                //Now process the server manifest and see whats different between the two
                TiXmlElement* pServerPackageEl = Core::XMLUtils::FirstChildElementWithName(m_serverManifest->RootElement(), "Package");
                while(pServerPackageEl)
                {
                    //Store the local ID's and checksums for comparison later
                    std::string strServerPackageID = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pServerPackageEl, "ID", "");
                    std::string strServerPackageChecksum = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pServerPackageEl, "Checksum", "");
					std::string strMinVersionForPackage = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pServerPackageEl, "MinVersion", "");
                    
                    std::unordered_map<std::string, std::string>::iterator it = mapPackageIDToChecksum.find(strServerPackageID);
				
                    //Based on the state of the file decide whether it needs updating
                    if(it != mapPackageIDToChecksum.end())
                    {
                        //The package exists in the documents directory
                        if(it->second != strServerPackageChecksum)
                        {
                            //The package in documents is outdated lets check if the files all exist in the bundle
                            //that way we don't need to download
                            AddToDownloadListIfNotInBundle(pServerPackageEl);                            
                        }
                        else
                        {
                            //The checksum in the local manifest matches that on the
                            //server but the file may have been altered locally
                            //Check if the files within are corrupt
                            //Check all the file names
                            TiXmlElement* pFileEl = Core::XMLUtils::FirstChildElementWithName(pServerPackageEl, "File");
                            while(pFileEl)
                            {
                                std::string strFullPath = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pFileEl, "Location", "");
                                std::string strChecksum = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pFileEl, "Checksum", "");
                                
                                if(strFullPath.empty()) //Maintain backwards compatability with old versions
                                {
                                    std::string strFileName = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pFileEl, "Name", "");
                                    strFullPath = strServerPackageID + "/" + strFileName;
                                }
                                
                                if(!DoesFileExist(strFullPath, strChecksum, false))
                                {
                                    std::string strPackageUrl = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pServerPackageEl, "URL", "");
                                    u32 udwPackageSize = Core::XMLUtils::GetAttributeValueOrDefault<u32>(pServerPackageEl, "Size", 0);
                                    m_runningToDownloadTotal += udwPackageSize;

                                    PackageDetails packageDetails;
                                    packageDetails.m_id = strServerPackageID;
                                    packageDetails.m_url = strPackageUrl;
                                    packageDetails.m_checksum = strServerPackageChecksum;
                                    packageDetails.m_size = udwPackageSize;
                                    m_packageDetails.push_back(packageDetails);
                                    break;
                                }
                                
                                pFileEl = Core::XMLUtils::NextSiblingElementWithName(pFileEl);
                            }
                        }
                        
                        mapPackageIDToChecksum.erase(it);
                    }
                    else
                    {
                        //Couldn't find the package in the local manifest if it isn't
                        //in the bundle then we need to download it
                        AddToDownloadListIfNotInBundle(pServerPackageEl);
                    }
                    
                    //On to the next package
                    pServerPackageEl = Core::XMLUtils::NextSiblingElementWithName(pServerPackageEl);
                }
               
                //Any packages left in the local manifest need to be removed
                for(std::unordered_map<std::string, std::string>::iterator it = mapPackageIDToChecksum.begin(); it != mapPackageIDToChecksum.end(); ++it)
                {
                    m_removePackageIds.push_back(it->first);
                }
            }
            
            //Notify the delegate of our completion and whether the need to update anything
            bool bRequiresUpdating = (!m_removePackageIds.empty() || !m_packageDetails.empty());
            
            if(bRequiresUpdating && m_dlcCachePurged)
            {
                m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_availableBlocking);
            }
            else if(bRequiresUpdating && !m_dlcCachePurged)
            {
                m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_available);
            }
            else
            {
                m_onUpdateCheckCompleteDelegate(CheckForUpdatesResult::k_notAvailable);
            }
            
            CS_SAFEDELETE(pCurrentManifest);
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::AddToDownloadListIfNotInBundle(TiXmlElement* in_packageEl)
        {
            //Check all the file names
            TiXmlElement* pFileEl = Core::XMLUtils::FirstChildElementWithName(in_packageEl, "File");
            while(pFileEl)
            {
                std::string strFileName = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pFileEl, "Name", "");
                std::string strChecksum = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(pFileEl, "Checksum", "");
                std::string strPackageID = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(in_packageEl, "ID", "");
                
                if(!DoesFileExist(strPackageID + "/" + strFileName, strChecksum, true))
                {
                    //It doesn't exist in the bundle either!
                    std::string strPackageUrl = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(in_packageEl, "URL", "");
                    std::string strPackageChecksum = Core::XMLUtils::GetAttributeValueOrDefault<std::string>(in_packageEl, "Checksum", "");
                    
                    u32 udwPackageSize = Core::XMLUtils::GetAttributeValueOrDefault<u32>(in_packageEl, "Size", 0);
					m_runningToDownloadTotal += udwPackageSize;
                    
                    PackageDetails packageDetails;
                    packageDetails.m_id = strPackageID;
                    packageDetails.m_url = strPackageUrl;
                    packageDetails.m_checksum = strPackageChecksum;
                    packageDetails.m_size = udwPackageSize;
                    m_packageDetails.push_back(packageDetails);
                    //It doesn't matter if all the other files exist we need to pull down this package anyway
                    break;
                }
                else
                {
                    //It exists in the bundle let's remove the old version from DLC cache
                    //Remove old content
                    Core::Application::Get()->GetFileSystem()->DeleteFile(Core::StorageLocation::k_DLC, strPackageID + "/" + strFileName);
                    
                    //On to the next file
                    pFileEl = Core::XMLUtils::NextSiblingElementWithName(pFileEl);
                }
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        bool ContentManagementSystem::SavePackageToFile(const PackageDetails& in_packageDetails, const std::string& in_zippedPackage, bool in_fullyDownloaded)
        {
            std::string strFile = "Temp/" + in_packageDetails.m_id + ".packzip";

            //Append to the file as it can take multiple writes
            Core::FileStreamSPtr pFileStream = Core::Application::Get()->GetFileSystem()->CreateFileStream(Core::StorageLocation::k_DLC, strFile, Core::FileMode::k_writeBinaryAppend);
			pFileStream->Write((s8*)in_zippedPackage.data(), (s32)in_zippedPackage.size());
            pFileStream->Close();
            
            //Check if the full file has been written and perform a checksum validation 
            if(in_fullyDownloaded)
            {
                std::string strChecksum = CalculateChecksum(Core::StorageLocation::k_DLC, strFile);
                if(strChecksum != in_packageDetails.m_checksum)
                {
                    CS_LOG_ERROR("CMS: " + in_packageDetails.m_id + " Package download corrupted");
                    return false;
                }
            }
                
            return true;
        }

        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::ExtractFilesFromPackage(const ContentManagementSystem::PackageDetails& in_packageDetails) const
        {
			//Open zip
			std::string strZipFilePath(m_contentDirectory + "/Temp/" + in_packageDetails.m_id + ".packzip");
			
			unzFile ZippedFile = unzOpen(strZipFilePath.c_str());
			if(!ZippedFile)
			{
				CS_LOG_ERROR("CMS: Cannot unzip content package: " + in_packageDetails.m_id);
				return;
			}

            //Remove old content before installing the new stuff
            DeleteDirectory(in_packageDetails.m_id);
            
            //Go to the first file in the zip
            const u64 uddwFilenameLength = 256;
            s8 byaFileName[uddwFilenameLength];
            
            s32 dwStatus = unzGoToFirstFile(ZippedFile);
            
            while(dwStatus == UNZ_OK)
            {
                //Open the next file
                if (unzOpenCurrentFile(ZippedFile) != UNZ_OK)
                    break;
                
                //Get file information
                unz_file_info FileInfo;
                unzGetCurrentFileInfo(ZippedFile, &FileInfo, byaFileName, uddwFilenameLength, nullptr, 0, nullptr, 0);
                
                //Load the file into memory and then save it out to the directory
                s8* pbyDataBuffer =  new s8[FileInfo.uncompressed_size];
                unzReadCurrentFile(ZippedFile, pbyDataBuffer, FileInfo.uncompressed_size);
                
                //Create new stuff
                std::string strFilePath = std::string(byaFileName);
                if(ContainsDirectoryPath(strFilePath))
                {
                    //There is a nested folder so we need to create the directory structure
                    std::string strPath = GetPathExcludingFileName(strFilePath);
                    Core::Application::Get()->GetFileSystem()->CreateDirectoryPath(Core::StorageLocation::k_DLC, "/" + strPath);
                }
                
                if(IsFile(strFilePath))
                {
                    Core::Application::Get()->GetFileSystem()->WriteFile(Core::StorageLocation::k_DLC, "/" + strFilePath, pbyDataBuffer, FileInfo.uncompressed_size);
                }
                
                //Close current file and jump to the next
                CS_SAFEDELETE_ARRAY(pbyDataBuffer);
                unzCloseCurrentFile(ZippedFile);
                dwStatus = unzGoToNextFile(ZippedFile);
            }
            
            //Close the zip
            unzClose(ZippedFile);
        }
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		u32 ContentManagementSystem::GetRunningTotalToDownload()
		{
			return m_runningToDownloadTotal;
		}
		//-----------------------------------------------------------
		//-----------------------------------------------------------
		u32 ContentManagementSystem::GetRunningTotalDownloaded()
		{
			return m_runningDownloadedTotal + m_contentDownloader->GetCurrentDownloadedBytes();
		}
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        IContentDownloader* ContentManagementSystem::GetContentDownloader() const
        {
            return m_contentDownloader;
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        bool ContentManagementSystem::DoesFileExist(const std::string& in_filename, const std::string in_checksum, bool in_checkOnlyBundle)
        {
            if(in_checkOnlyBundle)
            {
                if(Core::Application::Get()->GetFileSystem()->DoesFileExist(Core::StorageLocation::k_package, Core::Application::Get()->GetFileSystem()->GetPackageDLCPath() + in_filename))
                {
                    //Check if the file has become corrupted
                    return (CalculateChecksum(Core::StorageLocation::k_package, Core::Application::Get()->GetFileSystem()->GetPackageDLCPath() + in_filename) == in_checksum);
                }
                
                return false;
            }
            else
            {
                if(Core::Application::Get()->GetFileSystem()->DoesFileExist(Core::StorageLocation::k_DLC, in_filename))
                {
                    //Check if the file has become corrupted
                    return (CalculateChecksum(Core::StorageLocation::k_DLC, in_filename) == in_checksum);
                }
                
                return false;
            }
        }
        //-----------------------------------------------------------
        //-----------------------------------------------------------
        void ContentManagementSystem::DeleteDirectory(const std::string& in_directory) const
        {
            ChilliSource::Core::Application::Get()->GetFileSystem()->DeleteDirectory(Core::StorageLocation::k_DLC, in_directory);
        }
    }
}
