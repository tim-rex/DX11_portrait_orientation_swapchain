// DX12_fullscreen_portrait_repro.cpp : Defines the entry point for the application.
//

// D3D12
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "dxcompiler.lib" )  // DX12 shader compiler
#pragma comment( lib, "dxguid.lib" )

#include "framework.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h> // DX12 shader compiler

#include <dxgidebug.h>

#include <assert.h>
#include <vector>
#include <inttypes.h>
#include <process.h>

#include "DX12_fullscreen_portrait_repro.h"



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
    LoadStringW(hInstance, IDC_DXFULLSCREENPORTRAITREPRO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DXFULLSCREENPORTRAITREPRO));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXFULLSCREENPORTRAITREPRO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DXFULLSCREENPORTRAITREPRO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


HWND hWnd;


// Ref: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12device
// Note: This interface was introduced in Windows 10. Applications targetting Windows 10 should use this interface instead of later versions.
// Applications targetting a later version of Windows 10 should use the appropriate version of the ID3D12Device interface.
// The latest version of this interface is ID3D12Device3 introduced in Windows 10 Fall Creators Update.


//ID3D12Device10* device10 = nullptr;
//ID3D12Device9* device = nullptr;
ID3D12Device8* device = nullptr;    // Works on Windows 10, despite documentation note above
//ID3D12Device7* device = nullptr;
//ID3D12Device6* device = nullptr;
//ID3D12Device5* device = nullptr;
//ID3D12Device4* device = nullptr;       // CreateCommandList1
//ID3D12Device3* device = nullptr;
//ID3D12Device2* device = nullptr;
//ID3D12Device1* device = nullptr;

//ID3D12Device* device = nullptr;
// 
//ID3D12Resource2* resource2 = nullptr;

//IDXGISwapChain1* swapchain = nullptr;
IDXGISwapChain4* swapchain4 = nullptr;


UINT window_width = 0;
UINT window_height = 0;
BOOL DXGI_fullscreen = false;
BOOL allowTearing = false;
D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};

BOOL framechanged = false;


void dxgi_debug_report()
{
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        //pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        //pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
        //pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);

        pDebug->Release();
        pDebug = nullptr;
    }
}


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
        hr = D3D12GetInterface(CLSID_D3D12Debug, IID_PPV_ARGS(&debugController));

        if (SUCCEEDED(hr))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            OutputDebugStringA("Failed to obtain D3D12Debug interface");
            exit(EXIT_FAILURE);
        }


    }
#endif
}

void dxgi_debug_post_device_init()
{
#ifndef NDEBUG
    assert(device);

    // Debug
    {
        //  Note
        //  For Windows 10, to create a device that supports the debug layer, enable the "Graphics Tools" optional feature.Go to the Settings panel, 
        //  under System, Apps& features, Manage optional Features, Add a feature, and then look for "Graphics Tools".


        // for debug builds enable VERY USEFUL debug break on API errors
        {
            ID3D12InfoQueue* info = nullptr;

            HRESULT hr = 0;
            hr = device->QueryInterface(IID_PPV_ARGS(&info));
            
            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to query interface for ID3D12InfoQueue from device, did you EnableDebugLayer?");
                exit(EXIT_FAILURE);
            }
            assert(info);

#if 0
            ID3D12InfoQueue1*info1;
            hr = info->QueryInterface(IID_PPV_ARGS(&info1));

            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to query interface for ID3D12InfoQueue1 from ID3D12InfoQueue");
                exit(EXIT_FAILURE);
            }
            assert(info1);
#endif


            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            //info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
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
            //info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_SHADER, TRUE);

            //info->AddMessage(D3D12_MESSAGE_CATEGORY_MISCELLANEOUS, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST MISCELLANOUS ERROR");
            //info->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, "TEST");
            //info->AddMessage(D3D12_MESSAGE_CATEGORY_MISCELLANEOUS, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST");

            info->Release();
            info = nullptr;
        }

        // enable debug break for DXGI too
        {
            IDXGIInfoQueue* info; 
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&info));

            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, FALSE);
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
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION, FALSE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER, TRUE);


            // NOTE: ApplicationMessage will not let us break
            //OutputDebugStringA("This will not trigger??? Why?\n");
            //info->AddApplicationMessage(DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, "TEST");

            // NOTE: Result message will let us break
            //info->AddMessage(DXGI_DEBUG_DXGI, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_UNKNOWN, "TEST");

            //debug_info->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

            info->Release();
            info = nullptr;
        }

        {
            IDXGIDebug1* pDebug = nullptr;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
            {
                pDebug->EnableLeakTrackingForThread();
                pDebug->Release();
                pDebug = nullptr;
            }
        }

        // after this there's no need to check for any errors on device functions manually
        // so all HRESULT return values in this code will be ignored
        // debugger will break on errors anyway


    }
#endif


    shaderModel.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;

    while (device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)) == E_INVALIDARG)
    {
        if (shaderModel.HighestShaderModel == D3D_SHADER_MODEL_5_1)
        {
            OutputDebugStringA("Failed to check D3D12_FEATURE_SHADER_MODEL from highest down to to 5_1");
            exit(EXIT_FAILURE);
        }
        else if (shaderModel.HighestShaderModel == D3D_SHADER_MODEL_6_0)
            shaderModel.HighestShaderModel = D3D_SHADER_MODEL_5_1;
        else
            shaderModel.HighestShaderModel = (D3D_SHADER_MODEL) (shaderModel.HighestShaderModel - 1);
    }

    {
        char msg[1024];
        snprintf(msg, 1024, "Highest available shader model : %x", shaderModel.HighestShaderModel);

        if (shaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_1)
            shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_1;
        else
        {
            snprintf(msg, 1024, "Require shader model %x but only %x is available", D3D_SHADER_MODEL_6_1, shaderModel.HighestShaderModel);
            OutputDebugStringA(msg);
            exit(EXIT_FAILURE);
        }
    }

    {
        D3D12_FEATURE_DATA_FEATURE_LEVELS levels = {};

        device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &levels, sizeof(levels));

        char msg[1024];
        snprintf(msg, 1024, "Max supported feature level : %x", levels.MaxSupportedFeatureLevel);
        OutputDebugStringA(msg);
    }
}


#define USE_WARP 0

#define MSAA_ENABLED 1
#define ROOT_CONSTANTS_ENABLED 1
#define DRAW_LOTS_UNOPTIMISED 1

#define RENDER_THREADS 4

const int numThreads = RENDER_THREADS;

const UINT numFrames = 2;
const UINT backbufferFrames = numFrames + 1;


#if RENDER_THREADS
struct ThreadParameter
{
    int threadIndex;
    ID3D12CommandAllocator* commandAllocator[backbufferFrames] = {};
    ID3D12GraphicsCommandList* commandList = nullptr;
};

ThreadParameter m_threadParameters[numThreads];

HANDLE threadHandles[numThreads];
HANDLE threadSignalBeginRenderFrame[numThreads];
HANDLE threadSignalFinishRenderFrame[numThreads];
#endif


ID3D12CommandQueue* commandQueue = nullptr;
ID3D12DescriptorHeap* rtvHeap = nullptr;

#if !ROOT_CONSTANTS_ENABLED
ID3D12DescriptorHeap* cbvHeap = nullptr;
#endif



// vsync off
// numFrames = 2 + backbufferFrames = 2
// 1 command allocator
// DRAW_LOTS disabled
// 580 internal frames + 60 fps present

// increasing numFrames + backbufferFrames == 3
// 620 internal frames + 60 fps present

// increasing backBufferFrames to 4 (1 more than numFrames)
// 750 internal frames


// If we use MULTIPLE (backbufferFrames) command allocators, we can get

// numFrames = 2 + backbufferFrames = 2
// 2 command allocators
// DRAW_LOTS disabled
// 550 to 900 internal frames + 57 fps present (skipping ?)
// Without contention
// 2,700 internal frames (stable) + 30 fps present (???)


// increasing numFrames + backbufferFrames == 3
// 600 - 800 internal frames + 60 fps present (locked) but some internal drops to 200(?)
// Without contention
// 2,800 internal frames (stable) + 30 fps present (???)


// increasing backBufferFrames to 4 (1 more than numFrames)
// 750 internal frames (stable) + 60 fps present (locked)
// Without contention
// 2,800 internal frames (stable) + 30 fps present (???)


// The discrepancy between internal frames vs present frames only occurs in windowed (composited) mode.. FSE or windowed fullscreen shows the same number of presents regardless
// Is it just presentmon being wierd?

// Multiple command allocators makes no difference



ID3D12Resource2* framebuffer[backbufferFrames] = {};

