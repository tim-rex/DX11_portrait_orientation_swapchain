// DX11_fullscreen_portrait_repro.cpp : Defines the entry point for the application.
//

// D3D11
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler
#pragma comment( lib, "dxguid.lib" )


#include "framework.h"
#include "DX11_fullscreen_portrait_repro.h"

#include <d3d11.h>       // D3D interface
#include <d3d11_1.h>     // D3D 11.1 extensions    TODO: Should we use 1_4 ? 1_5? 1_6?
#include <d3d11_4.h>     // ID3D11Device5
#include <dxgi1_6.h>     // CheckHardwareCompositionSupport

#include "d3dcompiler.h"

#include <dxgidebug.h>   // DXGI_INFO_QUEUE
#include <assert.h>
#include <vector>


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
ID3D11Device5* device = nullptr;
ID3D11DeviceContext1* device_context_11_x = nullptr;
IDXGISwapChain1* swapchain = nullptr;
ID3D11RenderTargetView* render_target_view = nullptr;
ID3D11RenderTargetView* msaa_render_target_view = nullptr;


D3D_FEATURE_LEVEL feature_level;

UINT window_width = 0;
UINT window_height = 0;
BOOL DXGI_fullscreen = false;
BOOL allowTearing = false;

BOOL framechanged = false;


#define MSAA_ENABLED 1
#define DRAW_LOTS_UNOPTIMISED 1


#if MSAA_ENABLED
ID3D11Texture2D* msaa_render_target;

constexpr unsigned floorlog2(unsigned x)
{
    return x == 1 ? 0 : 1 + floorlog2(x >> 1);
}

constexpr unsigned ceillog2(unsigned x)
{
    return x == 1 ? 0 : floorlog2(x - 1) + 1;
}

constexpr int availableMultisampleLevels = ceillog2(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT) + 1;

UINT sampleCountQuality[availableMultisampleLevels];
UINT MSAA_Count = 4;
UINT MSAA_Quality = 0;
#endif



ID3D11Buffer* shaderConstantBuffer_dims = NULL;

// Define the constant data used to communicate with shaders.
struct VS_CONSTANT_BUFFER
{
    float width;
    float height;
    float time;    // Buffer must be a multiple of 16 bytes
    float _padding1;
} VS_CONSTANT_BUFFER;

struct VS_CONSTANT_BUFFER VsConstData_dims = {};


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

void dxgi_debug_init()
{
    assert(device);
    

#ifndef NDEBUG

    // Debug
    {

        // OMFG.. This was hidden deep.. except, I already have that. FUCK.
        // Flat out DOES NOT WORK
        // //
        // Maybe because the errors are occuring within a DLL? Do we need to configure the info queue within the DLL also?

        // 
        //  Note
        //  For Windows 10, to create a device that supports the debug layer, enable the "Graphics Tools" optional feature.Go to the Settings panel, 
        //  under System, Apps& features, Manage optional Features, Add a feature, and then look for "Graphics Tools".


        // for debug builds enable VERY USEFUL debug break on API errors
        {
            ID3D11InfoQueue* info;
            device->QueryInterface(IID_PPV_ARGS(&info));
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
            //info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, TRUE);

            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_INITIALIZATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_CLEANUP, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_COMPILATION, TRUE);
            //info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_CREATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_SETTING, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_GETTING, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_SHADER, TRUE);

            //info->AddMessage(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, D3D11_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST MISCELLANOUS ERROR");
            //info->AddApplicationMessage(D3D11_MESSAGE_SEVERITY_ERROR, "TEST");
            //info->AddMessage(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, D3D11_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST");


            info->Release();
            info = nullptr;
        }

        // enable debug break for DXGI too
        {
            IDXGIInfoQueue* info;
            DXGIGetDebugInterface1(0, IID_PPV_ARGS(&info));

            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            //info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO, TRUE);
            info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE, TRUE);

            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION, TRUE);
            //info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER, TRUE);


            // NOTE: ApplicationMessage will not let us break
            //OutputDebugStringA("This will not trigger??? Why?\n");
            //info->AddApplicationMessage(DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, "TEST");

            // NOTE: Result message will let us break
            //info->AddMessage(DXGI_DEBUG_DXGI, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST");

            //info->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

            info->Release();
            info = nullptr;


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

}


