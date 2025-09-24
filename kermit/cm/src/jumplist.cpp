// Portions (C) Microsoft Corporation
// Provided under the MIT License
// https://github.com/microsoft/Windows-classic-samples/blob/main/LICENSE

#ifdef CKMODERNSHELL
#include <windows.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlobj.h>
#include <tchar.h>
#include <stdio.h>

#include "jumplist.h"
#include "config_file.h"

#define ICON_APP           0
#define ICON_TERM_CONN     1
#define ICON_TERM          2
#define ICON_CM            3

bool IsItemRemoved(IShellLink *psl, IObjectArray *poaRemoved)
{
    bool found = false;
    UINT cItems;
    LPTSTR args1 = (LPTSTR)malloc(sizeof(TCHAR) * 2048);
    LPTSTR args2 = (LPTSTR)malloc(sizeof(TCHAR) * 2048);
    if (SUCCEEDED(poaRemoved->GetCount(&cItems)))
    {
        IShellLink *pslCompare;
        for (UINT i = 0; !found && i < cItems; i++)
        {
            if (SUCCEEDED(poaRemoved->GetAt(i, IID_PPV_ARGS(&pslCompare))))
            {
                if (SUCCEEDED(psl->GetArguments(args1, 2048)) &&
                    SUCCEEDED(pslCompare->GetArguments(args2, 2048))) {

                    if (lstrcmp(args1, args2) == 0) {
                        found = true;
                    }
                }
                pslCompare->Release();
            }
        }
    }
    free(args1);
    free(args2);
    return found;
}

// Create a CLSID_ShellLink object and inserts it into the jump list. Title
// must UTF-16 as there is no ANSI version of InitPropVariantFromString
static HRESULT CreateShellLink(
    LPCTSTR appPathname, LPCTSTR pszArguments, LPCWSTR pszTitle, IShellLink **ppsl)
{
    // Create the shell link object...
    IShellLink *psl;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        hr = psl->SetPath(appPathname);
        if (SUCCEEDED(hr))
        {
            hr = psl->SetArguments(pszArguments);
            if (SUCCEEDED(hr))
            {
                // Jump list items provided as a Shell Link require a title
                // whcih is used as the display name.
                IPropertyStore *pps;
                hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT propvar;
                    hr = InitPropVariantFromString(pszTitle, &propvar);
                    if (SUCCEEDED(hr))
                    {
                        hr = pps->SetValue(PKEY_Title, propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->Commit();
                            if (SUCCEEDED(hr))
                            {
                                hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                            }
                        }
                        PropVariantClear(&propvar);
                    }
                    pps->Release();
                }
            }
        }
        psl->Release();
    }
    return hr;
}

static HRESULT AddTasksToJumpList(LPCTSTR cmPathName,
                                  ICustomDestinationList *pcdl) {
    // TODO: Transform cmPathName to also produce k95gPathName

    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(
        CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        IShellLink * psl;
       /* hr = CreateShellLink(cmPathName, TEXT("-q"), L"Quick Connect...", &psl);
        if (SUCCEEDED(hr))
        {
            hr = poc->AddObject(psl);
            psl->Release();
        }*/

        if (SUCCEEDED(hr))
        {
            hr = CreateShellLink(cmPathName, TEXT(""), L"Connection Manager", &psl);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIconLocation(cmPathName, ICON_CM);
                if (SUCCEEDED(hr)) {
                    hr = poc->AddObject(psl);
                    psl->Release();
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = CreateShellLink(cmPathName, TEXT("-n"), L"New Kermit 95 Window", &psl);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIconLocation(cmPathName, ICON_TERM);
                if (SUCCEEDED(hr)) {
                    hr = poc->AddObject(psl);
                    psl->Release();
                }
            }
        }

        // TODO: Will this work? Can we do jumplist stuff with console apps?
        /*if (SUCCEEDED(hr))
        {
            hr = CreateShellLink(cmPathName, TEXT("-t"), L"New Kermit 95 Window (Console)", &psl);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }*/

        // Add all of our tasks to the tasks category
        if (SUCCEEDED(hr))
        {
            IObjectArray * poa;
            hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
            if (SUCCEEDED(hr))
            {
                hr = pcdl->AddUserTasks(poa);
                poa->Release();
            }
        }
        poc->Release();
    }
    return hr;
}