#if MSAA_ENABLED
ID3D12DescriptorHeap* rtvHeap_MSAA = nullptr;
ID3D12Resource2* framebuffer_MSAA[backbufferFrames] = {};
#endif

// We need a command allocater + command list

ID3D12CommandAllocator* commandAllocator[backbufferFrames] = {};



// Define the constant data used to communicate with shaders.
struct VS_CONSTANT_BUFFER
{
    float width;
    float height;
    float time;
    float padding[61]; // Padding so the constant buffer is 256-byte aligned.
} VS_CONSTANT_BUFFER;

static_assert((sizeof(VS_CONSTANT_BUFFER) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

struct VS_CONSTANT_BUFFER VsConstData_dims = {};

#if !ROOT_CONSTANTS_ENABLED
uint8_t* persistentlyMappedConstantBuffer = nullptr;
D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView;
#endif


ID3D12GraphicsCommandList* commandListPre = nullptr;
ID3D12GraphicsCommandList* commandListPost = nullptr;

ID3D12RootSignature* rootSig = nullptr;
ID3D12PipelineState* pso = nullptr;

ID3D12Fence1* fence = nullptr;
UINT64 fenceValues[backbufferFrames];

HANDLE fenceEvent = INVALID_HANDLE_VALUE;

D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[backbufferFrames];

D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
};

#if MSAA_ENABLED

D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_MSAA[backbufferFrames];

D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_MSAA = {
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS,
};

constexpr unsigned floorlog2(unsigned x)
{
    return x == 1 ? 0 : 1 + floorlog2(x >> 1);
}

constexpr unsigned ceillog2(unsigned x)
{
    return x == 1 ? 0 : floorlog2(x - 1) + 1;
}

const UINT MAX_SAMPLE_COUNT = 32;   // Taken from D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT. No way to query this in D3D12

constexpr int availableMultisampleLevels = ceillog2(MAX_SAMPLE_COUNT) + 1;

UINT sampleCountQuality[availableMultisampleLevels];
UINT MSAA_Count = 4;
UINT MSAA_Quality = 0;

#endif


D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = backbufferFrames,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0
};

#if MSAA_ENABLED
// TODO: We could just use the same rtvHeapDesc (and increase NumDescriptors appropriately)

D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc_MSAA = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = backbufferFrames,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0
};
#endif


#if !ROOT_CONSTANTS_ENABLED
D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    .NumDescriptors = 1,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
    .NodeMask = 0
};
#endif


void InitD3D12(void)
{
    HRESULT result;
    IDXGIAdapter4* dxgiAdapter = nullptr;

#ifdef NDEBUG
    UINT flags = 0; 
#else
    UINT flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    IDXGIFactory7* pFactory;
    result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pFactory));

    if (FAILED(result))
    {
        exit(EXIT_FAILURE);
    }

    // Feature support

    // Usage of CheckFeatureSupport is poorly documented, but this examples shows the way
    // Ref: https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays

    // Validate available feature level / shader model
    // While Direct3D 12 can support older compiled shader blobs, shaders should be built using either Shader 
    // Model 5.1 with the FXC/D3DCompile APIs, or using Shader Model 6 using the DXIL DXC compiler. 
    // You should validate Shader Model 6 support with CheckFeatureSupport and D3D12_FEATURE_SHADER_MODEL.

    pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

    dxgi_debug_pre_device_init();

    // Device selection

#if 0
    // Get adapter
    UINT i = 0;
    IDXGIAdapter1* pAdapter;
    std::vector <IDXGIAdapter1*> vAdapters;
    while (pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        ++i;
    }

    // Uncomment to force skipping over Radeon RX 580 Series GPU
    // Purely a quick hack to prioritise secondary GPU

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
#else

#if USE_WARP
    pFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter));
#else
    pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdapter));
#endif


#endif

    result = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device) );

    if (FAILED(result))
    {
        OutputDebugStringA("Failed D3D12CreateDevice\n");
        exit(EXIT_FAILURE);
    }
    device->SetName(L"device");

    if (!dxgiAdapter)
    {
        LUID luid = device->GetAdapterLuid();
        result = pFactory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&dxgiAdapter));
    }

    if (FAILED(result))
    {
        OutputDebugStringA("Failed to enumerate Adapter from device LUID\n");
        exit(EXIT_FAILURE);
    }

    DXGI_ADAPTER_DESC3 desc;
    result = dxgiAdapter->GetDesc3(&desc);

    if (FAILED(result))
    {
        OutputDebugStringA("Failed to get adapter desc\n");
        exit(EXIT_FAILURE);
    }

    OutputDebugStringW(desc.Description);
    OutputDebugStringW(L"\n");

    {
        char msg[32];
        snprintf(msg, 32, "Node count == %d\n", device->GetNodeCount());
        OutputDebugStringA(msg);
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
            pOutput->Release();
        }
        pOutput = nullptr;

        // Ensure the window it located on the desktop identified by the above

        RECT rect;
        GetWindowRect(hWnd, &rect);
        MoveWindow(hWnd, display_rect.left, display_rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
    }



    DXGI_MODE_DESC1* best_fullscreen_mode = nullptr;

#if !USE_WARP  // Warp does not support querying output device

    assert(dxgiOutput);

    // Check hardware composition support
    {
        IDXGIOutput6* dxgiOutput6 = nullptr;
        DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAGS hardware_composition_support = {};

        result = dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput6));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to query dxgiOutput6 from dxgiOutput\n");
            exit(EXIT_FAILURE);
        }

        assert(dxgiOutput6);

        dxgiOutput->Release();
        dxgiOutput = nullptr;

        result = dxgiOutput6->CheckHardwareCompositionSupport((UINT*)&hardware_composition_support);
        

        if (FAILED(result))
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


        UINT supported = 0;
        result = dxgiOutput6->CheckOverlaySupport(DXGI_FORMAT_R8G8B8A8_UNORM, device, &supported);
        //result = dxgiOutput6->CheckOverlaySupport(DXGI_FORMAT_B8G8R8A8_UNORM, device, &supported);

        if (supported && DXGI_OVERLAY_SUPPORT_FLAG_DIRECT)
            OutputDebugStringA("DXGI_OVERLAY_SUPPORT_FLAG_DIRECT == Supported\n");

        if (supported && DXGI_OVERLAY_SUPPORT_FLAG_SCALING)
            OutputDebugStringA("DXGI_OVERLAY_SUPPORT_FLAG_SCALING  == Supported\n");

        if (dxgiOutput6->SupportsOverlays())
            OutputDebugStringA("Multi plane overlay (MPO) is supported\n");
        else
            OutputDebugStringA("Multi plane overlay (MPO) is NOT supported. This may affect the ability to enable tearing support\n");

        

        UINT flags = 0;
        UINT numModes = 0;

        // Consider available display modes. Prefer that which matches our current (ideally native) desktop dimensions

        dxgiOutput6->GetDisplayModeList1(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            flags,
            &numModes,
            nullptr
        );

        DXGI_MODE_DESC1* mode_descriptions = (DXGI_MODE_DESC1*)malloc(sizeof(DXGI_MODE_DESC1) * numModes);

        dxgiOutput6->GetDisplayModeList1(
            DXGI_FORMAT_R8G8B8A8_UNORM,
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

        dxgiOutput6->Release();
        dxgiOutput6 = nullptr;

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
#endif // USE_WARP


    // Determine available multisample quality levels
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels = {
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            .SampleCount = 32,
            .Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE
        };


#if MSAA_ENABLED
        for (UINT sampleCount = MAX_SAMPLE_COUNT, i = availableMultisampleLevels - 1;
            sampleCount > 0 && i >= 0;
            sampleCount = sampleCount >> 1, i--)
        {
            levels.SampleCount = sampleCount;

            result = device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels));

            if (FAILED(result))
            {
                OutputDebugStringA("Failed to query D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS\n");
                exit(EXIT_FAILURE);
            }

            // The valid range is between zero and one less than the level returned by CheckMultisampleQualityLevels
            sampleCountQuality[i] = levels.NumQualityLevels - 1;

            // If quality level is zero, it's unsupported. This will underflow to UINT_MAX            

            char msg[1024];
            snprintf(msg, 1024, "Multisample Count %d supports Max Quality %d\n", sampleCount, sampleCountQuality[i]);
            OutputDebugStringA(msg);
        }

        MSAA_Quality = sampleCountQuality[ceillog2(MSAA_Count)];

        // Zero - 1 (UINT_MAX) means not supported for this MSAA Count
        // The default sampler mode with no anti-aliasing has Count == 1 && Quality == 0
        assert(MSAA_Quality != UINT_MAX);

        // If MSAA_Count == 1, we need to disable MSAA entirely
        assert(MSAA_Count > 1);

        {
            char msg[1024];
            snprintf(msg, 1024, "Using Multisample Count %d : Quality %d\n", MSAA_Count, MSAA_Quality);
            OutputDebugStringA(msg);

        }
