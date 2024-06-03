// DX11_fullscreen_portrait_repro.cpp : Defines the entry point for the application.
//


// D3D11
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler
#pragma comment( lib, "dxguid.lib" )


#include "framework.h"
#include "DX11_fullscreen_portrait_repro.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxgidebug.h>   // DXGI_INFO_QUEUE

#include <assert.h>
#include <vector>
#include <inttypes.h>


#define ARRAY_COUNT(array) \
    (sizeof(array) / (sizeof(array[0]) * (sizeof(array) != sizeof(void *) || sizeof(array[0]) <= sizeof(void *))))





#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DX11FULLSCREENPORTRAITREPRO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DX11FULLSCREENPORTRAITREPRO));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DX11FULLSCREENPORTRAITREPRO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DX11FULLSCREENPORTRAITREPRO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


HWND hWnd;

const UINT numFrames = 2;

// Ref: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12device
// Note: This interface was introduced in Windows 10. Applications targetting Windows 10 should use this interface instead of later versions.
// Applications targetting a later version of Windows 10 should use the appropriate version of the ID3D12Device interface.
// The latest version of this interface is ID3D12Device3 introduced in Windows 10 Fall Creators Update.


//ID3D12Device10* device10 = nullptr;
//ID3D12Device9* device = nullptr;
//ID3D12Device8* device = nullptr;    // Works on Windows 10, despite documentation note above
//ID3D12Device7* device = nullptr;
//ID3D12Device6* device = nullptr;
//ID3D12Device5* device = nullptr;
ID3D12Device4* device = nullptr;       // CreateCommandList1
//ID3D12Device3* device = nullptr;
//ID3D12Device2* device = nullptr;
//ID3D12Device1* device = nullptr;

//ID3D12Device* device = nullptr;
// 
//ID3D12Resource2* resource2 = nullptr;

//IDXGISwapChain1* swapchain = nullptr;
IDXGISwapChain4* swapchain4 = nullptr;

ID3D12Resource2* render_target_view = nullptr;


UINT window_width = 0;
UINT window_height = 0;
BOOL DXGI_fullscreen = false;
BOOL allowTearing = false;


void dxgi_debug_pre_device_init()
{
    //assert(device);

#ifndef NDEBUG
    //ID3D12Debug6* debugController;
    //ID3D12Debug5* debugController;
    //ID3D12Debug4* debugController;
    ID3D12Debug3* debugController;
    //ID3D12Debug2* debugController;
    //ID3D12Debug1* debugController;
    //ID3D12Debug* debugController;
    
    // Enable the debug layer
    {
        HRESULT hr = 0;

        //hr = D3D12GetInterface(CLSID_D3D12Debug, IID_ID3D12Debug6, (void**)&debugController6);
        //hr = D3D12GetInterface(CLSID_D3D12Debug, IID_ID3D12Debug, (void**)&debugController);
        hr = D3D12GetInterface(CLSID_D3D12Debug, IID_PPV_ARGS(&debugController));

        if (SUCCEEDED(hr))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("Failed to obtain D3D12Debug interface");
            exit(-1);
        }


    }
}

void dxgi_debug_post_device_init()
{
    assert(device);

    // Debug
    {
        //  Note
        //  For Windows 10, to create a device that supports the debug layer, enable the "Graphics Tools" optional feature.Go to the Settings panel, 
        //  under System, Apps& features, Manage optional Features, Add a feature, and then look for "Graphics Tools".


        // for debug builds enable VERY USEFUL debug break on API errors
        {
            //ID3D12InfoQueue1* info = nullptr;
            ID3D12InfoQueue* info = nullptr;

            HRESULT hr = 0;
            hr = device->QueryInterface(IID_PPV_ARGS(&info));
            
            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to query interface for ID3D12InfoQueue from device, did you EnableDebugLayer?");
                exit(-1);
            }
            assert(info);

#if 0
            ID3D12InfoQueue1*info1;
            hr = info->QueryInterface(IID_PPV_ARGS(&info1));

            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to query interface for ID3D12InfoQueue1 from ID3D12InfoQueue");
                exit(-1);
            }
            assert(info1);
#endif


            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, TRUE);

            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_MISCELLANEOUS, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_INITIALIZATION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_CLEANUP, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_COMPILATION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_STATE_CREATION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_STATE_SETTING, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_STATE_GETTING, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_SHADER, TRUE);

            //info->AddMessage(D3D12_MESSAGE_CATEGORY_MISCELLANEOUS, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST MISCELLANOUS ERROR");
            //info->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, "TEST");
            //info->AddMessage(D3D12_MESSAGE_CATEGORY_MISCELLANEOUS, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST");

            info->Release();
        }

        // enable debug break for DXGI too
        {
            IDXGIInfoQueue* info; 
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&info));

            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE, TRUE);

            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER, TRUE);


            // NOTE: ApplicationMessage will not let us break
            //OutputDebugStringA("This will not trigger??? Why?\n");
            //info->AddApplicationMessage(DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, "TEST");

            // NOTE: Result message will let us break
            //info->AddMessage(DXGI_DEBUG_DXGI, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST");

            //debug_info->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

            info->Release();
        }

        // after this there's no need to check for any errors on device functions manually
        // so all HRESULT return values in this code will be ignored
        // debugger will break on errors anyway
    }