void InitD3D11(void)
{
    HRESULT result;
    IDXGIAdapter4 *dxgiAdapter = nullptr;

    D3D_FEATURE_LEVEL feature_level_req[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    UINT device_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if defined( DEBUG ) || defined( _DEBUG )
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    UINT factory_flags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    factory_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    IDXGIFactory2* pFactory2 = nullptr;
    //result = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
    result = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&pFactory2));

    if (FAILED(result))
    {
        exit(EXIT_FAILURE);
    }

    IDXGIFactory7* pFactory = nullptr;
    pFactory2->QueryInterface(IID_PPV_ARGS(&pFactory));

    if (FAILED(result))
    {
        exit(EXIT_FAILURE);
    }

    pFactory2->Release();
    pFactory2 = nullptr;

    // Feature support

    // Usage of CheckFeatureSupport is poorly documented, but this examples shows the way
    // Ref: https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays

    pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));


    // Device selection
    D3D_DRIVER_TYPE d3d_driver_type = D3D_DRIVER_TYPE_HARDWARE;


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

        // TODO: Use proper method to obtain adapter4
        dxgiAdapter = (IDXGIAdapter4*)adapter;
        d3d_driver_type = D3D_DRIVER_TYPE_UNKNOWN;
        break;
    }

    for (auto adapter : vAdapters)
    {
        adapter->Release();
        adapter = nullptr;
    }
    vAdapters.clear();
#else
    // Get the default device
    pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdapter));
    d3d_driver_type = D3D_DRIVER_TYPE_UNKNOWN;