#endif


    }

    // Command queue creation
    {
        commandQueue = nullptr;

        const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL ,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE ,
            //.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT,
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
            //.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // DXGI_FORMAT_R8G8B8A8_UNORM,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,		// TODO: How to specify SRGB? 
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
             },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = backbufferFrames,								// Needs to be >= 2 for FLIP swap effect
            .Scaling = DXGI_SCALING_NONE,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,	// DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            .Flags = swapchain_flags,
        };


        if (best_fullscreen_mode)
        //if (0)
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

        //if (best_fullscreen_mode)
        if (0)
        {
            fullscreen_desc.RefreshRate = best_fullscreen_mode->RefreshRate;
            fullscreen_desc.Scaling = best_fullscreen_mode->Scaling;
            fullscreen_desc.ScanlineOrdering = best_fullscreen_mode->ScanlineOrdering;
        }

        IDXGISwapChain1 *swapchain1;
        // For Direct3D 12 this is a pointer to a direct command queue (refer to ID3D12CommandQueue).
        result = pFactory->CreateSwapChainForHwnd(commandQueue, hWnd, &swapchain_descriptor, &fullscreen_desc, nullptr, &swapchain1);

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateSwapChainForHwnd from dxgiFactory\n");
            exit(EXIT_FAILURE);
        }

        result = swapchain1->QueryInterface(IID_PPV_ARGS(&swapchain4));
        if (FAILED(result))
        {
            OutputDebugStringA("Failed to query for IDXGISwapChain4 from IDXGISwapChain1\n");
            exit(EXIT_FAILURE);
        }

        UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();
        

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateSwapChainForHwnd from dxgiFactory\n");
            exit(EXIT_FAILURE);
        }


        // Uncomment to prevent Alt+Enter from triggering a fullscreen switch
        //pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        dxgiAdapter->Release();
        dxgiAdapter = nullptr;

        pFactory->Release();
        pFactory = nullptr;


        assert(SUCCEEDED(result) && swapchain4 && device);
    
        // First obtain the framebuffer images from the swapchain


#if MSAA_ENABLED
        // Create the MSAA buffers
        {
            // We need a descriptor heap for the MSAA target view
            {
                rtvHeap_MSAA = nullptr;

                result = device->CreateDescriptorHeap(&rtvHeapDesc_MSAA, IID_PPV_ARGS(&rtvHeap_MSAA));

                if (FAILED(result))
                {
                    OutputDebugStringA("Failed to CreateDescriptorHeap\n");
                    exit(EXIT_FAILURE);
                }

                rtvHeap_MSAA->SetName(L"rtvHeap_MSAA");
            }

            // Create the MSAA buffers
            {
#if 0
                D3D12_HEAP_PROPERTIES heapProperties = {
                    .Type = D3D12_HEAP_TYPE_CUSTOM,
                    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
                    .MemoryPoolPreference = D3D12_MEMORY_POOL_L1,
                    .CreationNodeMask = 0,
                    .VisibleNodeMask = 0
                };

                D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_SHARED;
                //D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
                //D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;

#else
                D3D12_HEAP_PROPERTIES heapProperties = {
                    .Type = D3D12_HEAP_TYPE_DEFAULT,
                };

                D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
#endif



                RECT rect;
                GetClientRect(hWnd, &rect);
                UINT64 width = rect.right - rect.left;
                UINT height = rect.bottom - rect.top;


                D3D12_RESOURCE_DESC resourceDesc = {
                    .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                    .Alignment = 0,
                    .Width = width,
                    .Height = height,
                    .DepthOrArraySize = 1,
                    .MipLevels = 1,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .SampleDesc = {
                        .Count = MSAA_Count,
                        .Quality = MSAA_Quality
                     },
                    .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                    .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET // | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,            
                };

                D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RENDER_TARGET;
                //D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_PRESENT;

                D3D12_CLEAR_VALUE clearValue = {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .Color = { 0.2f, 0.2f, 0.7f, 1.0f }
                };

                for (uint8_t i = 0; i < backbufferFrames; i++)
                {

                    device->CreateCommittedResource1(
                        &heapProperties,
                        heapFlags,
                        &resourceDesc,
                        state,
                        &clearValue,
                        nullptr,
                        IID_PPV_ARGS(&framebuffer_MSAA[i]));

                    wchar_t name[32];
                    wsprintf(name, L"MSAA buffer %d of %d", i, backbufferFrames);
                    framebuffer_MSAA[i]->SetName(name);
                }
            }

            // Create render target views for each MSAA image buffer
            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_MSAA = rtvHeap_MSAA->GetCPUDescriptorHandleForHeapStart();
                const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc_MSAA.Type);

                // Create a renderTargetView for each swapchain frame
                for (uint8_t i = 0; i < backbufferFrames; i++)
                {
                    device->CreateRenderTargetView(framebuffer_MSAA[i], &rtvDesc_MSAA, rtvHandle_MSAA);

                    rtvHandles_MSAA[i] = rtvHandle_MSAA;
                    rtvHandle_MSAA.ptr += incrementSize;
                }

                D3D12_RESOURCE_DESC desc = framebuffer_MSAA[frameIndex]->GetDesc();
                D3D12_RESOURCE_DESC1 desc1 = framebuffer_MSAA[frameIndex]->GetDesc1();

                // We don't release here, we'll release when (and if) we need to resize the framebuffer_MSAA
                //framebuffer_MSAA->Release();
            }
        }
#endif

        // Now we can create the render target image view (pointing at the framebufer images already)

        // We need a descriptor heap for the render target view
        {
            rtvHeap = nullptr;
            result = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

            if (FAILED(result))
            {
                OutputDebugStringA("Failed to CreateDescriptorHeap\n");
                exit(EXIT_FAILURE);
            }

            rtvHeap->SetName(L"rtvHeap");
        }

        // Create render target views for each swapchain image
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
            const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

            // Create a renderTargetView for each swapchain frame
            for (uint8_t i = 0; i < backbufferFrames; i++)
            {
                result = swapchain4->GetBuffer(i, IID_PPV_ARGS(&framebuffer[i]));
                assert(SUCCEEDED(result));

                device->CreateRenderTargetView(framebuffer[i], &rtvDesc, rtvHandle);

                wchar_t name[32];
                wsprintf(name, L"Framebuffer %d of %d", i, backbufferFrames);
                framebuffer[i]->SetName(name);
                rtvHandles[i] = rtvHandle;
                rtvHandle.ptr += incrementSize;       
            }

            D3D12_RESOURCE_DESC desc = framebuffer[frameIndex]->GetDesc();
            D3D12_RESOURCE_DESC1 desc1 = framebuffer[frameIndex]->GetDesc1();

            // We don't release here, we'll release when (and if) we need to resize the framebuffer
            //framebuffer->Release();
        }

        // We need a fence to signal when the frame has rendered
        {
            device->CreateFence(fenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            fenceValues[frameIndex]++;

            // TODO: This is a windows native event
            // Ref: https://learn.microsoft.com/en-us/windows/win32/sync/using-event-objects

            // Create an event handle to use for frame synchronisation
            fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

            if (fenceEvent == nullptr)
            {
                OutputDebugStringA("Failed to create fence event");
                exit(EXIT_FAILURE);
            }            
        }

#if !ROOT_CONSTANTS_ENABLED
        // We also need a heap for our constant uniform buffer
        {
            cbvHeap = nullptr;

            result = device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap));
            cbvHeap->SetName(L"rtvDescriptorHeap");

            if (FAILED(result))
            {
                OutputDebugStringA("Failed to CreateDescriptorHeap\n");
                exit(EXIT_FAILURE);
            }

            cbvHeap->SetName(L"cbvHeap");
        }
#endif

    }

    // TODO: Consider AgilitySDK
    // TODO: Consider using Visual Studio compile time for shaders



    // TODO: Ref: https://learn.microsoft.com/en-us/windows/win32/direct3d12/porting-from-direct3d-11-to-direct3d-12
    // While there are numerous ways to set up your application, generally applications have one ID3D12CommandAllocator 
    // per swap-chain buffer. This allows the application to proceed to building up a set of commands for the next 
    // frame while the GPU renders the previous.


    // Create a command allocator
    for (int i=0; i < backbufferFrames; i++)
    {
        HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateCommandAllocator\n");
            exit(EXIT_FAILURE);
        }

        wchar_t name[32];        
        _snwprintf_s(name, 32, L"command allocator %d", i);
        commandAllocator[i]->SetName(name);
    }