#endif

}


ID3D12CommandQueue* commandQueue = nullptr;
ID3D12DescriptorHeap* rtvHeap = nullptr;
ID3D12Resource2* framebuffer[numFrames] = {};


// We need a command list
/*
device->CreateCommandList();
device->CreateCommandList1();
device->CreateCommandQueue();
device->CreateCommandQueue1();
*/

ID3D12CommandAllocator* commandAllocator = nullptr;
ID3D12GraphicsCommandList* commandList = nullptr;

ID3D12PipelineState* pipelineState = nullptr;


ID3D12Fence1* fence = nullptr;
UINT64 fenceValue = 0;
HANDLE fenceEvent = INVALID_HANDLE_VALUE;

D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[numFrames];

D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
};

D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = numFrames,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0
};



void InitD3D12(void)
{
    HRESULT result;
    IDXGIAdapter4* dxgiAdapter = nullptr;


    D3D_FEATURE_LEVEL feature_level_req[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

#ifdef NDEBUG
    UINT flags = 0; 
#else
    UINT flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    IDXGIFactory7* pFactory;
    result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pFactory));

    if (!SUCCEEDED(result))
    {
        exit(EXIT_FAILURE);
    }


    // Feature support

    // Usage of CheckFeatureSupport is poorly documented, but this examples shows the way
    // Ref: https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays

    pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));


    dxgi_debug_pre_device_init();

    // Device selection
    D3D_DRIVER_TYPE d3d_driver_type = D3D_DRIVER_TYPE_HARDWARE;



    // Get adapter
    UINT i = 0;
    IDXGIAdapter1* pAdapter;
    std::vector <IDXGIAdapter1*> vAdapters;
    while (pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        ++i;
    }

    for (auto adapter : vAdapters)
    {
        DXGI_ADAPTER_DESC1 adapterDesc1;
        adapter->GetDesc1(&adapterDesc1);

        OutputDebugStringW(adapterDesc1.Description);

        if (wcscmp(adapterDesc1.Description, L"Radeon RX 580 Series") == 0)
            continue;

        dxgiAdapter = (IDXGIAdapter4 *) adapter;
        break;

    }

    //result = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_ID3D12Device10, (void **) & device);
    //result = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_ID3D12Device, (void**)&device);
    result = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device) );
    

    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed D3D12CreateDevice\n");
        exit(EXIT_FAILURE);
    }
    device->SetName(L"device");

    LUID luid = device->GetAdapterLuid();
    result = pFactory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&dxgiAdapter));

    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed to enumerate Adapter from device LUID\n");
        exit(EXIT_FAILURE);
    }

    dxgi_debug_post_device_init();


    IDXGIOutput* dxgiOutput = nullptr;

    // Get the display output description, we'll identify the window coordinates of the display and move our window to it
    {
        UINT i = 0;
        IDXGIOutput* pOutput;
        std::vector<IDXGIOutput*> vOutputs;

        RECT display_rect = {};
        while (dxgiAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            pOutput->GetDesc(&desc);

            OutputDebugStringA("Device : \n");
            OutputDebugString(desc.DeviceName);

            switch (desc.Rotation)
            {
            case DXGI_MODE_ROTATION_UNSPECIFIED:
                OutputDebugStringA("Rotation: DXGI_MODE_ROTATION_UNSPECIFIED\n");
                break;

            case DXGI_MODE_ROTATION_IDENTITY:
                OutputDebugStringA("Rotation: DXGI_MODE_ROTATION_IDENTITY\n");
                break;

            case DXGI_MODE_ROTATION_ROTATE90:
                OutputDebugStringA("Rotation: DXGI_MODE_ROTATION_ROTATE90\n");
                break;

            case DXGI_MODE_ROTATION_ROTATE180:
                OutputDebugStringA("Rotation: DXGI_MODE_ROTATION_ROTATE180\n");
                break;

            case DXGI_MODE_ROTATION_ROTATE270:
                OutputDebugStringA("Rotation: DXGI_MODE_ROTATION_ROTATE270\n");
                break;
            }

            display_rect = desc.DesktopCoordinates;
            dxgiOutput = pOutput;
            //pOutput->Release();

            // Return the first enumerated output, this should always be the device where the primary desktop is running
            // There are better ways, this is throwaway code

            // If you want fullscreen to occur against a different display, make that display the primary desktop
            // Note this has no bearing on the fullscreen issue we're trying to demonstrate in Portrait orientation, this code is simplified for a  minimal repro.

            break;
            //++i;
        }


        // Ensure the window it located on the desktop identified by the above

        RECT rect;
        GetWindowRect(hWnd, &rect);
        MoveWindow(hWnd, display_rect.left, display_rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
    }


    assert(dxgiOutput);

    DXGI_MODE_DESC1 *best_fullscreen_mode = nullptr;

    // Check hardware composition support
    {
        IDXGIOutput6* dxgiOutput6 = nullptr;
        DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAGS hardware_composition_support = {};

        result = dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput6));

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to query dxgiOutput6 from dxgiOutput\n");
            exit(EXIT_FAILURE);
        }

        result = dxgiOutput6->CheckHardwareCompositionSupport((UINT*)&hardware_composition_support);

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to query hardware composition support from dxgiOutput6\n");
            exit(EXIT_FAILURE);
        }

        if (hardware_composition_support && DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN)
            OutputDebugStringA("DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_FULLSCREEN == Supported\n");

        if (hardware_composition_support && DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)
            OutputDebugStringA("DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED == Supported\n");

        if (hardware_composition_support && DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED)
            OutputDebugStringA("DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_CURSOR_STRETCHED == Supported\n");


        assert(dxgiOutput6);

        UINT flags = 0;
        UINT numModes = 0;

        // Consider available display modes. Prefer that which matches our current (ideally native) desktop dimensions

        dxgiOutput6->GetDisplayModeList1(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            flags,
            &numModes,
            nullptr
        );

        DXGI_MODE_DESC1* mode_descriptions = (DXGI_MODE_DESC1*)malloc(sizeof(DXGI_MODE_DESC1) * numModes);

        dxgiOutput6->GetDisplayModeList1(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            flags,
            &numModes,
            mode_descriptions
        );

        OutputDebugStringA("Fullscreen modes available\n");
        OutputDebugStringA("Mode    Format Width Height     Refresh  Stereo Scaling ScanlineOrdering\n");


        DXGI_OUTPUT_DESC1 output_desc1;
        dxgiOutput6->GetDesc1(&output_desc1);
        UINT display_height = (UINT)output_desc1.DesktopCoordinates.bottom - output_desc1.DesktopCoordinates.top;
        UINT display_width = (UINT)output_desc1.DesktopCoordinates.right - output_desc1.DesktopCoordinates.left;


        for (unsigned int i = 0; i < numModes; i++)
        {
            DXGI_MODE_DESC1* ptr = &mode_descriptions[i];

            char msg[1024];
            snprintf(msg, 1024, "%4d  %8x  %4u   %4u %5u/%05u   %5s    %4x             %4x\n",
                i,
                ptr->Format,
                ptr->Width,
                ptr->Height,
                ptr->RefreshRate.Numerator, ptr->RefreshRate.Denominator,
                ptr->Stereo ? "True" : "False",
                ptr->Scaling,
                ptr->ScanlineOrdering);

            OutputDebugStringA(msg);

            if (ptr->Width == display_width && ptr->Height == display_height && !ptr->Stereo)
            {
                OutputDebugStringA("* Selected\n");
                best_fullscreen_mode = ptr;
            }
        }

        if (!best_fullscreen_mode)
        {
            MessageBoxA(hWnd, "Could not determine a native display mode matching the desktop dimensions for the active window. Fullscreen dimensions unspecified", "", MB_OK);
        }

        //assert(best_fullscreen_mode);
    }

    commandQueue = nullptr;
    // Command queue creation
    {

        const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL ,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE ,
            .NodeMask = 0,
        };

        device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
        commandQueue->SetName(L"commandQueue");
    }
    

    // Swapchain creation
    {
        UINT swapchain_flags = 0;

        //swapchain_flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        swapchain_flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;


        DXGI_SWAP_CHAIN_DESC1 swapchain_descriptor = {
            //.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, // DXGI_FORMAT_B8G8R8A8_UNORM,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,		// TODO: How to specify SRGB? 
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
             },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = numFrames,								// Needs to be >= 2 for FLIP swap effect
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,	// DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            .Flags = swapchain_flags,
        };


        if (best_fullscreen_mode)
        {
            swapchain_descriptor.Format = best_fullscreen_mode->Format;
            swapchain_descriptor.Width = best_fullscreen_mode->Width;
            swapchain_descriptor.Height = best_fullscreen_mode->Height;
            swapchain_descriptor.Stereo = best_fullscreen_mode->Stereo;
        }


        bool windowed = true;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_desc = {
            .RefreshRate = {
                .Numerator = 0,
                .Denominator = 1
            },
            .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
            .Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
            //.Scaling = DXGI_MODE_SCALING_CENTERED,
            //.Scaling = DXGI_MODE_SCALING_STRETCHED,
            .Windowed = windowed
        };


        IDXGISwapChain1 *swapchain1;
        // device:  For Direct3D 12 this is a pointer to a direct command queue (refer to ID3D12CommandQueue).
        result = pFactory->CreateSwapChainForHwnd(commandQueue, hWnd, &swapchain_descriptor, &fullscreen_desc, nullptr, &swapchain1);

        // Cast swapchain1 to swapchain4 (apparently, this is the way?)
        swapchain4 = (IDXGISwapChain4*)swapchain1;

        UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();
        

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreateSwapChainForHwnd from dxgiFactory\n");
            exit(EXIT_FAILURE);
        }


        // Uncomment to prevent Alt+Enter from triggering a fullscreen switch
        //pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        pFactory->Release();
        dxgiAdapter->Release();
        //dxgiDevice->Release();

        assert(S_OK == result && swapchain4 && device);
    
        // First obtain the framebuffer images from the swapchain

        D3D12_HEAP_PROPERTIES heapProperties = {
            .Type = D3D12_HEAP_TYPE_DEFAULT,
        };

        D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;


        RECT rect;
        GetClientRect(hWnd, &rect);
        UINT64 width = rect.right - rect.left;
        UINT height = rect.bottom - rect.top;
        