#endif


    // Temporary context, we'll replace with 11_1
    {
        ID3D11Device* device_11_0 = nullptr;
        ID3D11DeviceContext* device_context_11_0 = nullptr;

        result = D3D11CreateDevice(
            dxgiAdapter,
            d3d_driver_type,
            nullptr,
            device_flags,
            &feature_level_req[0],
            ARRAY_COUNT(feature_level_req),
            D3D11_SDK_VERSION,
            &device_11_0,
            &feature_level,
            &device_context_11_0
        );

        if (result == E_INVALIDARG)
        {
            OutputDebugStringA("Direct3D 11_1 device not available, trying again for 11_0\n");

            // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdeviceandswapchain
            //    If you provide a D3D_FEATURE_LEVEL array that contains D3D_FEATURE_LEVEL_11_1 on a computer 
            //    that doesn't have the Direct3D 11.1 runtime installed, this function immediately fails with 
            //    E_INVALIDARG.

            result = D3D11CreateDevice(
                dxgiAdapter,
                d3d_driver_type,
                nullptr,
                device_flags,
                &feature_level_req[1],
                ARRAY_COUNT(feature_level_req) - 1,
                D3D11_SDK_VERSION,
                &device_11_0,
                &feature_level,
                &device_context_11_0);
        }

        if (FAILED(result))
        {
            OutputDebugStringA("Failed D3D11CreateDevice\n");
            exit(EXIT_FAILURE);
        }


        result = device_11_0->QueryInterface(IID_PPV_ARGS(&device));
        if (FAILED(result))
        {
            OutputDebugStringA("Failed to QueryInterface for ID3D11Device5 from ID3D11Device\n");
            exit(EXIT_FAILURE);
        }

        result = device_context_11_0->QueryInterface(IID_PPV_ARGS(&device_context_11_x));

        if (FAILED(result))
        {
            OutputDebugStringA("Failed to QueryInterface for ID3D11DeviceContext1 from ID3D11DeviceContext\n");
            exit(EXIT_FAILURE);
        }


        device_context_11_0->Release();
        device_11_0->Release();

        device_context_11_0 = nullptr;
        device_11_0 = nullptr;
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


    dxgi_debug_init();


    dxgi_debug_report();



    IDXGIOutput* dxgiOutput = nullptr;


    // Get the display output description, we'll identify the window coordinates of the display and move our window to it
    {
        UINT i = 0;
        IDXGIOutput* pOutput;

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


    assert(dxgiOutput);

    DXGI_MODE_DESC1 *best_fullscreen_mode = nullptr;

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


#if MSAA_ENABLED
    // [35328] D3D11 ERROR : ID3D11Device::CreateTexture2D : SampleDesc.Quality specifies invalid value 1. 
    // For 4 samples, the current graphics implementation only supports 
    // SampleDesc.Quality less than 1. < -- if 0 is shown here for Quality, this means the graphics 
    // implementation doesn't support 4 samples at all. 
    // Use CheckMultisampleQualityLevels to detect what the graphics implementation supports.

    for (UINT sampleCount = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT, i = availableMultisampleLevels-1; 
        sampleCount > 0 && i >= 0;
        sampleCount = sampleCount >> 1, i--)
    {
        HRESULT hr = device->CheckMultisampleQualityLevels1(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, sampleCount, 0, &sampleCountQuality[i]);
        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to determine Multisample quality level");
            exit(EXIT_FAILURE);
        }

        // The valid range is between zero and one less than the level returned by CheckMultisampleQualityLevels
        sampleCountQuality[i]--;

        // If quality level is zero, it's unsupported. This will underflow to UINT_MAX

        
        char msg[1024];
        snprintf(msg, 1024, "Multisample Count %d supports Max Quality %d\n", sampleCount, sampleCountQuality[i]);
        OutputDebugStringA(msg);
    }

    //MSAA_Count = 4;
    MSAA_Quality = sampleCountQuality[ceillog2(MSAA_Count)];

    // Zero - 1 (UINT_MAX) means not supported for this MSAA Count
    // The default sampler mode with no anti-aliasing has Count == 1 && Quality == 0
    assert(MSAA_Quality != UINT_MAX);

    //assert(MSAA_Quality > 0 || (MSAA_Count == 1 && MSAA_Quality == 0 ));

    // If MSAA_Count == 1, we should just disable MSAA entirely and avoid the resolve
    assert(MSAA_Count > 1);


    {
        char msg[1024];
        snprintf(msg, 1024, "Using Multisample Count %d : Quality %d\n", MSAA_Count, MSAA_Quality);
        OutputDebugStringA(msg);

    }
#endif



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
            .BufferCount = 2,								// Needs to be >= 2 for FLIP swap effect
            .Scaling = DXGI_SCALING_NONE,
            //.Scaling = DXGI_SCALING_STRETCH,
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


        result = pFactory->CreateSwapChainForHwnd(device, hWnd, &swapchain_descriptor, &fullscreen_desc, nullptr, &swapchain);

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


        assert(S_OK == result && swapchain && device && device_context_11_x);

        OutputDebugStringA("Direct3D 11 device context created\n");

        char msg[1024];
        snprintf(msg, 1024, "            Feature level 0x%x\n", feature_level);
        OutputDebugStringA(msg);



#if MSAA_ENABLED

        // Create MSAA texture
        {
            RECT rect;
            GetClientRect(hWnd, &rect);


            UINT samples = 1;
            UINT quality = 0;
            device->CheckMultisampleQualityLevels1(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, samples, 0, &quality);



            D3D11_TEXTURE2D_DESC desc = {
                .Width = (UINT) rect.right - rect.left,
                .Height = (UINT) rect.bottom - rect.top,
                .MipLevels = 1,
                .ArraySize = 1,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                .SampleDesc = {
                    .Count = MSAA_Count,
                    .Quality = MSAA_Quality
                    },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_RENDER_TARGET,
                .CPUAccessFlags = 0,
                .MiscFlags = 0
            };



            result = device->CreateTexture2D(&desc, nullptr, &msaa_render_target);

            if (FAILED(result))
            {
                OutputDebugStringA("Failed to CreateTexture2D for MSAA texture\n");
                exit(EXIT_FAILURE);
            }


            // Now we can create the render target image view (pointing at the framebufer images already)

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS,
            };

            result = device->CreateRenderTargetView(msaa_render_target, &rtvDesc, &msaa_render_target_view);
            assert(SUCCEEDED(result));
        }
#endif

        // Setup the swapchain render target view
        {
            // First obtain the framebuffer from the swapchain

            ID3D11Texture2D* swapchain_framebuffer;

            result = swapchain->GetBuffer(0, IID_PPV_ARGS(&swapchain_framebuffer));
            assert(SUCCEEDED(result));

            // Now we can create the render target image view (pointing at the framebufer images already)

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
            };

            result = device->CreateRenderTargetView(swapchain_framebuffer, &rtvDesc, &render_target_view);
            assert(SUCCEEDED(result));

            D3D11_TEXTURE2D_DESC framebufferSurfaceDesc;
            swapchain_framebuffer->GetDesc(&framebufferSurfaceDesc);

            swapchain_framebuffer->Release();
            swapchain_framebuffer = nullptr;
        }

    }
}