#if RENDER_THREADS
    for (int i = 0; i < numThreads; i++)
    {
        for (int j = 0; j < backbufferFrames; j++)
        {
            HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_threadParameters[i].commandAllocator[j]));

            if (FAILED(result))
            {
                OutputDebugStringA("Failed to CreateCommandAllocator for render thread\n");
                exit(EXIT_FAILURE);
            }

            wchar_t name[64];
            _snwprintf_s(name, 64, L"command allocator render thread %d frameindex %d", i, j);
            m_threadParameters[i].commandAllocator[j]->SetName(name);
        }

    }
#endif


    // Pipeline state is setup later (during shader setup)

    // Create a command list
    {
        HRESULT result = device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandListPre));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateCommandList1\n");
            exit(EXIT_FAILURE);
        }
        commandListPre->SetName(L"commandListPre");



        result = device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandListPost));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateCommandList1\n");
            exit(EXIT_FAILURE);
        }
        commandListPost->SetName(L"commandListPost");


        // This command list is created in a closed state by default
    }

#if RENDER_THREADS
    for (int i=0; i < numThreads; i++)
    {
        HRESULT result = device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_threadParameters[i].commandList));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to CreateCommandList1\n");
            exit(EXIT_FAILURE);
        }

        wchar_t name[1024];        
        wsprintf(name, L"thread %d commandlist", i);
        m_threadParameters[i].commandList->SetName(name);

        // This command list is created in a closed state by default
    }
#endif
}


#if 1
void WaitForGPU(void)
{
    UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();

    commandQueue->Signal(fence, fenceValues[frameIndex]);

    fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent);
    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame
    fenceValues[frameIndex]++;
}

#else
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
#endif


void InitShaders(void)
{
    const char* shaderSource = R"(

        float2 pixelCoordToNCD(float2 pixel);
        struct vs_out vs_main(struct vs_in input);
        struct ps_out ps_main(struct ps_in input);

        cbuffer ConstantBuffer : register( b0 ) {
            float width;
            float height;
            float time;
            //float padding0;
            float padding1;
        }

        /* vertex attributes go here to input to the vertex shader */
        struct vs_in {
            uint vertexId : SV_VertexID;
        };

        /* outputs from vertex shader go here. can be interpolated to pixel shader */
        struct vs_out {
            float4 pos : SV_POSITION; // required output of VS
            float4 colour : COLOR0;
        };


        float2 pixelCoordToNCD(float2 pixel) {
            float2 ncd = ( (pixel / float2(width, height)) - float2(0.5, 0.5)) * 2;
            return ncd;
        }

        vs_out vs_main(vs_in input) {
          // TODO: What's the correct zero initialiser for hlsl 202x ?  {} and {0} do not work
          vs_out output = (vs_out)0; // zero the memory first

          // Center triangle

          if (input.vertexId == 0)
              output.pos = float4(0.5, -0.5, 0.5, 1.0);
          else if (input.vertexId == 1)
              output.pos = float4(-0.5, -0.5, 0.5, 1.0);
          else if (input.vertexId == 2)
              output.pos = float4(0.0, 0.5, 0.5, 1.0);


          // Top Left triangle

          if (input.vertexId == 3)
              output.pos = float4(-0.9, 0.9, 0.5, 1.0);
          else if (input.vertexId == 4)
              output.pos = float4(-0.8, 0.9, 0.5, 1.0);
          else if (input.vertexId == 5)
              output.pos = float4(-0.9, 0.8, 0.5, 1.0);


          // Top Right triangle

          if (input.vertexId == 6)
              output.pos = float4(0.9, 0.9, 0.5, 1.0);
          else if (input.vertexId == 7)
              output.pos = float4(0.9, 0.8, 0.5, 1.0);
          else if (input.vertexId == 8)
              output.pos = float4(0.8, 0.9, 0.5, 1.0);


          // Bottom Left triangle

          if (input.vertexId == 9)
              output.pos = float4(-0.9, -0.8, 0.5, 1.0);
          else if (input.vertexId == 10)
              output.pos = float4(-0.8, -0.9, 0.5, 1.0);
          else if (input.vertexId == 11)
              output.pos = float4(-0.9, -0.9, 0.5, 1.0);


          // Bottom Right triangle

          if (input.vertexId == 12)
              output.pos = float4(0.9, -0.8, 0.5, 1.0);
          else if (input.vertexId == 13)
              output.pos = float4(0.9, -0.9, 0.5, 1.0);
          else if (input.vertexId == 14)
              output.pos = float4(0.8, -0.9, 0.5, 1.0);


          // Now, using the frame dimensions, a solitary fixed dimension square 100x100 in the middle of the screen
          // The pixel dimensions need to be scaled against the NDC space

          float2 center = float2(width / 2, height / 2);

          float2 topleft     = pixelCoordToNCD(center + float2(-50, +50));
          float2 topright    = pixelCoordToNCD(center + float2(+50, +50));

          float2 bottomleft  = pixelCoordToNCD(center + float2(-50, -50));
          float2 bottomright = pixelCoordToNCD(center + float2(+50, -50));
          
          if (input.vertexId == 15)
              output.pos = float4(topleft, 0.5, 1.0);
          else if (input.vertexId == 16)
              output.pos = float4(topright, 0.5, 1.0);
          else if (input.vertexId == 17)
              output.pos = float4(bottomleft, 0.5, 1.0);

          else if (input.vertexId == 18)
              output.pos = float4(topright, 0.5, 1.0);
          else if (input.vertexId == 19)
              output.pos = float4(bottomright, 0.5, 1.0);
          else if (input.vertexId == 20)
              output.pos = float4(bottomleft, 0.5, 1.0);

          
          if (input.vertexId >= 15)
              //output.colour = float4(1.0, 1.0, 0.0, 1.0);
              output.colour = float4(1.0, 1.0, time, 1.0);
          else
              output.colour = clamp(output.pos, 0, 1);
          return output;
        }


        struct ps_in {
            float4 pos :  SV_POSITION;
            linear float4 colour : COLOR0;
        };

        struct ps_out {
            float4 colour : SV_TARGET;
        };

        ps_out ps_main(ps_in input) {
            ps_out output;
            output.colour = input.colour;
            //output.colour = float4(1.0, 1.0, 0.0, 1.0);
            return output;
        }
)";

     const DxcBuffer shaderSourceBuffer = {
        .Ptr = shaderSource,
        .Size = strlen(shaderSource),
        .Encoding = 0
     };


    HRESULT hr = 0;
    
    //IDxcLibrary* library = nullptr;
    IDxcCompiler3* compiler = nullptr;
    //IDxcBlobEncoding* sourceBlob = nullptr;

    IDxcUtils *utils = nullptr;

    /*
    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create Dxc library isntance\n");
        exit(EXIT_FAILURE);
    }
    */

    
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create Dxc compiler instance\n");
        exit(EXIT_FAILURE);
    }

    /*
    uint32_t codePage = CP_UTF8;
    
    //hr = library->CreateBlobFromFile(L"PS.hlsl", &codePage, &sourceBlob);    
    hr = library->CreateBlobWithEncodingOnHeapCopy(shaderSource, (uint32_t) strlen(shaderSource), codePage, &sourceBlob);

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create shader blob with encoding on heap copy\n");
        exit(EXIT_FAILURE);
    }
    */

    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create Dxc utils instance\n");
        exit(EXIT_FAILURE);
    }



    IDxcBlob* vs_code;
    IDxcBlob* ps_code;

    {
        IDxcOperationResult* result;
        IDxcCompilerArgs* args = nullptr;

        // Enable all warning flags for shaders
        // At a minimum:  -HV 202x -Wconversion -Wdouble-promotion -Whlsl-legacy-literal


        const wchar_t *arguments[] = {
            L"-HV", L"202x",
            L"-Weverything",
            //L"-Werror"

            L"-Wconversion",
            L"-Wdouble-promotion",
            L"-Whlsl-legacy-literal",

            L"-all-resources-bound" // D3DCOMPILE_ALL_RESOURCES_BOUND
        };


        hr = utils->BuildArguments(L"shader.hlsl",
            L"vs_main",
            L"vs_6_1",
            arguments, // LPCWSTR * pArguments,
            ARRAY_COUNT(arguments),       // UINT32           argCount,
            nullptr, // const DxcDefine * pDefines,
            0,       // UINT32           defineCount,
            &args
        );

        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to create VS build arguments\n");
            exit(EXIT_FAILURE);
        }

        hr = compiler->Compile(
            &shaderSourceBuffer,
            args->GetArguments(), args->GetCount(),
            nullptr,
            IID_PPV_ARGS(&result));

        if (SUCCEEDED(hr))
            result->GetStatus(&hr);

        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to compile vertex shader blob\n");

            if (result)
            {
                IDxcBlobEncoding* errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    char msg[2048];
                    snprintf(msg, 2048, "VS Compilation failed with errors:\n%hs\n", (const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA(msg);
                }
            }

            exit(EXIT_FAILURE);
        }
        else
        {
            if (result)
            {
                IDxcBlobEncoding* errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    char msg[2048];
                    snprintf(msg, 2048, "VS Compilation suceeded with output::\n%hs\n", (const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA(msg);
                    errorsBlob->Release();
                }
            }
        }

        result->GetResult(&vs_code);
        result->Release();
    }

    {
        //IDxcOperationResult* result;
        IDxcResult* result;
        IDxcCompilerArgs* args = nullptr;

        const wchar_t* arguments[] = {
            L"-HV", L"202x",
            L"-Weverything",
            //L"-Werror"

            L"-Wconversion",
            L"-Wdouble-promotion",
            L"-Whlsl-legacy-literal",

            L"-all-resources-bound" // D3DCOMPILE_ALL_RESOURCES_BOUND
        };

        /*
        const DxcDefine constants[] = {
            { L"D3DCOMPILE_ALL_RESOURCES_BOUND", L"1" }
        };
        */


        hr = utils->BuildArguments(L"shader.hlsl",
            L"ps_main",
            L"ps_6_1",
            arguments, // LPCWSTR * pArguments,
            ARRAY_COUNT(arguments),       // UINT32           argCount,
            nullptr, // const DxcDefine * pDefines,
            0,       // UINT32           defineCount,
            &args
        );

        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to create VS build arguments\n");
            exit(EXIT_FAILURE);
        }

        hr = compiler->Compile(
            &shaderSourceBuffer,
            args->GetArguments(), args->GetCount(),
            nullptr,
            IID_PPV_ARGS(&result));

        if (SUCCEEDED(hr))
            result->GetStatus(&hr);

        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to compile pixel shader blob\n");

            if (result)
            {
                IDxcBlobEncoding* errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    char msg[2048];
                    snprintf(msg, 2048, "PS Compilation failed with errors:\n%hs\n", (const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA(msg);
                }
            }

            exit(EXIT_FAILURE);
        }
        else
        {
            if (result)
            {
                IDxcBlobWide* output = nullptr;
                IDxcBlobWide* blobName = nullptr;

                // TODO: Doesn't seem to do anything
                result->GetOutput(result->PrimaryOutput(), IID_PPV_ARGS(&output), &blobName);
                
                IDxcBlobEncoding* errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    char msg[2048];
                    snprintf(msg, 2048, "VS Compilation suceeded with output::\n%hs\n", (const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA(msg);
                    errorsBlob->Release();
                }
            }
        }

        result->GetResult(&ps_code);
        result->Release();
    }

    utils->Release();
    compiler->Release();
    
    utils = nullptr;
    compiler = nullptr;


    // We need a root signature (defined globally)
    //ID3D12RootSignature *rootSig = nullptr;

    ID3DBlob* rootSigBlob = nullptr;

    ID3DBlob* errorsBlob = nullptr;