#if 0   // Buffer should be provided by the swapchain, we don't need to create our own here
        D3D12_RESOURCE_DESC resourceDesc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = width,
            .Height = height,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,           
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
             },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET // | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,            
        };

        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        //D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_PRESENT;

        D3D12_CLEAR_VALUE clearValue = {
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            .Color = { 0.2f, 0.2f, 0.7f, 1.0f }
        };

        device->CreateCommittedResource(
            &heapProperties,
            heapFlags,
            &resourceDesc,
            state,
            &clearValue,
            IID_PPV_ARGS(&framebuffer));

        framebuffer->SetName(L"framebuffer");

        result = swapchain4->GetBuffer(0, IID_ID3D12Resource2, (void**)&framebuffer);
        assert(SUCCEEDED(result));
#endif

        


        // Now we can create the render target image view (pointing at the framebufer images already)


        // We need a descriptor heap for the render target view

        //ID3D12Heap1 *rtvHeap = nullptr;
        //ID3D12Heap* rtvHeap = nullptr;
        rtvHeap = nullptr;

        result = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
        rtvHeap->SetName(L"rtvDescriptorHeap");

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreateDescriptorHeap\n");
            exit(EXIT_FAILURE);
        }

        rtvHeap->SetName(L"rtvHeap");

        
        // Create render target views for each swapchain image
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
            const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

            // Create a renderTargetView for each swapchain frame
            for (uint8_t i = 0; i < numFrames; i++)
            {
                swapchain4->GetBuffer(i, IID_PPV_ARGS(&framebuffer[i]));
                device->CreateRenderTargetView(framebuffer[i], &rtvDesc, rtvHandle);

                wchar_t name[32];
                wsprintf(name, L"Framebuffer %d of %d", i, numFrames);
                framebuffer[i]->SetName(name);
                rtvHandles[i] = rtvHandle;
                rtvHandle.ptr += incrementSize;                
            }

            D3D12_RESOURCE_DESC desc = framebuffer[frameIndex]->GetDesc();
            D3D12_RESOURCE_DESC1 desc1 = framebuffer[frameIndex]->GetDesc1();

            //framebuffer->Release();
        }


        // We need a fence to signal when the frame has rendered
        {
            device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            fenceValue = 1;

            // TODO: This is a windows native event
            // Ref: https://learn.microsoft.com/en-us/windows/win32/sync/using-event-objects

            // Create an event handle to use for frame synchronisation
            fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

            if (fenceEvent == nullptr)
            {
                OutputDebugStringA("Failed to create fence event");
                exit(-1);
            }
        }

    }


    // Create a command allocator
    {
        HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreateCommandAllocator\n");
            exit(EXIT_FAILURE);
        }

        commandAllocator->SetName(L"allocator");
    }


    // TODO: Create pipeline state
    /*
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

        HRESULT result = device->CreatePipelineState(desc, IID_PPV_ARGS(&pipelineState));

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreatePipelineState\n");
            exit(EXIT_FAILURE);
        }

    }
    */


    // Create a command list
    {
        HRESULT result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, pipelineState, IID_PPV_ARGS(&commandList));

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreateCommandList\n");
            exit(EXIT_FAILURE);
        }
        commandList->SetName(L"commandList");
        commandList->Close();
    }

}


