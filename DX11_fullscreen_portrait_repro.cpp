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
#include <dxgi1_6.h>     // CheckHardwareCompositionSupport

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
ID3D11Device* device = nullptr;
ID3D11DeviceContext* device_context_11_0 = nullptr;
ID3D11DeviceContext1* device_context_11_x = nullptr;
IDXGISwapChain1* swapchain = nullptr;
ID3D11RenderTargetView* render_target_view = nullptr;

D3D_FEATURE_LEVEL feature_level;

UINT window_width = 0;
UINT window_height = 0;
BOOL DXGI_fullscreen = false;
BOOL allowTearing = false;


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
            device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, TRUE);
            info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, TRUE);

            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_INITIALIZATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_CLEANUP, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_COMPILATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_CREATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_SETTING, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_STATE_GETTING, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_EXECUTION, TRUE);
            info->SetBreakOnCategory(D3D11_MESSAGE_CATEGORY_SHADER, TRUE);

            //info->AddMessage(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, D3D11_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST MISCELLANOUS ERROR");
            //info->AddApplicationMessage(D3D11_MESSAGE_SEVERITY_ERROR, "TEST");
            //info->AddMessage(D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, D3D11_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST");

            info->Release();
        }

        // enable debug break for DXGI too
        {
            IDXGIInfoQueue* info;
            DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), (void**)&info);

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
            //info->AddMessage(DXGI_DEBUG_DXGI, DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, D3D11_MESSAGE_ID_UNKNOWN, "TEST");

            //debug_info->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

            info->Release();
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
    IDXGIAdapter* dxgiAdapter = nullptr;

    D3D_FEATURE_LEVEL feature_level_req[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    UINT device_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if defined( DEBUG ) || defined( _DEBUG )
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


    IDXGIFactory7* pFactory;
    result = CreateDXGIFactory1(IID_IDXGIFactory7, (void**)(&pFactory));

    if (!SUCCEEDED(result))
    {
        exit(EXIT_FAILURE);
    }


    // Feature support

    // Usage of CheckFeatureSupport is poorly documented, but this examples shows the way
    // Ref: https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays

    pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));


    // Device selection
    D3D_DRIVER_TYPE d3d_driver_type = D3D_DRIVER_TYPE_HARDWARE;

    IDXGIFactory7* factory = nullptr;

    // We must use IDXGI interfaces now to query and build a swapchain
    IDXGIDevice4* dxgiDevice4 = nullptr;


    result = D3D11CreateDevice(
        dxgiAdapter,
        d3d_driver_type,
        nullptr,
        device_flags,
        &feature_level_req[0],
        ARRAY_COUNT(feature_level_req),
        D3D11_SDK_VERSION,
        &device,
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
            &device,
            &feature_level,
            &device_context_11_0);
    }

    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed D3D11CreateDevice\n");
        exit(EXIT_FAILURE);
    }

    dxgi_debug_init();

    result = device->QueryInterface(IID_IDXGIDevice4, (void**)&dxgiDevice4);
    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed to query interface for dxgiDevice4\n");
        exit(EXIT_FAILURE);
    }

    result = dxgiDevice4->GetAdapter(&dxgiAdapter);
    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed to get adapter from dxgiDevice4\n");
        exit(EXIT_FAILURE);
    }

    result = dxgiAdapter->GetParent(IID_IDXGIFactory7, (void**)&factory);
    if (!SUCCEEDED(result))
    {
        OutputDebugStringA("Failed to retrieve factor from dxgiAdapter\n");
        exit(EXIT_FAILURE);
    }


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


        result = dxgiOutput->QueryInterface(IID_IDXGIOutput6, (void**)&dxgiOutput6);

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
        UINT numModes;

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
            .BufferCount = 2,								// Needs to be >= 2 for FLIP swap effect
            .Scaling = DXGI_SCALING_STRETCH,

                    // TODO: If we ever get internal rotation happening, we can go back to DXGI_SCALING_NONE without cause for concern

                    // DXGI_SCALING_STRETCH is the default, and gives us correct output in the presence of portrait orientation swapchains
                    // DXGI_SCALING_NONE has odd and IMHO undocumented or otherwise erroneous behaviour relating to portrait orientation fullscreen swapchains
                    // DXGI_SCALING_ASPECT_RATIO_STRETCH is not supported with CreateSwapChainForHwnd

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


        result = factory->CreateSwapChainForHwnd(device, hWnd, &swapchain_descriptor, &fullscreen_desc, nullptr, &swapchain);

        if (!SUCCEEDED(result))
        {
            OutputDebugStringA("Failed to CreateSwapChainForHwnd from dxgiFactory\n");
            exit(EXIT_FAILURE);
        }


        // Uncomment to prevent Alt+Enter from triggering a fullscreen switch
        //factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        factory->Release();
        dxgiAdapter->Release();
        dxgiDevice4->Release();

        assert(S_OK == result && swapchain && device && device_context_11_0);

        device_context_11_x = (ID3D11DeviceContext1*)device_context_11_0;

        OutputDebugStringA("Direct3D 11 device context created\n");

        char msg[1024];
        snprintf(msg, 1024, "            Feature level 0x%x\n", feature_level);
        OutputDebugStringA(msg);



        // First obtain the framebuffer from the swapchain

        ID3D11Texture2D* framebuffer;
        result = swapchain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&framebuffer);
        assert(SUCCEEDED(result));

        // Now we can create the render target image view (pointing at the framebufer images already)

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
        };

        result = device->CreateRenderTargetView(framebuffer, &rtvDesc, &render_target_view);
        assert(SUCCEEDED(result));

        D3D11_TEXTURE2D_DESC framebufferSurfaceDesc;
        framebuffer->GetDesc(&framebufferSurfaceDesc);

        framebuffer->Release();
    }
}