#if ROOT_CONSTANTS_ENABLED
    D3D12_ROOT_PARAMETER1 params[] = {
        {
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,            
            .Constants = {
                .ShaderRegister = 0,
                .RegisterSpace = 0,
                .Num32BitValues = 3
            },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
        }
    };
#else
    D3D12_ROOT_PARAMETER1 params[] = {
        {
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
            /*
            .Constants = {
                .ShaderRegister = 0,
                .RegisterSpace = 0,
                .Num32BitValues = 1
            },
            */
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
        }
        };
#endif

    //D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc = {
        .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
        .Desc_1_1 = {
            .NumParameters = ARRAY_COUNT(params),
            .pParameters = params,
            .NumStaticSamplers = 0,
            .pStaticSamplers = nullptr,
            .Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE
        }
    };


    //hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &rootSigBlob, &errorsBlob);

    hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &rootSigBlob, &errorsBlob);

    if (FAILED(hr))
    {
        char msg[2048];

        if (errorsBlob)
        {
            snprintf(msg, 2048, "Failed to deserialise root sig (hr = %x):\n%hs\n", hr, (const char*)errorsBlob->GetBufferPointer());
            OutputDebugStringA(msg);
            exit(EXIT_FAILURE);
        }

        snprintf(msg, 2048, "Failed to deserialise root sig (hr = %x): no error blob\n", hr);
        OutputDebugStringA(msg);
        exit(EXIT_FAILURE);
    }



    device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSig));
    
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendStates = {
        .SrcBlend = D3D12_BLEND_ONE,
        .DestBlend = D3D12_BLEND_ZERO,
        .BlendOp = D3D12_BLEND_OP_ADD,
        .SrcBlendAlpha = D3D12_BLEND_ONE,
        .DestBlendAlpha = D3D12_BLEND_ZERO,
        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
        .LogicOp = D3D12_LOGIC_OP_NOOP,
        .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
        .pRootSignature = rootSig,
        .VS = {
            .pShaderBytecode = vs_code->GetBufferPointer(),
            .BytecodeLength = vs_code->GetBufferSize()
            },
        .PS = {
            .pShaderBytecode = ps_code->GetBufferPointer(),
            .BytecodeLength = ps_code->GetBufferSize()
            },

        .BlendState = {
            .AlphaToCoverageEnable = false,
            .IndependentBlendEnable = false,
            .RenderTarget = { renderTargetBlendStates }
            },

        .SampleMask = UINT_MAX,

        .RasterizerState = {
            .FillMode = D3D12_FILL_MODE_SOLID,
            .CullMode = D3D12_CULL_MODE_BACK
            },
        .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        .NumRenderTargets = 1,
        .RTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
        .SampleDesc = {
#if MSAA_ENABLED
            .Count = MSAA_Count,
            .Quality = MSAA_Quality,
#else
            .Count = 1,
            .Quality = 0
#endif
            
            },
        .Flags = D3D12_PIPELINE_STATE_FLAG_NONE
    };
    

/*
#if defined( DEBUG ) || defined( _DEBUG )
    // Only valid on WARP devices
    .Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG
#else
    .Flags = 0
#endif
*/

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create pipeline state object\n");
        exit(EXIT_FAILURE);
    }


#if !ROOT_CONSTANTS_ENABLED
    // Create a constant buffer (for framebuffer dimensions)
    {
        ID3D12Resource *constantBuffer = nullptr;

        D3D12_HEAP_PROPERTIES heapProperties = {
            .Type = D3D12_HEAP_TYPE_UPLOAD
        };

        D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;

        D3D12_RESOURCE_DESC resourceDesc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = sizeof(VS_CONSTANT_BUFFER),
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0
                },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };


        hr = device->CreateCommittedResource1(
            &heapProperties,
            heapFlags,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            nullptr,
            IID_PPV_ARGS(&constantBuffer));

        assert(SUCCEEDED(hr));

        constantBuffer->SetName(L"constant buffer");


        constantBufferView = {
            .BufferLocation = constantBuffer->GetGPUVirtualAddress(),
            .SizeInBytes = (UINT) resourceDesc.Width
        };

        device->CreateConstantBufferView(&constantBufferView, cbvHeap->GetCPUDescriptorHandleForHeapStart());

        // Map and initialize the constant buffer.
        // We don't unmap this until the app closes.
        // Keeping things mapped for the lifetime of the resource is okay.
        // Ref: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloConstBuffers/D3D12HelloConstBuffers.cpp

        D3D12_RANGE readRange = { 0, 0 };
        
#if 1
        // Just keep it permanently mapped
        // TODO: Why is this ok?

        hr = constantBuffer->Map(0, &readRange, (void **)&persistentlyMappedConstantBuffer);
        memcpy(persistentlyMappedConstantBuffer, &VsConstData_dims, sizeof(VsConstData_dims));
#else
        uint8_t* destPtr;
        hr = constantBuffer->Map(0, &readRange, (void**)&destPtr);
        memcpy(destPtr, &VsConstData_dims, sizeof(VsConstData_dims));
        constantBuffer->Unmap(0, sizeof(VsConstData_dims));
#endif

    }
#endif
}