void WaitForPreviousFrame(void)
{
    // TODO: WAITING FOR THE FRAME TO COMPLETE BEFORE CONTIUING IS NOT BEST PRACTICE.
    // see DX12 sample code from HelloWindow.cpp

    const UINT64 localFenceValue = fenceValue;

    commandQueue->Signal(fence, localFenceValue);
    fenceValue++;

    // Wait until the previous frame is finished
    if (fence->GetCompletedValue() < localFenceValue)
    {
        fence->SetEventOnCompletion(localFenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // TODO: Relies on swapchain3, where does that live?
    UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();
}


void flushGpu()
{
    //for (int i = 0; i < numFrames; i++)
    {
        uint64_t fenceValueForSignal = ++fenceValue;
        commandQueue->Signal(fence, fenceValueForSignal);
        if (fence->GetCompletedValue() < fenceValue)
        {
            fence->SetEventOnCompletion(fenceValueForSignal, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }
    //frameIndex = 0;
}


void render(void)
{
    UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();

    // Now start rendering
    
    commandList->Reset(commandAllocator, nullptr);

    // Indicate that the back buffer will be used as a render target

    {
        // Create a resource barrier
        D3D12_RESOURCE_BARRIER resourceBarrierTransitionPresentTarget = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                .pResource = (ID3D12Resource*)framebuffer[frameIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET
            }
        };

        commandList->ResourceBarrier(1, &resourceBarrierTransitionPresentTarget);
    }
    
    // TODO: Set root signature
    //commandList1->SetGraphicsRootSignature(rootsig);


    // Clear the backbuffer entirely
    {
        const float clearColor1[4] = { 0.2f, 0.2f, 0.7f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandles[frameIndex], clearColor1, 0, nullptr);
    }


    // Set the viewport
    {
        D3D12_VIEWPORT viewport = {
            0.0f, 0.0f,   // X, Y
            (FLOAT)window_width,
            (FLOAT)window_height,
            0.0f, 1.0f  // DepthMin, DepthMax
        };

        char msg[1024];
        snprintf(msg, 1024, "render time viewport dimensions %f x %f\n", viewport.Width, viewport.Height);
        OutputDebugStringA(msg);
        commandList->RSSetViewports(1, &viewport);
    }


    // Set render targets
    {
        commandList->OMSetRenderTargets(1, &rtvHandles[frameIndex], FALSE, nullptr);
    }


    // Draw something
    {
        // Clear a 200x200 pixel square at position 100x100
        {
            const FLOAT clearColor[4] = { 0.7f, 0.7f, 1.0f, 1.0f };

            // D3D11 appears to be top to bottom
            //RECT = {x1, y1, x2, y2}
            const D3D12_RECT rect = {
                .left = 100,
                .top = 100,
                .right = 300,
                .bottom = 300
            };

            commandList->ClearRenderTargetView(rtvHandles[frameIndex], clearColor, 1, &rect);
        }


        {
            const FLOAT clearColor[4] = { 1.0f, 0.2f, 0.7f, 1.0f };
            D3D12_RECT rect;
            rect.left = std::min(600, (int32_t)window_width);
            rect.top = std::min(100, (int32_t)window_height);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Pink from top
            commandList->ClearRenderTargetView(rtvHandles[frameIndex], clearColor, 1, &rect);
        }


        {
            const FLOAT clearColor2[4] = { 0.2f, 1.0f, 0.7f, 1.0f };
            D3D12_RECT rect;
            rect.left = std::min(600L, (LONG)window_width);
            rect.top = std::max(0L, (LONG)window_height - 600L);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Green from bottom
            commandList->ClearRenderTargetView(rtvHandles[frameIndex], clearColor2, 1, &rect);
        }

        // TODO: How to enable blending?
        // We need a blend attachment

        {
            const FLOAT clearColor3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            D3D12_RECT rect;
            rect.left = std::max(600L, (LONG)window_width - 200L);
            rect.top = std::max(0L, (LONG)window_height - 200L);
            rect.right = rect.left + 100;
            rect.bottom = rect.top + 100;

            // Black from bottom right
            commandList->ClearRenderTargetView(rtvHandles[frameIndex], clearColor3, 1, &rect);
        }

    }


    // Indicate that the back buffer will be used to present

    {
        // Create a resource barrier
        D3D12_RESOURCE_BARRIER resourceBarrierTransitionTargetPresent = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                .pResource = (ID3D12Resource*)framebuffer[frameIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                .StateAfter = D3D12_RESOURCE_STATE_PRESENT
            }
        };

        commandList->ResourceBarrier(1, &resourceBarrierTransitionTargetPresent);
    }


    commandList->Close();


    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);

    /*  The fix here, we needed to set a resource barrier at the start/end of the commandList to indicate a state transition from PRESENT to RENDER_TARGET, and at the end from RENDER_TARGET to PRESENT

    * D3D12 ERROR: ID3D12CommandQueue::ExecuteCommandLists: Using ClearRenderTargetView on Command List (0x000001C6EA147D30:'Unnamed ID3D12GraphicsCommandList Object'):
          Resource state (0x0: D3D12_RESOURCE_STATE_[COMMON|PRESENT]) of resource (0x000001C6EA11A530:'Unnamed ID3D12Resource Object') (subresource: 0) is invalid for use as a render target.
          Expected State Bits (all): 0x4: D3D12_RESOURCE_STATE_RENDER_TARGET,
          Actual State: 0x0: D3D12_RESOURCE_STATE_[COMMON|PRESENT],
          Missing State: 0x4: D3D12_RESOURCE_STATE_RENDER_TARGET. [ EXECUTION ERROR #538: INVALID_SUBRESOURCE_STATE]

    */


    // Probably we need to reset some state, or otherwise wait for the frame to be marked as complete (WaitForFrame())
    /*
    *
    * D3D12 ERROR: ID3D12CommandQueue::ExecuteCommandLists: A command list, which writes to a swapchain back buffer, may only be executed when that back buffer is the back buffer
        that will be presented during the next call to Present*.
        Such a back buffer is also referred to as the "current back buffer".
        Swap Chain: 0x0000021A24B53BF0:'Unnamed Object' - Current Back Buffer Buffer: 0x0000021A24BB8990:'Unnamed ID3D12Resource Object' -
    */



    const UINT vsync = 1;
    const UINT presentFlags = 0;
    const DXGI_PRESENT_PARAMETERS presentParameters = {};

    swapchain4->Present1(vsync, presentFlags, &presentParameters);

    WaitForPreviousFrame();
}



BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   InitD3D12();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}