void render(void)
{
    // Clear the backbuffer entirely
    {
        float clearColor1[4] = { 0.2f, 0.2f, 0.7f, 1.0f };
        device_context_11_0->ClearRenderTargetView(render_target_view, &clearColor1[0]);
    }


#if 0
    // Set the viewport
    {
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,   // X, Y
            (FLOAT)window_width,
            (FLOAT)window_height,
            0.0f, 1.0f };   // DepthMin, DepthMax

        char msg[1024];
        snprintf(msg, 1024, "render time viewport dimensions %f x %f\n", viewport.Width, viewport.Height);
        OutputDebugStringA(msg);
        device_context_11_0->RSSetViewports(1, &viewport);
    }
#endif


    UINT num_views = 1;
    device_context_11_0->OMSetRenderTargets(num_views, &render_target_view, nullptr);
    
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

            device_context_11_x->ClearView(render_target_view, clearColor, &rect, 1);
        }


        {
            const FLOAT clearColor[4] = { 1.0f, 0.2f, 0.7f, 1.0f };
            D3D11_RECT rect;
            rect.left = std::min(600, (int32_t)window_width);
            rect.top = std::min(100, (int32_t)window_height);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Pink from top
            device_context_11_x->ClearView(render_target_view, clearColor, &rect, 1);
        }


        {
            const FLOAT clearColor2[4] = { 0.2f, 1.0f, 0.7f, 1.0f };
            D3D11_RECT rect;
            rect.left = std::min(600L, (LONG)window_width);
            rect.top = std::max(0L, (LONG)window_height - 600L);
            rect.right = rect.left + 500;
            rect.bottom = rect.top + 500;

            // Green from bottom
            device_context_11_x->ClearView(render_target_view, clearColor2, &rect, 1);
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
            device_context_11_x->ClearView(render_target_view, clearColor3, &rect, 1);
        }

    }





    const UINT vsync = 1;
    const UINT presentFlags = 0;
    const DXGI_PRESENT_PARAMETERS presentParameters = {};

    swapchain->Present1(vsync, presentFlags, &presentParameters);

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

   InitD3D11();

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
                    swapchain->ResizeTarget(&target_mode);
                }

                swapchain->SetFullscreenState(DXGI_fullscreen, nullptr);



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

            swapchain->ResizeTarget(&target_mode);

            device_context_11_0->OMSetRenderTargets(0, 0, 0);

            // Release all outstanding references to the swap chain's buffers.
            render_target_view->Release();


            //DXGI_OUTPUT_DESC output_desc;
            //DXGI_OUTPUT_DESC1 output_desc1;
            //platform_backend_d3d11().d3d11.device_info->dxgiOutput6->GetDesc(&output_desc);
            //dxgiOutput6->GetDesc1(&output_desc1);

            HRESULT hr;
            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.


            //hr = swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swapchain_flags);
            hr = swapchain->ResizeBuffers(0, window_width, window_height, DXGI_FORMAT_UNKNOWN, swapchain_flags);

            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to resize swapchain buffer\n");
                exit(EXIT_FAILURE);
            }

            // Get buffer and create a render-target-view.
            ID3D11Texture2D* pBuffer;
            hr = swapchain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&pBuffer);

            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to retrieve swapchain buffer\n");
                exit(EXIT_FAILURE);
            }

            D3D11_TEXTURE2D_DESC desc;
            pBuffer->GetDesc(&desc);

            //char msg[1024];
            snprintf(msg, 1024, "Swapchain buffer size : %d x %d\n", desc.Width, desc.Height);
            OutputDebugStringA(msg);



            hr = device->CreateRenderTargetView(pBuffer, NULL, &render_target_view);

            if (!SUCCEEDED(hr))
            {
                OutputDebugStringA("Failed to create render target view\n");
                exit(EXIT_FAILURE);
            }


            pBuffer->Release();

            device_context_11_0->OMSetRenderTargets(1, &render_target_view, nullptr);

            // Set up the viewport.
            D3D11_VIEWPORT vp = {
                .Width = (float)window_width,
                .Height = (float)window_height
            };

 
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            device_context_11_0->RSSetViewports(1, &vp);
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