//ID3D11InputLayout* input_layout_ptr = NULL;


void InitShaders(void)
{
    const char* shaderSource = R"(

        cbuffer ConstantBuffer : register( b0 ) {
            float width;
            float height;
            float time;
            float padding1;
        };

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


    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ID3DBlob *vs_blob_ptr = nullptr;
    ID3DBlob *ps_blob_ptr = nullptr;
    ID3DBlob *error_blob = nullptr;

    HRESULT hr = 0;
    hr = D3DCompile(shaderSource, strlen(shaderSource), "basic vertex shader", nullptr, nullptr,
        "vs_main",
        "vs_5_0",
        flags, 
        0,
        &vs_blob_ptr,
        &error_blob);

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to compile vertex shader\n");
        if (error_blob)
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
        exit(EXIT_FAILURE);
    }

    if (error_blob) error_blob->Release();
    error_blob = nullptr;


    hr = D3DCompile(shaderSource, strlen(shaderSource), "basic pixel shader", nullptr, nullptr,
        "ps_main",
        "ps_5_0",
        flags,
        0,
        &ps_blob_ptr,
        &error_blob);

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to compile pixel shader\n");
        if (error_blob)
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
        exit(EXIT_FAILURE);
    }

    if (error_blob) error_blob->Release();
    error_blob = nullptr;

    ID3D11VertexShader* vertex_shader_ptr = NULL;
    ID3D11PixelShader* pixel_shader_ptr = NULL;

    hr = device->CreateVertexShader(
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        NULL,
        &vertex_shader_ptr);
    assert(SUCCEEDED(hr));

    hr = device->CreatePixelShader(
        ps_blob_ptr->GetBufferPointer(),
        ps_blob_ptr->GetBufferSize(),
        NULL,
        &pixel_shader_ptr);
    assert(SUCCEEDED(hr));

    vs_blob_ptr->Release();
    ps_blob_ptr->Release();
 
    // Skipping input layout, we're not going to use vertex buffers but let the shader do all the work

    // Bind the shaders to the context
    device_context_11_x->VSSetShader(vertex_shader_ptr, nullptr, 0);
    device_context_11_x->PSSetShader(pixel_shader_ptr, nullptr, 0);