float time_lerp(void)
{
    // calculate a 't' value that will linearly interpolate from 0 to 1 and back every 20 seconds

    static DWORD m_startTime = GetTickCount();

    DWORD currentTime = GetTickCount();
    if (m_startTime == 0)
    {
        m_startTime = currentTime;
    }
    float t = 2 * ((currentTime - m_startTime) % 20000) / 20000.0f;
    if (t > 1.0f)
    {
        t = 2 - t;
    }
    return t;
}



void DrawLotsUnoptimised(ID3D12GraphicsCommandList* cmdList, UINT threadIndex, UINT numThreads)
{
    assert(threadIndex < numThreads);
    // We'll split this into numThreads batches and operate solely on the threadIndex batch for this iteration
    
    // Now loop and create some artificial load
    {
        const int viewports_x = 100;
        const int viewports_y = 100;

        const int total_viewports = viewports_x * viewports_y;
        const float per_batch = (float)total_viewports / numThreads;

        const int this_batch_begin = (int) (per_batch * threadIndex);
        const int this_batch_end = ((int)(per_batch * float(threadIndex + 1) )) - 1;

        //char msg[1024];
        //snprintf(msg, 1024, "threadId %d of %d threads. Total Viewports = %d. Per batch = %d.  Begin: %d  End: %d\n", threadIndex, numThreads, total_viewports, (int)per_batch, this_batch_begin, this_batch_end);
        //OutputDebugStringA(msg);

        float offset_x = (((float)window_width / viewports_x) * 0.5f);
        float offset_y = (((float)window_height / viewports_y) * 0.5f);

        for (int i = this_batch_begin; i <= this_batch_end; i++)
        {
            // Determine cell for this item
            UINT cell_x = i % viewports_x;
            UINT cell_y = i / viewports_x;

            //char msg[1024];
            //snprintf(msg, 1024, "viewport %d has cell x=%d  y=%d\n", i, cell_x, cell_y);
            //OutputDebugStringA(msg);

            {
                // Set a viewport for this block
                D3D12_VIEWPORT viewport1 = {
                    .TopLeftX = (float)window_width / viewports_x * cell_x,
                    .TopLeftY = (float)window_height / viewports_y * cell_y,
                    .Width = (float)window_width / viewports_x,
                    .Height = (float)window_height / viewports_y
                };

                cmdList->RSSetViewports(1, &viewport1);
                cmdList->DrawInstanced(21, 1, 0, 0);   // 7 tri's


                // An additional viewport, offset

                D3D12_VIEWPORT viewport2 = {
                    .TopLeftX = ((float)window_width / viewports_x * cell_x) + offset_x,
                    .TopLeftY = ((float)window_height / viewports_y * cell_y) + offset_y,
                    .Width = (float)window_width / viewports_x,
                    .Height = (float)window_height / viewports_y
                };

                cmdList->RSSetViewports(1, &viewport2);
                cmdList->DrawInstanced(21, 1, 0, 0);   // 7 tri's
            }

        }
    }
}


void InitCommandListForDraw(ID3D12GraphicsCommandList*);
void PopulateCommandList(ID3D12GraphicsCommandList*);


void render(void)
{
    UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();

#if MSAA_ENABLED
    D3D12_CPU_DESCRIPTOR_HANDLE* rtvTarget = &rtvHandles_MSAA[frameIndex];
#else
    D3D12_CPU_DESCRIPTOR_HANDLE* rtvTarget = &rtvHandles[frameIndex];
#endif

    // Now start rendering

    // TODO: Consider at least one *additional* commandAllocator
    // This may allow us to avoid 

    commandAllocator[frameIndex]->Reset();
    commandListPre->Reset(commandAllocator[frameIndex], pso);


#if DRAW_LOTS_UNOPTIMISED && RENDER_THREADS
    for (int i = 0; i < numThreads; i++)
    {
        m_threadParameters[i].commandAllocator[frameIndex]->Reset();
        m_threadParameters[i].commandList->Reset(m_threadParameters[i].commandAllocator[frameIndex], pso);
    }

    // Tell our render threads to kick into action
    // They'll use their own commandLists and we'll coalesce them later

    // You can execute command lists on any thread. Depending on the work 
    // load, apps can choose between using ExecuteCommandLists on one thread 
    // vs ExecuteCommandList from multiple threads.

    // We need to coalesce and execute BEFORE the MSAA resolve

    for (int i = 0; i < numThreads; i++)
        SetEvent(threadSignalBeginRenderFrame[i]);
#endif



#if MSAA_ENABLED

    // Indicate that the MSAA buffer will transition to a RENDER_TARGET state
    {
        // Create a resource barrier
        D3D12_RESOURCE_BARRIER resourceBarrierTransitionTargetPresent = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                .pResource = (ID3D12Resource*)framebuffer_MSAA[frameIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET
            }
        };

        commandListPre->ResourceBarrier(1, &resourceBarrierTransitionTargetPresent);
    }


#else
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

        commandListPre->ResourceBarrier(1, &resourceBarrierTransitionPresentTarget);
    }
#endif



    InitCommandListForDraw(commandListPre);

    // Set render targets
    {
        commandListPre->OMSetRenderTargets(1, rtvTarget, FALSE, nullptr);
    }

    // Clear the backbuffer entirely
    {
        const float clearColor1[4] = { 0.2f, 0.2f, 0.7f, 1.0f };
        commandListPre->ClearRenderTargetView(*rtvTarget, clearColor1, 0, nullptr);
    }

    // Draw something
    {
        // Clear a 200x200 pixel square at position 100x100
        {
            const FLOAT clearColor[4] = { 0.7f, 0.7f, 1.0f, 1.0f };

            // D3D12 appears to be top to bottom
            //RECT = {x1, y1, x2, y2}
            const D3D12_RECT rect = {
                .left = 100,
                .top = 100,
                .right = 300,
                .bottom = 300
            };

            commandListPre->ClearRenderTargetView(*rtvTarget, clearColor, 1, &rect);
        }

        {
            const FLOAT clearColor[4] = { 1.0f, 0.2f, 0.7f, 1.0f };
            D3D12_RECT rect;
            rect.left = std::min(600, (int32_t)window_width);
            rect.top = std::min(100, (int32_t)window_height);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Pink from top
            commandListPre->ClearRenderTargetView(*rtvTarget, clearColor, 1, &rect);
        }

        {
            const FLOAT clearColor2[4] = { 0.2f, 1.0f, 0.7f, 1.0f };
            D3D12_RECT rect;
            rect.left = std::min(600L, (LONG)window_width);
            rect.top = std::max(0L, (LONG)window_height - 600L);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Green from bottom
            commandListPre->ClearRenderTargetView(*rtvTarget, clearColor2, 1, &rect);
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
            commandListPre->ClearRenderTargetView(*rtvTarget, clearColor3, 1, &rect);
        }
    }



    //if (framechanged)
    if (1)
    {
        // Update constant data (frame dimensions, time)
        VsConstData_dims.width = (float)window_width;
        VsConstData_dims.height = (float)window_height;
        VsConstData_dims.time = time_lerp();
        framechanged = false;
    }


#if !ROOT_CONSTANTS_ENABLED
    //if (framechanged)
    if (1)
    {
        assert(persistentlyMappedConstantBuffer);
        memcpy(persistentlyMappedConstantBuffer, &VsConstData_dims, sizeof(VsConstData_dims));
    }
#endif


    // Populate the commandlist
    PopulateCommandList(commandListPre);

#if DRAW_LOTS_UNOPTIMISED && !RENDER_THREADS
     DrawLotsUnoptimised(commandListPre, 0, 1);
#endif

    commandListPre->Close();
    static ID3D12CommandList *commandListPreList[] = { commandListPre };
    commandQueue->ExecuteCommandLists(1, commandListPreList);



#if DRAW_LOTS_UNOPTIMISED && RENDER_THREADS

    // We need to coalesce and execute our render threads BEFORE the MSAA resolve
    // Wait for our render threads
    WaitForMultipleObjects(numThreads, threadSignalFinishRenderFrame, TRUE, INFINITE);

    // Our thread generated commandLists are ready to execute

    {
        ID3D12CommandList* ppCommandLists[numThreads] = {};
        for (int i = 0; i < numThreads; i++)
        {
            ppCommandLists[i] = m_threadParameters[i].commandList;

        }

        commandQueue->ExecuteCommandLists(numThreads, ppCommandLists);
    }
#endif

    // Drawing complete

    // Perform the resolve

    commandListPost->Reset(commandAllocator[frameIndex], pso);