// Enable to handle portrait orientation displays ourselves (opt out of automatic pre-rotation)
#define D3D_SUPPORT_NONPREROTATED 0


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_CHAR:
        {
            if (wParam == 's')
            {

                DXGI_fullscreen = !DXGI_fullscreen;

#if 0
                IDXGIOutput* output;
                swapchain->GetContainingOutput(&output);

                DXGI_OUTPUT_DESC output_desc;
                output->GetDesc(&output_desc);

                window_width = output_desc.DesktopCoordinates.right - output_desc.DesktopCoordinates.left;
                window_height = output_desc.DesktopCoordinates.bottom - output_desc.DesktopCoordinates.top;
#endif


#if D3D_SUPPORT_NONPREROTATED
                //this->device_context_11_x->ClearState();
                bool apply_rotation = DXGI_fullscreen && (output_desc.Rotation == DXGI_MODE_ROTATION_ROTATE90 || output_desc.Rotation == DXGI_MODE_ROTATION_ROTATE270);
#else
                bool apply_rotation = false;
#endif

                DXGI_MODE_DESC target_mode = {
                    .Width = window_width,
                    .Height = window_height,
                    .Format = DXGI_FORMAT_UNKNOWN
                };

                if (apply_rotation)
                {
                    target_mode.Width = window_height;
                    target_mode.Height = window_width;
                }


                if (DXGI_fullscreen)
                {
                    // Only when going *into* fullscreen (avoids flicker)
                    swapchain4->ResizeTarget(&target_mode);
                }

                swapchain4->SetFullscreenState(DXGI_fullscreen, nullptr);


/*
                {
                    ID3D11Texture2D* framebuffer;
                    HRESULT result = swapchain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&framebuffer);
                    assert(SUCCEEDED(result));
                    D3D11_TEXTURE2D_DESC framebufferSurfaceDesc;
                    framebuffer->GetDesc(&framebufferSurfaceDesc);
                    char msg[1024];
                    snprintf(msg, 1024, "Framebuffer surface dimensions : %d x %d\n", framebufferSurfaceDesc.Width, framebufferSurfaceDesc.Height);
                    OutputDebugStringA(msg);
                    framebuffer->Release();
                }
*/

            }
            return 0;

        } break;

    case WM_SIZE:
        {
            char msg[1024];
            snprintf(msg, 1024, "New WM_SIZE event: %d x %d\n", LOWORD(lParam), HIWORD(lParam));

            OutputDebugStringA(msg);

            UINT swapchain_flags = 0;
            //swapchain_flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            BOOL is_fullscreen;
            swapchain4->GetFullscreenState(&is_fullscreen, nullptr);

    #if D3D_SUPPORT_NONPREROTATED
            if (DXGI_fullscreen && is_fullscreen)
                swapchain_flags |= DXGI_SWAP_CHAIN_FLAG_NONPREROTATED;
    #endif

            // It is recommended to always use the tearing flag when it is supported.
            swapchain_flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;


            IDXGIOutput* output;
            HRESULT hr = swapchain4->GetContainingOutput(&output);
            if (!SUCCEEDED(hr))
            {
                // This will fail if the window resides on a display for a differencedevice
                // TODO: Handle this
                OutputDebugStringA("Failed to retrieve containing output, window may have moved to a different display/interface?");
                exit(-1);
            }

            DXGI_OUTPUT_DESC output_desc;
            output->GetDesc(&output_desc);
            output->Release();


            //this->device_context_11_x->ClearState();
            bool apply_rotation = DXGI_fullscreen && ((swapchain_flags & DXGI_SWAP_CHAIN_FLAG_NONPREROTATED)) && (output_desc.Rotation == DXGI_MODE_ROTATION_ROTATE90 || output_desc.Rotation == DXGI_MODE_ROTATION_ROTATE270);

            if (DXGI_fullscreen)
            {
                // Get dimensions from display output
                window_width = output_desc.DesktopCoordinates.right - output_desc.DesktopCoordinates.left;
                window_height = output_desc.DesktopCoordinates.bottom - output_desc.DesktopCoordinates.top;
            }
            else
            {
                window_width = LOWORD(lParam);
                window_height = HIWORD(lParam);
            }

            if (apply_rotation)
            {
                UINT tmp = window_width;
                window_width = window_height;
                window_height = tmp;
            }
          


            DXGI_MODE_DESC target_mode = {
                .Width = window_width,
                .Height = window_height,
                .Format = DXGI_FORMAT_UNKNOWN
            };

            swapchain4->ResizeTarget(&target_mode);


#if 0   // Not yet operational

            UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();


            // Reset the command list
            //flushGpu();
            //WaitForPreviousFrame();
            commandAllocator->Reset();
            commandList->Reset(commandAllocator, nullptr);


            // Set render targets
            commandList->OMSetRenderTargets(1, &rtvHandles[frameIndex], FALSE, nullptr);


            // Release all outstanding references to the swap chain's buffers.
            //render_target_view->Release();


            //DXGI_OUTPUT_DESC output_desc;
            //DXGI_OUTPUT_DESC1 output_desc1;
            //platform_backend_d3d11().d3d11.device_info->dxgiOutput6->GetDesc(&output_desc);
            //dxgiOutput6->GetDesc1(&output_desc1);

            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.


            swapchain4->ResizeBuffers(numFrames, window_width, window_height, DXGI_FORMAT_UNKNOWN, swapchain_flags);

            //hr = swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swapchain_flags);
            //hr = swapchain->ResizeBuffers(0, window_width, window_height, DXGI_FORMAT_UNKNOWN, swapchain_flags);

            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to resize swapchain buffer\n");
                exit(EXIT_FAILURE);
            }

            // Get buffer and create a render-target-view.
            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
                const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

                for (uint8_t i = 0; i < numFrames; i++)
                {
                    hr = swapchain4->GetBuffer(i, IID_PPV_ARGS(&framebuffer[i]));

                    if (!SUCCEEDED(hr))
                    {
                        OutputDebugStringA("Failed to retrieve swapchain buffer\n");
                        exit(EXIT_FAILURE);
                    }

                    D3D12_RESOURCE_DESC1 desc1 = framebuffer[frameIndex]->GetDesc1();

                    //char msg[1024];
                    snprintf(msg, 1024, "Swapchain buffer[%u] size : %" PRIu64 " x %u\n", i, desc1.Width, desc1.Height);
                    OutputDebugStringA(msg);

                    device->CreateRenderTargetView(framebuffer[i], &rtvDesc, rtvHandle);
                    //hr = device->CreateRenderTargetView(pBuffer, NULL, &render_target_view);

                    if (!SUCCEEDED(hr))
                    {
                        OutputDebugStringA("Failed to create render target view\n");
                        exit(EXIT_FAILURE);
                    }


                    wchar_t name[32];
                    wsprintf(name, L"Framebuffer %d of %d", i, numFrames);
                    framebuffer[i]->SetName(name);
                    rtvHandles[i] = rtvHandle;
                    rtvHandle.ptr += incrementSize;
                }
            }

            commandList->OMSetRenderTargets(1, &rtvHandles[frameIndex], FALSE, nullptr);
            //device_context_11_0->OMSetRenderTargets(1, &render_target_view, nullptr);


            // Set up the viewport.
            D3D12_VIEWPORT vp = {
                .Width = (float)window_width,
                .Height = (float)window_height
            };

 
            // Set viewports
            
            D3D12_VIEWPORT viewport = {
                0.0f, 0.0f,   // X, Y
                (FLOAT)window_width,
                (FLOAT)window_height,
                0.0f, 1.0f  // DepthMin, DepthMax
            };

            commandList->RSSetViewports(1, &viewport);
            //device_context_11_0->RSSetViewports(1, &vp);
            //commandList->Close();
            //ID3D12CommandList* ppCommandLists[] = { commandList };
            //commandQueue->ExecuteCommandLists(1, ppCommandLists);
            
            //WaitForPreviousFrame();
#endif

        }
        return 0;

    case WM_PAINT:
        {
            //PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            //EndPaint(hWnd, &ps);

            render();
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