#if 0
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      /*
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
    };

    hr = device->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        &input_layout_ptr);
    assert(SUCCEEDED(hr));
#endif

    
    // Create a constant buffer (for framebuffer dimensions

    {
        // Fill in a buffer description.
        D3D11_BUFFER_DESC VsConstData_dims_desc = {
            .ByteWidth = sizeof(VS_CONSTANT_BUFFER),
            .Usage = D3D11_USAGE_DYNAMIC,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
            .MiscFlags = 0,
            .StructureByteStride = 0
        };

        D3D11_SUBRESOURCE_DATA InitData = {
            .pSysMem = &VsConstData_dims,
            .SysMemPitch = 0,
            .SysMemSlicePitch = 0
        };

        hr = device->CreateBuffer(&VsConstData_dims_desc, &InitData, &shaderConstantBuffer_dims);

        if (FAILED(hr))
        {
            OutputDebugStringA("Failed to create constant buffer");
            exit(EXIT_FAILURE);
        }
    }
    
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

void render(void)
{

#if MSAA_ENABLED
    ID3D11RenderTargetView* target = msaa_render_target_view;
#else
    ID3D11RenderTargetView* target = render_target_view;
#endif

    // Clear the backbuffer entirely
    {
        float clearColor1[4] = { 0.2f, 0.2f, 0.7f, 1.0f };
        device_context_11_x->ClearRenderTargetView(target, &clearColor1[0]);
    }



#if 1
    // Set the viewport
    {
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,   // X, Y
            (FLOAT)window_width,
            (FLOAT)window_height,
            0.0f, 1.0f };   // DepthMin, DepthMax

#if 0
        char msg[1024];
        snprintf(msg, 1024, "render time viewport dimensions %f x %f\n", viewport.Width, viewport.Height);
        OutputDebugStringA(msg);
#endif

        device_context_11_x->RSSetViewports(1, &viewport);

    }
#endif


    UINT num_views = 1;
    device_context_11_x->OMSetRenderTargets(num_views, &target, nullptr);
    
    if (feature_level >= 1)
    {
        // Only supported on D3D 11.1, otherwise we have to draw quads (per Metal)

        // Clear a 200x200 pixel square at position 100x100
        {
            const FLOAT clearColor[4] = { 0.7f, 0.7f, 1.0f, 1.0f };

            // D3D11 appears to be top to bottom
            //RECT = {x1, y1, x2, y2}
            const D3D11_RECT rect = {
                .left = 100,
                .top = 100,
                .right = 300,
                .bottom = 300
            };

            device_context_11_x->ClearView(target, clearColor, &rect, 1);
        }


        {
            const FLOAT clearColor[4] = { 1.0f, 0.2f, 0.7f, 1.0f };
            D3D11_RECT rect;
            rect.left = std::min(600, (int32_t)window_width);
            rect.top = std::min(100, (int32_t)window_height);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Pink from top
            device_context_11_x->ClearView(target, clearColor, &rect, 1);
        }


        {
            const FLOAT clearColor2[4] = { 0.2f, 1.0f, 0.7f, 1.0f };
            D3D11_RECT rect;
            rect.left = std::min(600L, (LONG)window_width);
            rect.top = std::max(0L, (LONG)window_height - 600L);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Green from bottom
            device_context_11_x->ClearView(target, clearColor2, &rect, 1);
        }

        // TODO: How to enable blending?
        // We need a blend attachment

        {
            const FLOAT clearColor3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            D3D11_RECT rect;
            rect.left = std::max(600L, (LONG)window_width - 200L);
            rect.top = std::max(0L, (LONG)window_height - 200L);
            rect.right = rect.left + 100;
            rect.bottom = rect.top + 100;

            // Black from bottom right
            device_context_11_x->ClearView(target, clearColor3, &rect, 1);
        }

    }



    // Draw something, we don't use vertex buffers - all the magic happens in the vertex shader

    device_context_11_x->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Seems we still require an Input Assembler

    // Not necessary, we're using shader defined vertices

    //device_context_11_x->IASetVertexBuffers(0, 0, nullptr, 0, 0);
    //device_context_11_x->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    //device_context_11_x->IASetInputLayout(nullptr);    



    // We must use MAP to update constant buffers
    //device_context_11_x->UpdateSubresource(shaderConstantBuffer_dims, 0, 0, &VsConstData_dims, 0, 0);

    //if (framechanged)
    if (1)
    {
        VsConstData_dims.width = (float)window_width;
        VsConstData_dims.height = (float)window_height;
        VsConstData_dims.time = time_lerp();
        framechanged = false;

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        // Lock the constant buffer so it can be written to.
        device_context_11_x->Map(shaderConstantBuffer_dims, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

        // Get a pointer to the data in the constant buffer.
        //ConstBuffer* dataPtr = (ConstBuffer*)mappedResource.pData;
        memcpy(mappedResource.pData, &VsConstData_dims, sizeof(VsConstData_dims));

        // Unlock the constant buffer.
        device_context_11_x->Unmap(shaderConstantBuffer_dims, 0);
    }

    device_context_11_x->VSSetConstantBuffers(0, 1, &shaderConstantBuffer_dims);

    //device_context_11_x->Draw(15, 0);   // 5 tri's
    device_context_11_x->Draw(21, 0);   // 7 tri's

#if DRAW_LOTS_UNOPTIMISED
    // Now loop and create some artificial load
    {
        const int viewports_x = 100;
        const int viewports_y = 100;

        for (int i = 0; i < viewports_x; i++)
        {
            for (int j = 0; j < viewports_y; j++)
            {
                // Set a viewport for this block
                D3D11_VIEWPORT viewport = {
                    .TopLeftX = (float)window_width / viewports_x * i,
                    .TopLeftY = (float)window_height / viewports_y * j,
                    .Width = (float)window_width / viewports_x,
                    .Height = (float)window_height / viewports_y
                };

                device_context_11_x->RSSetViewports(1, &viewport);
                device_context_11_x->Draw(21, 0);   // 7 tri's
            }
        }

        // And another layer, offset 50% in X and Y
        for (int i = 0; i < viewports_x; i++)
        {
            for (int j = 0; j < viewports_y; j++)
            {
                // Set a viewport for this block
                D3D11_VIEWPORT viewport = {
                    .TopLeftX = ((float)window_width / viewports_x * i) + (((float)window_width / viewports_x) * 0.5f),
                    .TopLeftY = ((float)window_height / viewports_y * j) + (((float)window_height / viewports_y) * 0.5f),
                    .Width = (float)window_width / viewports_x,
                    .Height = (float)window_height / viewports_y
                };

                device_context_11_x->RSSetViewports(1, &viewport);
                device_context_11_x->Draw(21, 0);   // 7 tri's
            }
        }
    }
#endif


    // Rendering is done, do the resolve

#if MSAA_ENABLED

    // Get the swapchain backbuffer
    ID3D11Texture2D* pBuffer;
    HRESULT hr = swapchain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));

    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to retrieve swapchain buffer\n");
        exit(EXIT_FAILURE);
    }

    device_context_11_x->ResolveSubresource(pBuffer, 0, msaa_render_target, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

    // Now set the actual swapchain render target
    device_context_11_x->OMSetRenderTargets(1, &render_target_view, nullptr);

    pBuffer->Release();
#endif

    // Do any UI rendering here to the non-MSAA swapchain buffer



    const UINT vsync = 0;
    //const UINT presentFlags = allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
    const UINT presentFlags = (allowTearing && !DXGI_fullscreen) ? DXGI_PRESENT_ALLOW_TEARING : 0;


    const DXGI_PRESENT_PARAMETERS presentParameters = {};

    swapchain->Present1(vsync, presentFlags, &presentParameters);
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

   InitD3D11();
   InitShaders();

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
                    swapchain->ResizeTarget(&target_mode);
                }

                swapchain->SetFullscreenState(DXGI_fullscreen, nullptr);



                {
                    ID3D11Texture2D* swapchain_framebuffer;
                    HRESULT result = swapchain->GetBuffer(0, IID_PPV_ARGS(&swapchain_framebuffer));
                    assert(SUCCEEDED(result));
                    D3D11_TEXTURE2D_DESC framebufferSurfaceDesc;
                    swapchain_framebuffer->GetDesc(&framebufferSurfaceDesc);
                    char msg[1024];
                    snprintf(msg, 1024, "Swapchain framebuffer surface dimensions : %d x %d\n", framebufferSurfaceDesc.Width, framebufferSurfaceDesc.Height);
                    OutputDebugStringA(msg);
                    swapchain_framebuffer->Release();
                    swapchain_framebuffer = nullptr;
                }

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
            swapchain->GetFullscreenState(&is_fullscreen, nullptr);

    #if D3D_SUPPORT_NONPREROTATED
            if (DXGI_fullscreen && is_fullscreen)
                swapchain_flags |= DXGI_SWAP_CHAIN_FLAG_NONPREROTATED;
    #endif

            // It is recommended to always use the tearing flag when it is supported.
            swapchain_flags |= allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;


            IDXGIOutput* output;
            swapchain->GetContainingOutput(&output);

            DXGI_OUTPUT_DESC output_desc;
            output->GetDesc(&output_desc);
            output->Release();
            output = nullptr;

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
          
            if (!window_width || !window_height)
                return 0;


            DXGI_MODE_DESC target_mode = {
                .Width = window_width,
                .Height = window_height,
                .Format = DXGI_FORMAT_UNKNOWN
            };

#if MSAA_ENABLED
            // We need to resize the msaa_framebuffer also
            {
                msaa_render_target->Release();
                msaa_render_target_view->Release();
                
                D3D11_TEXTURE2D_DESC desc = {
                    .Width = window_width,
                    .Height = window_height,
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .SampleDesc = {
                            .Count = MSAA_Count,
                            .Quality = MSAA_Quality
                        },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_RENDER_TARGET,
                    .CPUAccessFlags = 0,
                    .MiscFlags = 0
                };

                device->CreateTexture2D(&desc, nullptr, &msaa_render_target);
            }
#endif

            swapchain->ResizeTarget(&target_mode);

            device_context_11_x->OMSetRenderTargets(0, 0, 0);

            // Release all outstanding references to the swap chain's buffers.
            render_target_view->Release();
            render_target_view = nullptr;


            //device_context_11_x->ClearState();
            //device_context_11_x->Flush();



            //DXGI_OUTPUT_DESC output_desc;
            //DXGI_OUTPUT_DESC1 output_desc1;
            //platform_backend_d3d11().d3d11.device_info->dxgiOutput6->GetDesc(&output_desc);
            //dxgiOutput6->GetDesc1(&output_desc1);

            HRESULT hr;
            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.


            //hr = swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swapchain_flags);
            hr = swapchain->ResizeBuffers(0, window_width, window_height, DXGI_FORMAT_UNKNOWN, swapchain_flags);

            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to resize swapchain buffer\n");
                exit(EXIT_FAILURE);
            }


#if MSAA_ENABLED
            // Now we can create the render target image view (pointing at the framebufer images already)
            {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS,
                };

                assert(msaa_render_target);
                HRESULT result = device->CreateRenderTargetView(msaa_render_target, &rtvDesc, &msaa_render_target_view);
                assert(SUCCEEDED(result));
            }
#endif

            // Get buffer and create a render-target-view.
            ID3D11Texture2D* pBuffer;
            hr = swapchain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));

            if (FAILED(hr))
            {
                OutputDebugStringA("Failed to retrieve swapchain buffer\n");
                exit(EXIT_FAILURE);
            }

            D3D11_TEXTURE2D_DESC desc;
            pBuffer->GetDesc(&desc);

            //char msg[1024];
            snprintf(msg, 1024, "Swapchain buffer size : %d x %d\n", desc.Width, desc.Height);
            OutputDebugStringA(msg);


            {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
                };

                hr = device->CreateRenderTargetView(pBuffer, &rtvDesc, &render_target_view);

                if (FAILED(hr))
                {
                    OutputDebugStringA("Failed to create render target view\n");
                    exit(EXIT_FAILURE);
                }
            }

            pBuffer->Release();
            pBuffer = nullptr;


#if MSAA_ENABLED
            device_context_11_x->OMSetRenderTargets(1, &msaa_render_target_view, nullptr);
#else
            device_context_11_x->OMSetRenderTargets(1, &render_target_view, nullptr);
#endif

            // Set up the viewport.
            D3D11_VIEWPORT vp = {
                .Width = (float)window_width,
                .Height = (float)window_height
            };

 
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            device_context_11_x->RSSetViewports(1, &vp);
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
        device_context_11_x->DiscardView(render_target_view);

#if MSAA_ENABLED
        msaa_render_target_view->Release();
        msaa_render_target->Release();
        device_context_11_x->DiscardResource(msaa_render_target);
#endif

        //input_layout_ptr->Release();
        render_target_view->Release();
        render_target_view = nullptr;


        device_context_11_x->VSSetConstantBuffers(0, 0, nullptr);
        device_context_11_x->VSSetShader(nullptr, 0, 0);
        device_context_11_x->PSSetShader(nullptr, 0, 0);
        device_context_11_x->OMSetRenderTargets(0, nullptr, nullptr);

        device_context_11_x->DiscardResource(shaderConstantBuffer_dims);
        shaderConstantBuffer_dims->Release();

        shaderConstantBuffer_dims = nullptr;



        swapchain->Release();
        swapchain = nullptr;

        device_context_11_x->ClearState();
        device_context_11_x->Flush();

        device_context_11_x->Release();
        device_context_11_x = nullptr;

#ifndef NDEBUG
        ID3D11InfoQueue* info;
        device->QueryInterface(IID_PPV_ARGS(&info));
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, FALSE);
        info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_CREATION, FALSE);
        info->Release();
        info = nullptr;
#endif

        IDXGIInfoQueue* infoqueue;
        DXGIGetDebugInterface1(0, IID_PPV_ARGS(& infoqueue));
        
        infoqueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, FALSE);
        infoqueue->SetBreakOnCategory(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION, FALSE);
        infoqueue->Release();
        infoqueue = nullptr;

        device->Release();
        device = nullptr;

        

        dxgi_debug_report();
    }
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