#if MSAA_ENABLED

    // Transition MSAA buffer from TARGET to SOURCE


    // Indicate that the MSAA buffer will be used as a SOURCE for the resolve operation
    // Indicate that the swapchain back buffer will transition to a RESOLVE_DEST state
    {
        // Create a resource barrier
        D3D12_RESOURCE_BARRIER resourceBarrierTransitionMSAATargetToSource_Framebuffer_PresentToResolve[] = {

            // MSAA Target to Source
            {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = (ID3D12Resource*)framebuffer_MSAA[frameIndex],
                    .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                    .StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE
                }
            },
            // Swapchain buffer Present to ResolveDest
            {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    .pResource = (ID3D12Resource*)framebuffer[frameIndex],
                    .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                    .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                    .StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST
                }
            }
        };

        commandListPost->ResourceBarrier(2, &resourceBarrierTransitionMSAATargetToSource_Framebuffer_PresentToResolve[0]);
    }



    commandListPost->ResolveSubresource(framebuffer[frameIndex], 0, framebuffer_MSAA[frameIndex], 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

    // Now set the actual swapchain render target
    commandListPost->OMSetRenderTargets(1, rtvTarget, FALSE, nullptr);


    // Indicate that the back buffer will be used to present
    {
        // Create a resource barrier
        D3D12_RESOURCE_BARRIER resourceBarrierTransitionTargetPresent = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                .pResource = (ID3D12Resource*)framebuffer[frameIndex],
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST,
                .StateAfter = D3D12_RESOURCE_STATE_PRESENT
            }
        };

        commandListPost->ResourceBarrier(1, &resourceBarrierTransitionTargetPresent);
    }


#else

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

        commandListPost->ResourceBarrier(1, &resourceBarrierTransitionTargetPresent);
    }

#endif


    commandListPost->Close();

    {
        ID3D12CommandList* ppCommandLists[] = { commandListPost };
        commandQueue->ExecuteCommandLists(ARRAY_COUNT(ppCommandLists), ppCommandLists);
    }

    const UINT vsync = 0;
    const UINT presentFlags = (allowTearing && !DXGI_fullscreen && vsync == 0) ? DXGI_PRESENT_ALLOW_TEARING : 0;
    //const UINT presentFlags = 0;


    // The DXGI_PRESENT_ALLOW_TEARING flag cannot be used in an application that is currently in full screen exclusive 
    // mode (set by calling SetFullscreenState(TRUE)). It can only be used in windowed mode. 
    // To use this flag in full screen Win32 apps, the application should present to a fullscreen borderless window 
    // and disable automatic ALT+ENTER fullscreen switching using IDXGIFactory::MakeWindowAssociation. 


    const DXGI_PRESENT_PARAMETERS presentParameters = {};

    swapchain4->Present1(vsync, presentFlags, &presentParameters);

    //WaitForPreviousFrame();
    WaitForGPU();
}



void InitCommandListForDraw(ID3D12GraphicsCommandList* commandList)
{
    // We need to define state for individual command lists

    // Set the root signature before doing anything
    commandList->SetGraphicsRootSignature(rootSig);

#if ROOT_CONSTANTS_ENABLED
    commandList->SetGraphicsRoot32BitConstants(0, 3, &VsConstData_dims, 0);
#else

    // Set descriptor heaps
    {
        ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap };
        commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    }
#endif


    D3D12_VIEWPORT viewport = {
        0.0f, 0.0f,   // X, Y
        (FLOAT)window_width,
        (FLOAT)window_height,
        0.0f, 1.0f  // DepthMin, DepthMax
    };
    D3D12_RECT scissorRect = { 0, 0, (LONG)window_width, (LONG)window_height };

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    commandList->SetPipelineState(pso);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    /*
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
    pCommandList->OMSetStencilRef(0);
    */
}

void PopulateCommandList(ID3D12GraphicsCommandList *commandList)
{
    // Draw something, we don't use vertex buffers - all the magic happens in the vertex shader
    commandList->DrawInstanced(21, 1, 0, 0);
}




#if RENDER_THREADS

static unsigned int WINAPI RenderThread(LPVOID lpParameter)
{
    ThreadParameter* parameter = (ThreadParameter*)lpParameter;
    int threadIndex = parameter->threadIndex;
    
    assert(threadIndex >= 0);
    assert(threadIndex < numThreads);

    while (threadIndex >= 0 && threadIndex < numThreads)
    {
        // Wait for the main thread to signal us

        WaitForSingleObject(threadSignalBeginRenderFrame[threadIndex], INFINITE);

        // Turns out we need to setup the root sig + render targets for EACH command list
        // Even if the preceding command list has already done so for this frame
        // Very annoying

        InitCommandListForDraw(parameter->commandList);

        // Set render targets
        {
            UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();

#if MSAA_ENABLED
            D3D12_CPU_DESCRIPTOR_HANDLE* rtvTarget = &rtvHandles_MSAA[frameIndex];
#else
            D3D12_CPU_DESCRIPTOR_HANDLE* rtvTarget = &rtvHandles[frameIndex];
#endif

            parameter->commandList->OMSetRenderTargets(1, rtvTarget, FALSE, nullptr);
        }


        {
            // Ref: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12pipelinestate

            static bool pso_changed = true;
            if (pso_changed)
            {
                parameter->commandList->SetPipelineState(pso);
                pso_changed = false;
            }

            parameter->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }


        D3D12_RECT scissorRect = { 0, 0, (LONG)window_width, (LONG)window_height };
        parameter->commandList->RSSetScissorRects(1, &scissorRect);

        // Do some stuff
        // Write to our own commandList
        DrawLotsUnoptimised(parameter->commandList, threadIndex, numThreads);
        // Close the command list

        parameter->commandList->Close();

        // Tell the main thread we're done
        SetEvent(threadSignalFinishRenderFrame[threadIndex]);
    }

    return 0;
}
#endif

