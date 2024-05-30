# DX11_portrait_orientation_swapchain
This repository contains a minimal repro of an issue that prevents fullscreen swapchain (FSE) from presenting correctly on a Portrait orientation display

Note that this is *not* production quality code by any stretch.
It represents a simple minimal repro of an issue I'm having elsewhere. It does not pretend to follow any reasonable coding standard :)

# Building
Load the solution (currently targets Visual Studio 2022)
Build the solution

# Running
1. Set your primary display to one that has portrait orientation
2. Start debugging
3. Press 'S' to toggle between windowed and fullscreen mode (alternatively use Alt-Enter)

# Observe
On my setup at least, the fullscreen swapchain on a portrait orientation display is incorrectly displayed.
The image is vertically compressed and horizontally stretched, as if the width/height are flipped somewhere between the Present1() and the image hitting the display

Detailed write up here:
https://stackoverflow.com/questions/78549082/dxgi-fullscreen-swapchain-rendering-incorrectly-on-portrait-orientation

And originally here:
https://developercommunity.microsoft.com/t/DXGI-fullscreen-swapchain-rendering-inco/10667183?scope=follow

And then here:
https://www.microsoft.com/en-us/windowsinsider/feedbackhub/fb?contextid=382&feedbackid=930634e6-62ed-4bb4-893e-817346f4c1d3&form=1&utm_source=product-placement&utm_medium=feedback-hub&utm_campaign=feedback-hub-redirect


