// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

//#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include "strsafe.h"
#include "stringapiset.h"
#include <string>

namespace Havtorn
{
    I32 ToWideString(const std::string& s, std::wstring& ws)
    {
        std::wstring wsTmp(s.begin(), s.end());
        ws = wsTmp;
        return 0;
    }

    I32 ToString(const std::wstring& ws, std::string& s)
    {
#pragma warning( push )
#pragma warning( disable : 4244 )
        std::string sTmp(ws.begin(), ws.end());
        s = sTmp;
        return 0;
#pragma warning( pop )
    }

    // CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces 
    //              to create and store a shortcut to the specified object. 
    //
    // Returns the result of calling the member functions of the interfaces. 
    //
    // Parameters:
    // objectPath  - Address of a buffer that contains the path of the object,
    //                including the file name.
    // linkPath     - Address of a buffer that contains the path where the 
    //                Shell link is to be stored, including the file name.
    // lpszDesc     - Address of a buffer that contains a description of the 
    //                Shell link, stored in the Comment field of the link
    //                properties.
    HRESULT CreateLink(const char* objectPath, const char* linkPath, LPCWSTR lpszDesc)
    {
        HRESULT hres;
        IShellLink* psl;

        // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
        // has already been called.
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (SUCCEEDED(hres))
        {
            IPersistFile* ppf;

            std::wstring wideObjectPath;
            ToWideString(objectPath, wideObjectPath);
            LPCWSTR lpszPathObj = wideObjectPath.c_str();

            // Set the path to the shortcut target and add the description. 
            psl->SetPath(lpszPathObj);
            psl->SetDescription(lpszDesc);

            // Query IShellLink for the IPersistFile interface, used for saving the 
            // shortcut in persistent storage. 
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

            if (SUCCEEDED(hres))
            {
                // Save the link by calling IPersistFile::Save. 
                std::wstring wideLinkPath;
                ToWideString(linkPath, wideLinkPath);
                LPCWSTR lpszPathLink = wideLinkPath.c_str();

                hres = ppf->Save(lpszPathLink, TRUE);
                ppf->Release();
            }
            psl->Release();
        }
        return hres;
    }

    // ResolveIt - Uses the Shell's IShellLink and IPersistFile interfaces 
    //             to retrieve the path and description from an existing shortcut. 
    //
    // Returns the result of calling the member functions of the interfaces. 
    //
    // Parameters:
    // hwnd         - A handle to the parent window. The Shell uses this window to 
    //                display a dialog box if it needs to prompt the user for more 
    //                information while resolving the link.
    // linkPath     - Address of a buffer that contains the path of the link,
    //                including the file name.
    // objectPath   - Address of a buffer that receives the path of the link
    //                target, including the file name.
    // lpszDesc     - Address of a buffer that receives the description of the 
    //                Shell link, stored in the Comment field of the link
    //                properties.
    HRESULT ResolveIt(HWND hwnd, const char* linkPath, char* objectPath, int iPathBufferSize)
    {
        HRESULT hres;
        IShellLink* psl;
        WCHAR szGotPath[MAX_PATH];
        WCHAR szDescription[MAX_PATH];
        WIN32_FIND_DATA wfd;

        *objectPath = 0; // Assume failure 

        // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
        // has already been called. 
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (SUCCEEDED(hres))
        {
            IPersistFile* ppf;

            // Get a pointer to the IPersistFile interface. 
            hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

            if (SUCCEEDED(hres))
            {
                // Load the shortcut. 
                std::wstring wideLinkPath;
                ToWideString(linkPath, wideLinkPath);
                hres = ppf->Load(wideLinkPath.c_str(), STGM_READ);

                if (SUCCEEDED(hres))
                {
                    // Resolve the link. 
                    hres = psl->Resolve(hwnd, 0);

                    if (SUCCEEDED(hres))
                    {
                        // Get the path to the link target. 
                        hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

                        if (SUCCEEDED(hres))
                        {
                            // Get the description of the target. 
                            hres = psl->GetDescription(szDescription, MAX_PATH);

                            if (SUCCEEDED(hres))
                            {
                                std::string gotPath;
                                ToString(szGotPath, gotPath);
                                hres = StringCbCopyA(objectPath, iPathBufferSize, gotPath.c_str());
                                if (SUCCEEDED(hres))
                                {
                                    // Handle success
                                }
                                else
                                {
                                    // Handle the error
                                }
                            }
                        }
                    }
                }

                // Release the pointer to the IPersistFile interface. 
                ppf->Release();
            }

            // Release the pointer to the IShellLink interface. 
            psl->Release();
        }
        return hres;
    }
}