void InitThreads()
{
#if RENDER_THREADS
    for (int i = 0; i < numThreads; i++)
    {
        // Initialise event handles

        threadSignalBeginRenderFrame[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        threadSignalFinishRenderFrame[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        assert(threadSignalBeginRenderFrame[i]);
        assert(threadSignalFinishRenderFrame[i]);

        m_threadParameters[i].threadIndex = i;

        threadHandles[i] = (HANDLE)_beginthreadex(nullptr, 0, &RenderThread, (LPVOID) &m_threadParameters[i], 0, nullptr);

        assert(threadHandles[i]);
    }
#endif
}

DWORD windowStyle = WS_OVERLAPPEDWINDOW;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, windowStyle,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   InitD3D12();
   InitShaders();
   InitThreads();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


static bool windowed_fullscreen = false;

// Convert a styled window into a fullscreen borderless window and back again.
void ToggleFullscreenWindow(void)
{
    static RECT windowRect = {};

    if (DXGI_fullscreen)
    {
        // If we're already in FSE, abort
        // We should only toggle windowed fullscreen if in windowed mode
        return;
    }

    if (windowed_fullscreen)
    {
        // Restore the window's attributes and size.
        SetWindowLong(hWnd, GWL_STYLE, windowStyle);

        SetWindowPos(
            hWnd,
            HWND_NOTOPMOST,
            windowRect.left,
            windowRect.top,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hWnd, SW_NORMAL);
    }
    else
    {
        // Save the old window rect so we can restore it when exiting fullscreen mode.
        GetWindowRect(hWnd, &windowRect);

        // Make the window borderless so that the client area can fill the screen.
        SetWindowLong(hWnd, GWL_STYLE, windowStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

        // Get the settings of the primary display. We want the app to go into
        // fullscreen mode on the display that supports Independent Flip.
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

        SetWindowPos(
            hWnd,
            HWND_TOPMOST,
            devMode.dmPosition.x,
            devMode.dmPosition.y,
            devMode.dmPosition.x + devMode.dmPelsWidth,
            devMode.dmPosition.y + devMode.dmPelsHeight,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(hWnd, SW_MAXIMIZE);
    }

    windowed_fullscreen = !windowed_fullscreen;
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

                if (windowed_fullscreen)
                {
                    // We're already in windowed fullscreen mode, abort
                    // We only support switching from a non-fullscreen window
                    return 0;
                }

                DXGI_fullscreen = !DXGI_fullscreen;

#if 0
                IDXGIOutput* output;
                swapchain4->GetContainingOutput(&output);

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
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
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
            }
            else if (wParam == 'f')
            {
                // We'll toggle between "windowed fullscreen" and regular "windowed"
                // As opposed to FSE

                ToggleFullscreenWindow();
            }
            return 0;

        } break;

    case WM_SIZE:
        {
            char msg[1024];
            snprintf(msg, 1024, "New WM_SIZE event: %d x %d\n", LOWORD(lParam), HIWORD(lParam));

            OutputDebugStringA(msg);

            framechanged = true;

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



            DXGI_OUTPUT_DESC output_desc;

            IDXGIOutput* output;
            HRESULT hr = swapchain4->GetContainingOutput(&output);
            if (SUCCEEDED(hr))
            {
                output->GetDesc(&output_desc);
                output->Release();
                output = nullptr;
            }
            else
            {
                // This will fail if the window resides on a display for a different device
                OutputDebugStringA("Failed to retrieve containing output, window may have moved to a different display/interface? Or using WARP adapter\n");

                // We'll hack up our own output_desc

                HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

                assert(monitor);

                MONITORINFO info = {
                    .cbSize = sizeof(MONITORINFO)
                };

                if (!GetMonitorInfo(monitor, &info))
                {
                    OutputDebugStringA("Failed to GetMonitorInfo\n");
                    exit(EXIT_FAILURE);
                }

                output_desc.DesktopCoordinates = info.rcWork;
            }


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


            UINT frameIndex = swapchain4->GetCurrentBackBufferIndex();

            hr = swapchain4->ResizeTarget(&target_mode);

            assert(SUCCEEDED(hr));

            // Reset the command list
            //flushGpu();
            //WaitForPreviousFrame();

            WaitForGPU();


#if MSAA_ENABLED
            // Resize MSAA buffers
            {

                if (!window_width || !window_height)
                    return 0;

                // Release the existing MSAA resources

                for (UINT i = 0; i < backbufferFrames; i++)
                {
                    framebuffer_MSAA[i]->Release();
                    framebuffer_MSAA[i] = nullptr;
                }

                D3D12_HEAP_PROPERTIES heapProperties = {
                    .Type = D3D12_HEAP_TYPE_DEFAULT,
                };

                D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;

                D3D12_RESOURCE_DESC resourceDesc = {
                    .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                    .Alignment = 0,
                    .Width = window_width,
                    .Height = window_height,
                    .DepthOrArraySize = 1,
                    .MipLevels = 1,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .SampleDesc = {
                        .Count = MSAA_Count,
                        .Quality = MSAA_Quality
                     },
                    .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                    .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET // | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,            
                };

                D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
                //D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_PRESENT;

                D3D12_CLEAR_VALUE clearValue = {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .Color = { 0.2f, 0.2f, 0.7f, 1.0f }
                };

                for (uint8_t i = 0; i < backbufferFrames; i++)
                {

                    device->CreateCommittedResource1(
                        &heapProperties,
                        heapFlags,
                        &resourceDesc,
                        state,
                        &clearValue,
                        nullptr,
                        IID_PPV_ARGS(&framebuffer_MSAA[i]));

                    wchar_t name[32];
                    wsprintf(name, L"MSAA buffer %d of %d", i, backbufferFrames);
                    framebuffer_MSAA[i]->SetName(name);
                }


                // Create render target views for each MSAA image buffer
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_MSAA = rtvHeap_MSAA->GetCPUDescriptorHandleForHeapStart();
                    const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc_MSAA.Type);

                    // Create a renderTargetView for each swapchain frame
                    for (uint8_t i = 0; i < backbufferFrames; i++)
                    {
                        device->CreateRenderTargetView(framebuffer_MSAA[i], &rtvDesc_MSAA, rtvHandle_MSAA);

                        rtvHandles_MSAA[i] = rtvHandle_MSAA;
                        rtvHandle_MSAA.ptr += incrementSize;
                    }

                    D3D12_RESOURCE_DESC desc = framebuffer_MSAA[frameIndex]->GetDesc();
                    D3D12_RESOURCE_DESC1 desc1 = framebuffer_MSAA[frameIndex]->GetDesc1();

                    // We don't release here, we'll release when (and if) we need to resize the framebuffer_MSAA
                    //framebuffer_MSAA->Release();
                }


            }
#endif


            // Release the resources holding references to the swap chain (required by ResizeBufers)
            // and reset the frame fence values to the current fence value

            for (UINT i = 0; i < backbufferFrames; i++)
            {
                //framebuffer[i].Reset();  // <-- relies on ComPtr
                framebuffer[i]->Release();
                framebuffer[i] = nullptr;
                
                fenceValues[i] = fenceValues[frameIndex]; // ??
            }

            commandAllocator[frameIndex]->Reset();
            commandListPre->Reset(commandAllocator[frameIndex], nullptr);

            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.

            swapchain4->ResizeBuffers(backbufferFrames, window_width, window_height, DXGI_FORMAT_UNKNOWN, swapchain_flags);

            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to resize swapchain buffer\n");
                exit(EXIT_FAILURE);
            }

            frameIndex = swapchain4->GetCurrentBackBufferIndex();

            // Get buffer and create a render-target-view.
            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
                const UINT incrementSize = device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

                for (uint8_t i = 0; i < backbufferFrames; i++)
                {
                    hr = swapchain4->GetBuffer(i, IID_PPV_ARGS(&framebuffer[i]));

                    if (FAILED(hr))
                    {
                        OutputDebugStringA("Failed to retrieve swapchain buffer\n");
                        exit(EXIT_FAILURE);
                    }

                    D3D12_RESOURCE_DESC1 desc1 = framebuffer[i]->GetDesc1();

                    //char msg[1024];
                    snprintf(msg, 1024, "Swapchain buffer[%u] size : %" PRIu64 " x %u\n", i, desc1.Width, desc1.Height);
                    OutputDebugStringA(msg);
                    
                    device->CreateRenderTargetView(framebuffer[i], &rtvDesc, rtvHandle);

                    if (FAILED(hr))
                    {
                        OutputDebugStringA("Failed to create render target view\n");
                        exit(EXIT_FAILURE);
                    }

                    wchar_t name[32];
                    wsprintf(name, L"Framebuffer %d of %d", i, backbufferFrames);
                    framebuffer[i]->SetName(name);
                    rtvHandles[i] = rtvHandle;
                    rtvHandle.ptr += incrementSize;
                }
            }

            // Set render targets
#if MSAA_ENABLED
            commandListPre->OMSetRenderTargets(1, &rtvHandles_MSAA[frameIndex], FALSE, nullptr);
#else
            commandListPre->OMSetRenderTargets(1, &rtvHandles[frameIndex], FALSE, nullptr);
#endif

            // Set up the viewport.

            D3D12_VIEWPORT viewport = {
                0.0f, 0.0f,   // X, Y
                (FLOAT)window_width,
                (FLOAT)window_height,
                0.0f, 1.0f  // DepthMin, DepthMax
            };

            D3D12_RECT scissorRect = { 0, 0, (LONG) window_width, (LONG) window_height };

            commandListPre->RSSetViewports(1, &viewport);
            commandListPre->RSSetScissorRects(1, &scissorRect);
            commandListPre->Close();

            ID3D12CommandList* ppCommandLists[] = { commandListPre };
            commandQueue->ExecuteCommandLists(1, ppCommandLists);
            
            WaitForGPU();
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
        {
            // Cleanup

            WaitForGPU();

#if MSAA_ENABLED
            for (UINT i = 0; i < backbufferFrames; i++)
            {
                framebuffer_MSAA[i]->Release();
                framebuffer_MSAA[i] = nullptr;
            }
#endif

#if RENDER_THREADS
            for (UINT i = 0; i < numThreads; i++)
            {
                m_threadParameters[i].commandList->Release();
                m_threadParameters[i].commandList = nullptr;

                for (UINT j = 0; j < backbufferFrames; j++)
                {
                    m_threadParameters[i].commandAllocator[j]->Release();
                    m_threadParameters[i].commandAllocator[j] = nullptr;
                }
            }
#endif


            for (UINT i = 0; i < backbufferFrames; i++)
            {
                framebuffer[i]->Release();
                framebuffer[i] = nullptr;
            }

            commandListPre->Release();
            commandListPost->Release();

            for (UINT i = 0; i < backbufferFrames; i++)
                commandAllocator[i]->Release();

            fence->Release();
            commandQueue->Release();

            pso->Release();
            pso = nullptr;

            swapchain4->Release();
            swapchain4 = nullptr;

#ifndef NDEBUG
            ID3D12InfoQueue* info;
            device->QueryInterface(IID_PPV_ARGS(&info));
            info->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
            info->SetBreakOnCategory(D3D12_MESSAGE_CATEGORY_STATE_CREATION, FALSE);
            info->Release();
            info = nullptr;

            IDXGIInfoQueue* infoqueue;
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&infoqueue));

            infoqueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, FALSE);
            infoqueue->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION, FALSE);
            infoqueue->Release();
            infoqueue = nullptr;
#endif

            device->Release();
            device = nullptr;

            dxgi_debug_report();
            PostQuitMessage(0);
            break;
        }

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