HRESULT AddMRUConnectionsToJumpList(LPCTSTR cmPathName,
                                    ICustomDestinationList *pcdl,
                                    IObjectArray *poaRemoved,
                                    UINT cMinSlots) {
    extern ConfigFile *g_ConfigFile;

    if (g_ConfigFile == NULL) return 0;

    // Call supplying no list to get the count.
    unsigned int mruProfileCount = g_ConfigFile->getMostRecentlyUsedProfiles(NULL, 0);

    // No point fetching more profiles than Windows will display.
    if (mruProfileCount > cMinSlots) mruProfileCount = cMinSlots;

    int* mruProfileIds = (int*)malloc(sizeof(int) * mruProfileCount);
    mruProfileCount = g_ConfigFile->getMostRecentlyUsedProfiles(
        mruProfileIds, mruProfileCount);

    // Nothing to add, nothing to do.
    if (mruProfileCount == 0) return 1;

    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
                                  CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        for (unsigned int i = 0; i < mruProfileCount; i++) {
            ConnectionProfile *profile = g_ConfigFile->getProfileById(
                mruProfileIds[i]);
            if (profile == NULL) continue;

            LPTSTR pszArguments = (LPTSTR)malloc(sizeof(TCHAR) * 255);
            _sntprintf(pszArguments, 255, TEXT("-c %d"), mruProfileIds[i]);

            // In unicode builds, CMString uses widechars, while in ANSI builds
            // it uses chars.
            CMString profileName = profile->name();
#ifdef UNICODE
            PCWSTR pszTitle = profileName.data();
#else
            PWSTR pszTitle = profileName.toUtf16();
#endif

            IShellLink * psl;
            hr = CreateShellLink(cmPathName, pszArguments, pszTitle, &psl);
            if (SUCCEEDED(hr))
            {
                // Check the shell link we just created isn't one of the ones
                // the user removed...
                if (!IsItemRemoved(psl, poaRemoved)) {
                    hr = psl->SetIconLocation(cmPathName, ICON_TERM_CONN);
                    if (SUCCEEDED(hr)) {
                        // if not, add it to the collection
                        hr = poc->AddObject(psl);
                        psl->Release();
                    }
                } else {
                    // This item is removed. Remove it from the MRU list so it
                    // doesn't reappear next time.
                    g_ConfigFile->removeMostRecentlyUsedProfile(profile);
                }
            }

            free(pszArguments);

#ifndef UNICODE
            free(pszTitle);
#endif
        }

        // Convert the collection to an array of objects that we can hand off
        // to the shell.
        IObjectArray *poa;
        hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
        if (SUCCEEDED(hr))
        {
            hr = pcdl->AppendCategory(L"Recent Sessions", poa);
            poa->Release();
        }
        poc->Release();
    }

    free(mruProfileIds);

    return hr;
}

void UpdateJumpList() {
    LPTSTR cmPathName = (LPTSTR)malloc(sizeof(TCHAR) * MAX_PATH);
    if (!GetModuleFileName(NULL, cmPathName, MAX_PATH))
    {
        // Failed to get pathname for the connection manager
        free(cmPathName);
        return;
    }

    // Create a new custom jump list which will replace any previous jump list.
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(
        CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr)) {
        // This is the minimum number of items the JumpList UI is guraranteed
        // to display. We can supply more, but Windows might not display them
        // depending on various factors such as user preference and screen res
        UINT cMinSlots;

        // This will contain items that the user chose to remove. We're not
        // allowed to add these back in this transaction
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
        if (SUCCEEDED(hr))
        {
            // Add content to the Jump List.
            hr = AddMRUConnectionsToJumpList(cmPathName, pcdl, poaRemoved,
                                             cMinSlots);
            if (SUCCEEDED(hr))
            {
                hr = AddTasksToJumpList(cmPathName, pcdl);
                if (SUCCEEDED(hr))
                {
                    // Commit the list-building transaction.
                    hr = pcdl->CommitList();
                }
            }
            poaRemoved->Release();
        }
        pcdl->Release();
    }

    free(cmPathName);
}

void DeleteJumpList() {
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(
        CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr))
    {
        hr = pcdl->DeleteList(NULL);
        pcdl->Release();
    }
}

#else /* not CKMODERNSHELL */

void UpdateJumpList() {
    // does nothing.
}

#endif /* CKMODERNSHELL */