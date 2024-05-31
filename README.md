# DX11_portrait_orientation_swapchain
This repository contains a minimal repro of an issue that prevents fullscreen swapchain (FSO) from presenting correctly on a Portrait orientation display

Note that this is *not* production quality code by any stretch.
It represents a simple minimal repro of an issue I'm having elsewhere. It does not pretend to follow any reasonable coding standard :)

# Possible Fix
See <a href="https://github.com/tim-rex/DX11_portrait_orientation_swapchain/pull/1">this</a> pull request, based directly from <a href="https://github.com/tim-rex/DX11_portrait_orientation_swapchain/tree/portrait_fullscreen_scaling_possible_fix">this</a> branch

See the pull request notes for a deeper dive and discussion on wether or not this is a code issue, a documentation issue, or a DXGI issue.
The PR will remain unmerged until we can get further clarity. In the meantime, borderless fullscreen should pose no such issue.. Alternatively we should also be exploring `DXGI_SWAP_CHAIN_FLAG_NONPREROTATED` regardless

# Building
Load the solution (currently targets Visual Studio 2022)
Build the solution

Alternatively, grab a pre-built release (x64 only) from <a href="https://github.com/tim-rex/DX11_portrait_orientation_swapchain/releases">here</a>



# Running
1. Set your primary display to one that has portrait orientation
2. Start debugging
3. Press 'S' to toggle between windowed and fullscreen mode (alternatively use Alt-Enter)

# Observe
On my setup at least, the fullscreen swapchain on a portrait orientation display is incorrectly displayed.
The image is vertically compressed and horizontally stretched, as if the width/height are flipped somewhere between the Present1() and the image hitting the display

<table>
  <tr>
    <td>What it <b><i>should</i></b> look like</td>
    <td>What it <b><i>actually</i></b> looks like (on my setup)</td>
  </tr>
  <tr>
    <td><img width="403" alt="image" src="https://github.com/tim-rex/DX11_portrait_orientation_swapchain/assets/19639392/d511948f-80ed-44b7-81a3-83ebdfa46e4f"></td>
    <td><img width="403" alt="image" src="https://github.com/tim-rex/DX11_portrait_orientation_swapchain/assets/19639392/c98f56e1-1471-42ab-91bd-89a02e4bdfa3"></td>
  </tr>
</table>


Detailed write up here:
https://stackoverflow.com/questions/78549082/dxgi-fullscreen-swapchain-rendering-incorrectly-on-portrait-orientation

And originally here:
https://developercommunity.microsoft.com/t/DXGI-fullscreen-swapchain-rendering-inco/10667183?scope=follow

And then here:
https://www.microsoft.com/en-us/windowsinsider/feedbackhub/fb?contextid=382&feedbackid=930634e6-62ed-4bb4-893e-817346f4c1d3&form=1&utm_source=product-placement&utm_medium=feedback-hub&utm_campaign=feedback-hub-redirect


